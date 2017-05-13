// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Taxes/LegacyTaxes/TaxApply.h"

#include "Common/FallbackUtil.h"
#include "Common/Global.h"
#include "Common/TrxUtil.h"
#include "Common/Logger.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/TaxCodeReg.h"
#include "Rules/RuleUtil.h"
#include "Taxes/Common/PricingTrxOps.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxMap.h"
#include "Taxes/LegacyTaxes/TaxOnTax.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"

using namespace tse;

namespace tse
{
FALLBACK_DECL(taxProcessExemptionTable);
};

Tax*
TaxApplicator::findSpecialTax(TaxMap& taxMap, TaxCodeReg& taxCodeReg)
{
  return taxMap.getSpecialTax(taxCodeReg.specialProcessNo());
}

const std::string TaxApply::HIDDEN_POINT_PARAM_NAME = "HIDDENPOINT";
const std::string TaxApply::HIDDEN_POINT_LOC1_VALUE = "LOC1";
const std::string TaxApply::HIDDEN_POINT_LOC2_VALUE = "LOC2";
const std::string TaxApply::HIDDEN_POINT_BOTH_LOCS_VALUE = "BOTH";

log4cxx::LoggerPtr
TaxApply::_logger(log4cxx::Logger::getLogger("atseintl.Taxes.TaxApply"));

// ----------------------------------------------------------------------------
// Constructor
// ----------------------------------------------------------------------------

TaxApply::TaxApply() : _travelSegStartIndex(0), _travelSegEndIndex(0) {}

// ----------------------------------------------------------------------------
// Destructor
// ----------------------------------------------------------------------------

TaxApply::~TaxApply() {}

void
TaxApply::applyTax(PricingTrx& trx,
                   TaxResponse& taxResponse,
                   TaxMap& taxMap,
                   TaxCodeReg& taxCodeReg,
                   const CountrySettlementPlanInfo* cspi)
{
  long spn = taxCodeReg.specialProcessNo();
  if (spn && TaxDiagnostic::isSpnOff(trx, spn))
    spn=0;

  Tax* tax = taxMap.getSpecialTax(spn);

  if (tax == nullptr)
  {
    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::NO_SPECIAL_TAX, Diagnostic809);

    return;
  }

  validateTaxSeq(trx, taxResponse, *tax, taxCodeReg, cspi);
}

void
TaxApply::setHiddenStopInfo(PricingTrx& trx, Tax& tax, TaxCodeReg& taxCodeReg)
{
  tax.handleHiddenPoints() = Tax::HIDDEN_POINT_NOT_HANDLED;

  if (!taxCodeReg.specConfigName().empty())
  {
    std::string rndConf = utc::getSpecConfigParameter(
        tax.taxSpecConfig(), HIDDEN_POINT_PARAM_NAME, trx.getRequest()->ticketingDT());

    if (!rndConf.empty())
    {
      if (rndConf == HIDDEN_POINT_BOTH_LOCS_VALUE)
        tax.handleHiddenPoints() = Tax::HIDDEN_POINT_BOTH_LOCS;
      else if (rndConf == HIDDEN_POINT_LOC1_VALUE)
        tax.handleHiddenPoints() = Tax::HIDDEN_POINT_LOC1;
      else if (rndConf == HIDDEN_POINT_LOC2_VALUE)
        tax.handleHiddenPoints() = Tax::HIDDEN_POINT_LOC2;
    }
  }
}

void
TaxApply::validateTaxSeq(PricingTrx& trx,
                         TaxResponse& taxResponse,
                         Tax& tax,
                         TaxCodeReg& taxCodeReg,
                         const CountrySettlementPlanInfo* cspi)
{
  if (UNLIKELY(tax.failCode() == TaxDiagnostic::NO_SPECIAL_TAX))
  {
    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::NO_SPECIAL_TAX, Diagnostic809);

    return;
  }

  tax.getTaxSpecConfig(trx, taxCodeReg);
  tax.preparePortionOfTravelIndexes(trx, taxResponse, taxCodeReg);

  setHiddenStopInfo(trx, tax, taxCodeReg);
  tax.setupLandToAirStopover(trx, taxResponse);

  bool rc = tax.validateItin(trx, taxResponse, taxCodeReg);

  if (!rc)
    return;

  rc = tax.validateBaseTax(trx, taxResponse, taxCodeReg);

  if (!rc)
    return;

  bool isZzTax = utc::isZZTax(trx, tax.taxSpecConfig());
  if (isZzTax && !tax.validateZZTax(trx, cspi))
    return;

  _travelSegStartIndex = 0;
  _travelSegEndIndex = tax.getTravelSeg(taxResponse).size() - 1;

  for (uint16_t travelSegIndex = _travelSegStartIndex;
       travelSegIndex < tax.getTravelSeg(taxResponse).size();
       travelSegIndex++)
  {
    _travelSegStartIndex = travelSegIndex;

    if (UNLIKELY( isZzTax &&
         !tax.getTravelSeg(taxResponse)[travelSegIndex]->unflown() ))
    {
      continue;
    }

    if (!tax.validateTaxOnChangeFees(trx, taxResponse, taxCodeReg))
    {
      _travelSegStartIndex = _travelSegEndIndex;
      continue;
    }

    rc = doInitialTravelSegValidations(trx, taxResponse, tax, taxCodeReg);

    if (rc)
    {
      rc = applyOccurenceValidation(trx, taxResponse, tax, taxCodeReg);
    }

    if (rc)
    {
      rc = tax.validateSequence(
          trx, taxResponse, taxCodeReg, _travelSegStartIndex, _travelSegEndIndex);
    }

    if (rc)
    {
      rc = tax.validateTicketDesignator(trx, taxResponse, taxCodeReg, _travelSegStartIndex);
    }

    if (rc == false)
    {
      if (_travelSegStartIndex)
        travelSegIndex = _travelSegStartIndex;

      continue;
    }

    rc = doFinalTravelSegValidations(trx, taxResponse, tax, taxCodeReg);

    if (rc == false)
      continue;

    tax.taxCreate(trx, taxResponse, taxCodeReg, _travelSegStartIndex, _travelSegEndIndex);

    if (TaxOnTax::useTaxOnTax(taxCodeReg))
    {
      tax.applyTaxOnTax(trx, taxResponse, taxCodeReg);
    }

    if (!tax.isExemptedTax() || fallback::taxProcessExemptionTable(&trx))
      tax.adjustTax(trx, taxResponse, taxCodeReg);

    tax.doTaxRange(trx, taxResponse, _travelSegStartIndex, _travelSegEndIndex, taxCodeReg);
    tax.doTaxRound(trx, taxCodeReg);

    initializeTaxItem(trx, tax, taxResponse, taxCodeReg);

    if (taxCodeReg.taxType() == PERCENTAGE)
    {
      if (TrxUtil::isAutomatedRefundCat33Enabled(trx))
      {
        if (isReissueOrRefund(trx))
        {
          setMixedTaxType(trx, taxResponse, taxCodeReg);
        }
      }
      else
      {
        if (trx.excTrxType() == PricingTrx::PORT_EXC_TRX ||
            trx.excTrxType() == PricingTrx::AR_EXC_TRX ||
            trx.excTrxType() == PricingTrx::NEW_WITHIN_ME ||
            trx.excTrxType() == PricingTrx::EXC1_WITHIN_ME ||
            trx.excTrxType() == PricingTrx::EXC2_WITHIN_ME)
        {
          setMixedTaxType(trx, taxResponse, taxCodeReg);
        }
      }
    }

    if ((taxCodeReg.taxType() == PERCENTAGE) &&
        (taxCodeReg.specialProcessNo() != MULTIPLE_PERCENTAGE_TAX31) &&
        (taxCodeReg.specialProcessNo() != MULTIPLE_PERCENTAGE_TAXKH1) &&
        (taxCodeReg.taxCode() != "US1") && !Tax::shouldSplitPercentageTax(trx, taxCodeReg.taxCode()) &&
        (taxCodeReg.specialProcessNo() != MULTIPLE_PERCENTAGE_TAX_JP1_00))
    {
      return;
    }

    if (travelSegIndex < _travelSegStartIndex)
      travelSegIndex = _travelSegStartIndex;
  }
}

bool
TaxApply::isReissueOrRefund(PricingTrx& trx) const
{
  return trx.excTrxType() == PricingTrx::PORT_EXC_TRX ||
      trx.excTrxType() == PricingTrx::AR_EXC_TRX ||
      trx.excTrxType() == PricingTrx::AF_EXC_TRX ||
      trx.excTrxType() == PricingTrx::NEW_WITHIN_ME ||
      trx.excTrxType() == PricingTrx::EXC1_WITHIN_ME ||
      trx.excTrxType() == PricingTrx::EXC2_WITHIN_ME;
}

// ----------------------------------------------------------------------------
// Description:  TaxApply::setMixedTaxType
//                  This function is only called for exchange transactions.
//                  It will set a TaxItem to be mixed tax type if tax on tax
//                  exist for that tax item.
// ----------------------------------------------------------------------------
void
TaxApply::setMixedTaxType(PricingTrx& trx, TaxResponse& taxResponse, TaxCodeReg& taxCodeReg)
{
  if (taxCodeReg.taxOnTaxCode().empty() || taxResponse.taxItemVector().empty())
    return;

  MoneyAmount moneyAmount = 0.0;
  std::vector<std::string>::iterator taxOnTaxCodeI = taxCodeReg.taxOnTaxCode().begin();
  std::vector<std::string>::iterator taxOnTaxCodeE = taxCodeReg.taxOnTaxCode().end();

  std::vector<TaxItem*>::const_iterator taxItemI;
  std::vector<TaxItem*>::const_iterator taxItemEndIter = taxResponse.taxItemVector().end();

  for (; taxOnTaxCodeI != taxOnTaxCodeE; taxOnTaxCodeI++)
  {
    for (taxItemI = taxResponse.taxItemVector().begin(); taxItemI != taxItemEndIter; taxItemI++)
    {
      if ((*taxOnTaxCodeI) == (*taxItemI)->taxCode())
        moneyAmount += (*taxItemI)->taxAmount();
    }
  }

  if (moneyAmount)
  {
    for (taxItemI = taxResponse.taxItemVector().begin(); taxItemI != taxItemEndIter; taxItemI++)
    {
      TaxItem& taxItem = *(*taxItemI);
      if (taxItem.taxCode() == taxCodeReg.taxCode())
        taxItem.mixedTax() = true;
    }
  }
}

// ----------------------------------------------------------------------------
// Description:  TaxApply::applyOccurenceValidation
//                  This will reject a Tax that contains the same Board/Off
//                  point as a previousely applied Tax of the same TaxCode and
//                  sambe Board/Off Point
// ----------------------------------------------------------------------------

bool
TaxApply::applyOccurenceValidation(PricingTrx& trx,
                                   TaxResponse& taxResponse,
                                   const Tax& tax,
                                   TaxCodeReg& taxCodeReg)
{
  if ((taxCodeReg.occurrence() != APPLY_TAX_ONCE_PER_BOARD_OFF) ||
      (taxResponse.taxItemVector().empty()))
    return true;

  const Loc* origin = tax.getTravelSeg(taxResponse)[_travelSegStartIndex]->origin();
  const Loc* destination = tax.getTravelSeg(taxResponse)[_travelSegEndIndex]->destination(); // lint !e530

  const Loc* appliedOrigin;
  const Loc* appliedDestination;

  std::vector<TaxItem*>::const_iterator taxItemI = taxResponse.taxItemVector().begin();

  for (; taxItemI != taxResponse.taxItemVector().end(); taxItemI++)
  {
    if ((*taxItemI)->taxCode() != taxCodeReg.taxCode())
      continue;

    appliedOrigin =
      tax.getTravelSeg(taxResponse)[(*taxItemI)->travelSegStartIndex()]->origin();
    appliedDestination =
      tax.getTravelSeg(taxResponse)[(*taxItemI)->travelSegEndIndex()]->destination();

    if ((appliedOrigin->loc() == origin->loc()) &&
        (appliedDestination->loc() == destination->loc()))
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::TAX_ONCE_PER_BOARD_OFF, Diagnostic809);
      return false;
    }
  }
  return true;
}

// ----------------------------------------------------------------------------
// Description:  TaxApply::doInitialTravelSegValidations
// ----------------------------------------------------------------------------

bool
TaxApply::doInitialTravelSegValidations(PricingTrx& trx,
                                        TaxResponse& taxResponse,
                                        Tax& tax,
                                        TaxCodeReg& taxCodeReg)
{
  uint16_t startIndex = _travelSegStartIndex;
  uint16_t endIndex = _travelSegEndIndex;

  if (tax.shouldCheckTravelDate() && taxCodeReg.tvlDateasoriginInd() != YES)
  {
    TravelSeg* travelSeg = tax.getTravelSeg(taxResponse)[_travelSegStartIndex]; // lint !e530

    if (travelSeg->departureDT() < taxCodeReg.firstTvlDate() ||
        travelSeg->departureDT() > taxCodeReg.lastTvlDate())
    {
      TaxDiagnostic::collectErrors(
          trx, taxCodeReg, taxResponse, TaxDiagnostic::TRAVEL_DATE, Diagnostic809);

      return false;
    }
  }

  if (!tax.validateLocRestrictions(trx, taxResponse, taxCodeReg, startIndex, endIndex))
    return false;

  if (UNLIKELY(endIndex > tax.getTravelSeg(taxResponse).size() - 1))
    endIndex = tax.getTravelSeg(taxResponse).size() - 1;

  if (!tax.validateTripTypes(trx, taxResponse, taxCodeReg, startIndex, endIndex))
    return false;

  if (!tax.validateGeoSpecLoc1(trx, taxResponse, taxCodeReg, startIndex, endIndex))
    return false;

  if (!tax.validateRange(trx, taxResponse, taxCodeReg, startIndex, endIndex))
    return false;

  _travelSegStartIndex = startIndex;
  _travelSegEndIndex = endIndex;

  return true;
}
// ----------------------------------------------------------------------------
// Description:  TaxApply::doInitialTravelSegValidations
// ----------------------------------------------------------------------------

bool
TaxApply::doFinalTravelSegValidations(PricingTrx& trx,
                                      TaxResponse& taxResponse,
                                      Tax& tax,
                                      TaxCodeReg& taxCodeReg)
{

  if (!tax.validateTransit(trx, taxResponse, taxCodeReg, _travelSegStartIndex))
    return false;

  if (!tax.validateCarrierExemption(trx, taxResponse, taxCodeReg, _travelSegStartIndex))
    return false;

  if (!tax.validateEquipmentExemption(trx, taxResponse, taxCodeReg, _travelSegStartIndex))
    return false;

  if (!tax.validateFareClass(trx, taxResponse, taxCodeReg, _travelSegStartIndex))
    return false;

  if (!tax.validateCabin(trx, taxResponse, taxCodeReg, _travelSegStartIndex))
    return false;

  if (!tax.validateFinalGenericRestrictions(
          trx, taxResponse, taxCodeReg, _travelSegStartIndex, _travelSegEndIndex))
    return false;

  return true;
}

// ----------------------------------------------------------------------------
// Description:  TaxApply::initializeTaxItem
// ----------------------------------------------------------------------------

void
TaxApply::initializeTaxItem(PricingTrx& trx,
                            Tax& tax,
                            TaxResponse& taxResponse,
                            TaxCodeReg& taxCodeReg)
{
  TaxItem* pTaxItem = nullptr;

  trx.dataHandle().get(pTaxItem);

  if (UNLIKELY(pTaxItem == nullptr))
  {
    LOG4CXX_WARN(_logger, "NO MEMORY AVAILABLE ***** TaxApply::initializeTaxItem *****");

    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::BAD_DATA_HANDLER_POINTER, Diagnostic809);

    return;
  }

  if (taxResponse.taxItemVector().empty())
    addUniqueTaxResponse(taxResponse, trx);

  pTaxItem->buildTaxItem(trx, tax, taxResponse, taxCodeReg); // lint !e413
  taxResponse.taxItemVector().push_back(pTaxItem);
}


