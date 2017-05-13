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

#include "Taxes/LegacyTaxes/TaxRecord.h"

#include "Common/CurrencyConversionFacade.h"
#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/FareCalcUtil.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/TaxRound.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/BankerSellRate.h"
#include "DBAccess/CountrySettlementPlanInfo.h"
#include "DBAccess/Currency.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Nation.h"
#include "DBAccess/NUCInfo.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxNation.h"
#include "DBAccess/TaxOrderTktIssue.h"
#include "FareCalc/FareCalcConsts.h"
#include "Taxes/LegacyFacades/FacadesUtils.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"
#include "Taxes/Pfc/PfcItem.h"

#include <algorithm>

#include <boost/bind.hpp>

using namespace tse;
using namespace std;

namespace tse
{
FALLBACK_DECL(fallbackValidatingCxrMultiSp);
FALLBACK_DECL(ATPCO_TAX_TaxPreSortFix);
}

log4cxx::LoggerPtr
TaxRecord::_logger(log4cxx::Logger::getLogger("atseintl.Taxes.TaxRecord"));

namespace
{
static const TaxType BLANKTAXTYPE("000");
bool
isZZTaxInTaxRecord(const TaxRecord* taxRecord)
{
  return taxRecord->taxCode().equalToConst("ZZ");
}
bool
isZZTaxInTaxItem(const TaxItem* taxItem)
{
  return taxItem->taxCode().equalToConst("ZZ");
}
inline bool
predicateEqualTaxCode(const TaxRecord* taxRecord, const TaxCode& taxCode, const TaxType& taxType)
{
  if (!taxRecord->atpcoTaxName().taxCode.empty())
  {
    return taxRecord->atpcoTaxName().taxCode == taxCode &&
           taxRecord->atpcoTaxName().taxType == taxType;
  }

  return taxRecord->taxCode() == taxCode;
}
bool
useFullTaxCode(const TaxCode& taxCode)
{
  // should probably also use list of OC tax codes..
  return taxCode == "US1" || taxCode == "US2";
}
TaxCode
getShortTaxCode(const TaxCode& taxCode)
{
  return useFullTaxCode(taxCode) ? taxCode : TaxCode(taxCode.substr(0, 2));
}
void
addUniqueTaxCodesFromTaxNation(const TaxNation* taxNation, std::vector<TaxCode>& taxCodeVec)
{
  std::vector<TaxCode>::iterator taxCodeVecI;
  for (const TaxCode& taxCode : taxNation->taxCodeOrder())
  {
    TaxCode shortCode = getShortTaxCode(taxCode);
    taxCodeVecI = std::find_if(taxCodeVec.begin(), taxCodeVec.end(),
                               [&](const TaxCode& code)
                               {
                                 return code == shortCode;
                               });
    if (taxCodeVecI == taxCodeVec.end())
      taxCodeVec.push_back(shortCode);
  }
}
}

const string
TaxRecord::TAX_CODE_XF("XF");
const string
TaxRecord::TAX_CODE_US1("US1");
const string
TaxRecord::TAX_CODE_US2("US2");
const string
TaxRecord::PFC("PASSENGER FACILITY CHARGE");

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxOut::TaxRecord
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------

TaxRecord::TaxRecord()
  : _failCode(0),
    _taxAmount(0),
    _taxAmountAdjusted(0),
    _publishedAmount(0),
    _taxNoDec(0),
    _publishedNoDec(0),
    _taxCurrencyCode(""),
    _publishedCurrencyCode(""),
    _taxCode(""),
    _taxDescription(""),
    _taxNation(""),
    _taxType(0),
    _taxRollXTNotAllowedInd(0),
    _taxRolledXTInd(0),
    _interlineTaxInd(0),
    _multiOccConvRndInd(0),
    _gstTaxInd(0),
    _serviceFee(0),
    _taxItemIndex(0),
    _carrierCode(""),
    _localBoard(""),
    _intermediateCurrency(""),
    _intermediateNoDec(0),
    _exchangeRate1(0),
    _exchangeRate1NoDec(0),
    _exchangeRate2(0),
    _exchangeRate2NoDec(0),
    _intermediateUnroundedAmount(0),
    _intermediateAmount(0),
    _legId(-1),
    _specialProcessNo(0),
    _taxOnChangeFee(false)
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxRecord::~TaxRecord
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------

TaxRecord::~TaxRecord()
{
}
void
TaxRecord::buildTicketLine(PricingTrx& trx,
                           TaxResponse& taxResponse,
                           bool isFT,
                           bool showExemptedTaxes,
                           bool isAtpcoProcessing)
{
  buildTicketLine(trx,
                  taxResponse,
                  trx.countrySettlementPlanInfo(),
                  isFT,
                  showExemptedTaxes,
                  isAtpcoProcessing);
}

void
TaxRecord::buildTicketLine(PricingTrx& trx,
                           TaxResponse& taxResponse,
                           const CountrySettlementPlanInfo* cspi,
                           bool isFT,
                           bool showExemptedTaxes,
                           bool isAtpcoProcessing)
{
  if ((taxResponse.taxItemVector().empty()) && (taxResponse.pfcItemVector().empty()))
    return;

  bool isShoppingTaxRequest = TrxUtil::isShoppingTaxRequest(&trx);

  if (showExemptedTaxes)
    taxResponse.taxRecordVector().clear();

  if (!fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP())
  {
    if (trx.getRequest()->getSettlementMethod() == "TCH" ||
        (cspi && cspi->getSettlementPlanTypeCode() == "TCH"))
    {
      std::stable_partition(taxResponse.taxItemVector().begin(),
          taxResponse.taxItemVector().end(),
          isZZTaxInTaxItem);
    }
  }
  else
  {
    if (trx.getRequest()->getSettlementMethod() == "TCH")
    {
      std::stable_partition(taxResponse.taxItemVector().begin(),
          taxResponse.taxItemVector().end(),
          isZZTaxInTaxItem);
    }
  }

  if (!isAtpcoProcessing ||
      (isAtpcoProcessing && trx.atpcoTaxesActivationStatus().isTaxOnItinYqYrTaxOnTax()))
  {
    fillTaxRecordVector(trx, taxResponse, isFT, showExemptedTaxes, false, isAtpcoProcessing);
  }

  if (!isAtpcoProcessing ||
      (isAtpcoProcessing && trx.atpcoTaxesActivationStatus().isTaxOnChangeFee()))
  {
    bool isChangeFeeVector = true;
    fillTaxRecordVector(trx, taxResponse, isFT, showExemptedTaxes, isChangeFeeVector);
  }

  if (LIKELY(!isShoppingTaxRequest))
  {
    // Put Taxes in requested Origin Country Country order
    if (!TrxUtil::isTOIAllowed(trx))
      sortOrigin(trx, taxResponse);
    else
      preSortAtpco(trx, taxResponse);

    // Put Taxes in requested Country order
    sort(trx, taxResponse);

    if (!fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP())
    {
      if (trx.getRequest()->getSettlementMethod() == "TCH" ||
          (cspi && cspi->getSettlementPlanTypeCode() == "TCH"))
      {
        std::stable_partition(taxResponse.taxRecordVector().begin(),
            taxResponse.taxRecordVector().end(),
            isZZTaxInTaxRecord);
      }
    }
    else
    {
      if (trx.getRequest()->getSettlementMethod() == "TCH")
      {
        std::stable_partition(taxResponse.taxRecordVector().begin(),
            taxResponse.taxRecordVector().end(),
            isZZTaxInTaxRecord);
      }
    }
  }

  // Round certain totaled taxes
  round(trx, taxResponse);

  const bool skipPfcForAtpcoTaxes =
    isAtpcoProcessing && trx.atpcoTaxesActivationStatus().isOldTaxesCalculated();

  if (!skipPfcForAtpcoTaxes)
  {
    addPfcItems(trx, taxResponse);
  }

  if (LIKELY(!isShoppingTaxRequest))
  {
    // Set Tax Record to Ticket Box Request
    ticketBoxCompression(trx, taxResponse);
  }
}

void
TaxRecord::addPfcItems(PricingTrx& trx, TaxResponse& taxResponse)
{
  if (taxResponse.pfcItemVector().empty())
  {
    return;
  }

  TaxRecord* pTaxRecord = nullptr;

  // lint --e{413}
  trx.dataHandle().get(pTaxRecord);
  if (pTaxRecord == nullptr)
  {
    LOG4CXX_WARN(_logger, "NO MEMORY AVAILABLE *** TaxRecord::buildTicketLine ***");
    return;
  }

  CurrencyConversionFacade ccFacade;
  TaxRound taxRound;

  std::vector<PfcItem*>::const_iterator pfcItemI = taxResponse.pfcItemVector().begin();

  pTaxRecord->initialize(trx, **pfcItemI);

  RoundingFactor roundingUnit = 0.1;
  CurrencyNoDec roundingNoDec = pTaxRecord->taxNoDec();
  RoundingRule roundingRule = NONE;
  MoneyAmount taxAmount;
  std::map<std::pair<CarrierCode, int16_t>, TaxRecord*> snapMap;

  for (; pfcItemI != taxResponse.pfcItemVector().end(); pfcItemI++)
  {
    if (trx.snapRequest())
    {
      const std::pair<CarrierCode, int16_t> key((*pfcItemI)->carrierCode(), (*pfcItemI)->legId());
      if (snapMap.find(key) != snapMap.end())
      {
        pTaxRecord = snapMap[key];
      }
      else
      {
        pTaxRecord = nullptr;
        trx.dataHandle().get(pTaxRecord);
        pTaxRecord->initialize(trx, **pfcItemI);
        roundingNoDec = pTaxRecord->taxNoDec();
        snapMap[key] = pTaxRecord;
      }
    }

    if (pTaxRecord->taxCurrencyCode() == (*pfcItemI)->pfcCurrencyCode())
    {
      pTaxRecord->setTaxAmount(pTaxRecord->getTaxAmount() + (*pfcItemI)->pfcAmount());
      continue;
    }

    Money targetMoney(pTaxRecord->_taxCurrencyCode);
    Money sourceMoney((*pfcItemI)->pfcAmount(), (*pfcItemI)->pfcCurrencyCode());

    if (!ccFacade.convert(targetMoney, sourceMoney, trx, false, CurrencyConversionRequest::TAXES))
    {
      LOG4CXX_WARN(_logger, "Currency Convertion Collection *** TaxRecord::buildTicketLine ***");
      continue;
    }

    taxRound.retrieveNationRoundingSpecifications(trx, roundingUnit, roundingNoDec, roundingRule);

    taxAmount = taxRound.applyTaxRound(
        targetMoney.value(), pTaxRecord->taxCurrencyCode(), roundingUnit, roundingRule);

    if (taxAmount)
    {
      pTaxRecord->setTaxAmount(pTaxRecord->getTaxAmount() + taxAmount);
    }
  }

  if (trx.snapRequest())
  {
    std::map<std::pair<CarrierCode, int16_t>, TaxRecord*>::const_iterator snapMapIt =
        snapMap.begin();
    const std::map<std::pair<CarrierCode, int16_t>, TaxRecord*>::const_iterator snapMapItEnd =
        snapMap.end();
    for (; snapMapIt != snapMapItEnd; ++snapMapIt)
    {
      taxResponse.taxRecordVector().push_back(snapMapIt->second);
    }
  }
  else
  {
    taxResponse.taxRecordVector().push_back(pTaxRecord);
  }
}

void
TaxRecord::fillTaxRecordVector(PricingTrx& trx,
                               TaxResponse& taxResponse,
                               bool isFT,
                               bool showExemptedTaxes,
                               bool isChangeFeeVector,
                               bool isAtpcoProcessing)
{
  std::vector<TaxItem*>::const_iterator taxItemI;
  std::vector<TaxItem*>::const_iterator taxItemEndIter;

  if (isChangeFeeVector)
  {
    taxItemI = taxResponse.changeFeeTaxItemVector().begin();
    taxItemEndIter = taxResponse.changeFeeTaxItemVector().end();
  }
  else
  {
    taxItemI = taxResponse.taxItemVector().begin();
    taxItemEndIter = taxResponse.taxItemVector().end();
  }

  for (; taxItemI != taxItemEndIter; taxItemI++)
  {
    if (!(*taxItemI)->serviceFee())
    {
      if (((*taxItemI)->taxAmount() < EPSILON) && (!(*taxItemI)->failCode()))
        continue;
    }

    if ((*taxItemI)->failCode())
    {
      if ((*taxItemI)->failCode() != TaxItem::EXEMPT_ALL_TAXES &&
          (*taxItemI)->failCode() != TaxItem::EXEMPT_SPECIFIED_TAXES)
        continue;

      if (!showExemptedTaxes)
        continue;
    }

    if (isAtpcoProcessing &&
        trx.atpcoTaxesActivationStatus().isOldTaxesCalculated() &&
        !trx.atpcoTaxesActivationStatus().isTaxOnItinYqYrTaxOnTax() &&
        FacadesUtils::isYqYr((*taxItemI)->taxCode()))
    {
      continue;
    }

    if (adjust(trx, **taxItemI, taxResponse, isFT))
      continue;

    TaxRecord* pTaxRecord = nullptr;

    // lint --e{413}
    trx.dataHandle().get(pTaxRecord);

    if (UNLIKELY(pTaxRecord == nullptr))
    {
      LOG4CXX_WARN(_logger, "NO MEMORY AVAILABLE *** TaxRecord::buildTicketLine ***");

      continue;
    }

    pTaxRecord->initialize(**taxItemI);

    taxResponse.taxRecordVector().push_back(pTaxRecord);
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int TaxRecord::adjustTaxRecord
//
//
// </PRE>
// ----------------------------------------------------------------------------

bool
TaxRecord::adjust(PricingTrx& trx, const TaxItem& taxItem, TaxResponse& taxResponse, bool isFT)
{
  std::vector<TaxRecord*>::iterator taxRecordI = taxResponse.taxRecordVector().begin();

  for (; taxRecordI != taxResponse.taxRecordVector().end(); taxRecordI++)
  {
    if ((taxItem.taxCode() == TAX_CODE_US2) || (taxItem.taxCode() == TAX_CODE_US1))
    {
      if ((*taxRecordI)->taxCode() == taxItem.taxCode())
      {
        (*taxRecordI)->setTaxAmount((*taxRecordI)->getTaxAmount() + taxItem.taxAmount());
        return true;
      }
      continue;
    }

    if (UNLIKELY(trx.snapRequest() && (((*taxRecordI)->carrierCode() != taxItem.carrierCode()) ||
                              ((*taxRecordI)->legId() != taxItem.legId()))))
    {
      continue;
    }

    if (UNLIKELY(isFT))
    {
      if ((*taxRecordI)->taxCode() != taxItem.taxCode())
        continue;
    }
    else
    {
      if (strncmp((*taxRecordI)->taxCode().c_str(), taxItem.taxCode().c_str(), 2) != 0)
        continue;
    }

    if (UNLIKELY(taxItem.taxOnChangeFee()))
      continue;

    (*taxRecordI)->setTaxAmount((*taxRecordI)->getTaxAmount() + taxItem.taxAmount());
    return true;
  }

  return false;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int TaxRecord::initialize
//
//
// </PRE>
// ----------------------------------------------------------------------------

void
TaxRecord::initialize(const TaxItem& taxItem)
{
  _failCode = taxItem.failCode();
  setTaxAmount(taxItem.taxAmount());
  _taxAmountAdjusted = taxItem.taxAmountAdjusted();
  _publishedAmount = taxItem.taxAmt();
  _taxNoDec = taxItem.paymentCurrencyNoDec();
  _publishedNoDec = taxItem.taxNodec();
  _taxCurrencyCode = taxItem.paymentCurrency();
  _publishedCurrencyCode = taxItem.taxCur();
  _taxCode = taxItem.taxCode();
  _taxDescription = taxItem.taxDescription();
  _taxNation = taxItem.nation();
  _taxType = taxItem.taxType();
  _taxRollXTNotAllowedInd = taxItem.showseparateInd();
  _gstTaxInd = taxItem.gstTax();
  //  _serviceFee = taxItem.serviceFee();
  _multiOccConvRndInd = taxItem.multioccconvrndInd();
  _localBoard = taxItem.taxLocalBoard();
  _carrierCode = taxItem.carrierCode();
  _intermediateCurrency = taxItem.intermediateCurrency();
  _intermediateNoDec = taxItem.intermediateNoDec();
  _exchangeRate1 = taxItem.exchangeRate1();
  _exchangeRate1NoDec = taxItem.exchangeRate1NoDec();
  _exchangeRate2 = taxItem.exchangeRate2();
  _exchangeRate2NoDec = taxItem.exchangeRate2NoDec();
  _intermediateUnroundedAmount = taxItem.intermediateUnroundedAmount();
  _intermediateAmount = taxItem.intermediateAmount();
  _legId = taxItem.legId();
  _specialProcessNo = taxItem.specialProcessNo();
  _taxOnChangeFee = taxItem.taxOnChangeFee();
  _atpcoTaxName = taxItem.atpcoTaxName();
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int TaxRecord::sortOrigin
//
//
// </PRE>
// ----------------------------------------------------------------------------

void
TaxRecord::sortOrigin(PricingTrx& trx, TaxResponse& taxResponse)
{
  if (taxResponse.taxRecordVector().empty())
    return;

  const FareCalcConfig* fareCalcConfig = FareCalcUtil::getFareCalcConfig(trx);

  if (fareCalcConfig == nullptr)
    return;

  if (trx.getOptions()->sortTaxByOrigCity() != '3' &&
      fareCalcConfig->taxPlacementInd() != FareCalcConsts::FC_THREE)
    return;

  const AirSeg* airSeg;
  const TaxNation* taxNation;

  std::vector<TaxCode> taxCodeVec;
  std::vector<TaxCode>::iterator taxCodeVecI;

  std::vector<TaxCode>::const_iterator taxCodeI;

  std::vector<TravelSeg*>::const_iterator travelSegI =
      taxResponse.farePath()->itin()->travelSeg().begin();

  for (; travelSegI != taxResponse.farePath()->itin()->travelSeg().end(); travelSegI++)
  {
    airSeg = dynamic_cast<const AirSeg*>(*travelSegI);

    if (!airSeg)
      continue;

    taxNation = trx.dataHandle().getTaxNation((*travelSegI)->origin()->nation(),
                                              trx.getRequest()->ticketingDT());

    if (!taxNation)
      continue;

    if (UNLIKELY(taxNation->taxCodeOrder().empty()))
      continue;

    for (taxCodeI = taxNation->taxCodeOrder().begin(); taxCodeI != taxNation->taxCodeOrder().end();
         taxCodeI++)
    {
      for (taxCodeVecI = taxCodeVec.begin(); taxCodeVecI != taxCodeVec.end(); taxCodeVecI++)
      {
        if (*taxCodeVecI == *taxCodeI)
          break;
      }
      if (taxCodeVecI == taxCodeVec.end())
        taxCodeVec.push_back(*taxCodeI);
    }

    taxNation = trx.dataHandle().getTaxNation((*travelSegI)->destination()->nation(),
                                              trx.getRequest()->ticketingDT());

    if (!taxNation)
      continue;

    if (UNLIKELY(taxNation->taxCodeOrder().empty()))
      continue;

    for (taxCodeI = taxNation->taxCodeOrder().begin(); taxCodeI != taxNation->taxCodeOrder().end();
         taxCodeI++)
    {
      for (taxCodeVecI = taxCodeVec.begin(); taxCodeVecI != taxCodeVec.end(); taxCodeVecI++)
      {
        if (*taxCodeVecI == *taxCodeI)
          break;
      }
      if (taxCodeVecI == taxCodeVec.end())
        taxCodeVec.push_back(*taxCodeI);
    }
  }

  if (!taxCodeVec.empty())
  {
    std::vector<TaxRecord*>::iterator taxRecordI;
    std::vector<TaxRecord*>::iterator taxRecordSwapI;

    uint32_t taxOrderIndex = 0;

    std::vector<TaxCode>::iterator taxCodeI;
    taxCodeI = taxCodeVec.begin();

    for (; taxCodeI != taxCodeVec.end(); taxCodeI++)
    {
      for (taxRecordI = taxResponse.taxRecordVector().begin() + taxOrderIndex;
           taxRecordI < taxResponse.taxRecordVector().end();
           taxRecordI++)
      {
        if ((*taxRecordI)->taxCode() == (*taxCodeI))
        {
          taxRecordSwapI = taxResponse.taxRecordVector().begin() + taxOrderIndex;
          swap((*taxRecordSwapI), (*taxRecordI));
          taxOrderIndex++;
          break;
        }
      }
    }
  }
}

void
TaxRecord::preSortAtpco(PricingTrx& trx, TaxResponse& taxResponse)
{
  if (taxResponse.taxRecordVector().size() <= 1)
    return;

  const FareCalcConfig* fareCalcConfig = FareCalcUtil::getFareCalcConfig(trx);
  bool sortByOrigin = (fareCalcConfig != nullptr &&
                       (trx.getOptions()->sortTaxByOrigCity() == '3' ||
                        fareCalcConfig->taxPlacementInd() == FareCalcConsts::FC_THREE));

  std::vector<TaxCode> taxCodeVec;

  const TaxNation* taxNation = nullptr;
  if (!sortByOrigin)
  {
    if (fallback::ATPCO_TAX_TaxPreSortFix(&trx))
    {
      taxCodeVec.push_back("YQ");
      taxCodeVec.push_back("YR");
    }

    // First goes tax nation
    taxNation = trx.dataHandle().getTaxNation(
        trx.getRequest()->ticketingAgent()->agentLocation()->nation(),
        trx.getRequest()->ticketingDT());

    if (LIKELY(taxNation))
      addUniqueTaxCodesFromTaxNation(taxNation, taxCodeVec);
  }

  // And then nations as they appear in the itinerary
  const AirSeg* airSeg;
  for (const TravelSeg* travelSeg : taxResponse.farePath()->itin()->travelSeg())
  {
    airSeg = dynamic_cast<const AirSeg*>(travelSeg);

    if (!airSeg)
      continue;

    // Origin
    taxNation = trx.dataHandle().getTaxNation(travelSeg->origin()->nation(),
                                              trx.getRequest()->ticketingDT());
    if (LIKELY(taxNation))
      addUniqueTaxCodesFromTaxNation(taxNation, taxCodeVec);

    // Destination
    taxNation = trx.dataHandle().getTaxNation(travelSeg->destination()->nation(),
                                              trx.getRequest()->ticketingDT());
    if (LIKELY(taxNation))
      addUniqueTaxCodesFromTaxNation(taxNation, taxCodeVec);
  }

  if (!fallback::ATPCO_TAX_TaxPreSortFix(&trx))
  {
    // add YQ/YRs at the beginning or at the end
    if (!sortByOrigin)
    {
      taxCodeVec.insert(taxCodeVec.begin(), "YR");
      taxCodeVec.insert(taxCodeVec.begin(), "YQ");
    }
    else
    {
      taxCodeVec.push_back("YR");
      taxCodeVec.push_back("YQ");
    }
  }

  // XF at the end
  taxCodeVec.push_back("XF");

  // Now do the sorting
  unsigned int topEntryIdx = 0;
  std::vector<TaxRecord*>::iterator endIt = taxResponse.taxRecordVector().end();
  std::vector<TaxRecord*>::iterator targetIt;
  std::vector<TaxRecord*>::iterator foundIt;
  for (const TaxCode& taxCode : taxCodeVec)
  {
    do
    {
      targetIt = taxResponse.taxRecordVector().begin() + topEntryIdx;
      foundIt = std::find_if(targetIt,
                             endIt,
                             [&](const TaxRecord* taxRecord)
                             {
                               return taxCode == getShortTaxCode(taxRecord->taxCode());
                             });
      if (foundIt != endIt)
      {
        std::iter_swap(targetIt, foundIt);
        topEntryIdx++;
      }
    } while (foundIt != endIt);
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int TaxRecord::sort
//
//
// </PRE>
// ----------------------------------------------------------------------------

void
TaxRecord::sort(PricingTrx& trx, TaxResponse& taxResponse)
{
  if (taxResponse.taxRecordVector().empty())
    return;

  const TaxNation* taxNation =
      trx.dataHandle().getTaxNation(trx.getRequest()->ticketingAgent()->agentLocation()->nation(),
                                    trx.getRequest()->ticketingDT());

  if (LIKELY(taxNation))
  {
    if (!taxNation->taxOrderTktIssue().empty())
    {
      if (TrxUtil::isTOIAllowed(trx))
        sortAtpco(taxNation->taxOrderTktIssue(), taxResponse);
      else
        sortLegacy(taxNation->taxOrderTktIssue(), taxResponse);
    }
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int TaxRecord::sortLegacy
//
//
// </PRE>
// ----------------------------------------------------------------------------
void
TaxRecord::sortLegacy(const std::vector<TaxOrderTktIssue>& taxOrderTktIssues,
                      TaxResponse& taxResponse)
{
  std::vector<TaxRecord*>::iterator taxRecordI;
  std::vector<TaxRecord*>::iterator taxRecordSwapI;

  uint32_t taxOrderIndex = 0;

  std::vector<TaxOrderTktIssue>::const_iterator tktIssueI;
  tktIssueI = taxOrderTktIssues.begin();

  for (; tktIssueI != taxOrderTktIssues.end(); tktIssueI++)
  {
    if (tktIssueI->taxType() != BLANKTAXTYPE)
      continue;

    for (taxRecordI = taxResponse.taxRecordVector().begin() + taxOrderIndex;
         taxRecordI < taxResponse.taxRecordVector().end();
         taxRecordI++)
    {
      if ((*taxRecordI)->taxCode() == tktIssueI->taxCode())
      {
        taxRecordSwapI = taxResponse.taxRecordVector().begin() + taxOrderIndex;
        swap((*taxRecordSwapI), (*taxRecordI));
        taxOrderIndex++;
        break;
      }
    }
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int TaxRecord::sortAtpco
//
//
// </PRE>
// ----------------------------------------------------------------------------
void
TaxRecord::sortAtpco(const std::vector<TaxOrderTktIssue>& taxOrderTktIssues,
                     TaxResponse& taxResponse)
{
  unsigned int topEntryIdx = 0;
  for (const TaxOrderTktIssue& taxOrderTktIssue : taxOrderTktIssues)
  {
    if (taxOrderTktIssue.taxType() == BLANKTAXTYPE)
      continue;

    std::vector<TaxRecord*>::iterator foundIt;
    std::vector<TaxRecord*>::iterator targetIt =
        taxResponse.taxRecordVector().begin() + topEntryIdx;
    foundIt = std::find_if(
        targetIt, // no need to find from begin() every time
        taxResponse.taxRecordVector().end(),
        boost::bind(
            predicateEqualTaxCode, _1, taxOrderTktIssue.taxCode(), taxOrderTktIssue.taxType()));

    if (foundIt != taxResponse.taxRecordVector().end())
    {
      std::iter_swap(targetIt, foundIt);
      topEntryIdx++;
    }
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int TaxRecord::round
//
//
// </PRE>
// ----------------------------------------------------------------------------

void
TaxRecord::round(PricingTrx& trx, TaxResponse& taxResponse)
{
  if (taxResponse.taxRecordVector().empty())
    return;

  std::vector<TaxItem*>::const_iterator taxItemI;
  std::vector<TaxRecord*>::iterator taxRecordI;

  for (taxRecordI = taxResponse.taxRecordVector().begin();
       taxRecordI != taxResponse.taxRecordVector().end();
       taxRecordI++)
  {
    if (UNLIKELY((*taxRecordI)->multiOccConvRndInd() == YES))
    {
      MoneyAmount totalFareAmount = 0.0;

      std::vector<TaxItem*>::const_iterator taxItemIterMatch;
      std::vector<TaxItem*>::const_iterator taxItemIterEnd;

      taxItemI = taxResponse.taxItemVector().begin();
      taxItemIterEnd = taxResponse.taxItemVector().end();

      taxItemIterMatch = taxItemIterEnd;

      for (; taxItemI != taxItemIterEnd; taxItemI++)
      {
        if ((*taxItemI)->taxCode() == (*taxRecordI)->taxCode())
        {
          taxItemIterMatch = taxItemI;
          totalFareAmount += (*taxItemI)->taxableFare();
        }
      }

      if (taxItemIterMatch == taxItemIterEnd)
        continue;

      taxItemI = taxItemIterMatch;

      TaxRound taxRound;

      if (((*taxItemI)->spclTaxRounding() == YES) &&
          ((*taxItemI)->nation() == trx.getRequest()->ticketingAgent()->agentLocation()->nation()))
      {

        const std::vector<TaxSpecConfigReg*>* taxSpecConfig =
            (*taxItemI)->specConfigName().empty() ?
                nullptr : &trx.dataHandle().getTaxSpecConfig((*taxItemI)->specConfigName());

        MoneyAmount taxSpecialAmount =
          taxRound.doSpecialTaxRound(trx, totalFareAmount,
              (*taxRecordI)->getTaxAmount(), utc::specialTaxRoundCentNumber(trx, taxSpecConfig));

        if (taxSpecialAmount)
        {
          (*taxRecordI)->setTaxAmount(taxSpecialAmount);
        }
        continue;
      }

      RoundingFactor roundingUnit = (*taxItemI)->taxcdRoundUnit();
      CurrencyNoDec roundingNoDec = (*taxItemI)->taxcdRoundUnitNodec();
      RoundingRule roundingRule = (*taxItemI)->taxcdRoundRule();

      if (((*taxItemI)->taxCur() == (*taxItemI)->paymentCurrency()) &&
          ((*taxItemI)->taxType() == 'F') && (roundingRule == EMPTY))
        continue;

      if ((*taxItemI)->taxCur() != (*taxItemI)->paymentCurrency() || (roundingRule == EMPTY))
      {
        taxRound.retrieveNationRoundingSpecifications(
            trx, roundingUnit, roundingNoDec, roundingRule);
      }

      MoneyAmount taxAmount = taxRound.applyTaxRound((*taxRecordI)->getTaxAmount(),
                                                     (*taxItemI)->paymentCurrency(),
                                                     roundingUnit,
                                                     roundingRule);

      if (taxAmount)
      {
        (*taxRecordI)->setTaxAmount(taxAmount);
      }
    }
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int TaxRecord::buildTaxRecord
//
// Description:  This function builds the TaxRecord object from the TaxCodeReg
// object.
//
// @param  TaxCodeReg - structure from the TaxCodeReg class for one taxCode
//
// @return - index of new taxOutItem
//
// </PRE>
// ----------------------------------------------------------------------------

void
TaxRecord::initialize(PricingTrx& trx, const PfcItem& pfcItem)
{
  _publishedAmount = pfcItem.pfcAmount();
  _publishedCurrencyCode = pfcItem.pfcCurrencyCode();

  _taxCurrencyCode = trx.getRequest()->ticketingAgent()->currencyCodeAgent();

  if (!trx.getOptions()->currencyOverride().empty())
  {
    _taxCurrencyCode = trx.getOptions()->currencyOverride();
  }

  Money money(_taxCurrencyCode);
  _taxNoDec = money.noDec(trx.ticketingDate());

  Money pubMoney(_publishedCurrencyCode);
  _publishedNoDec = pubMoney.noDec(trx.ticketingDate());

  _taxCode = TAX_CODE_XF;
  _taxNation = NATION_US;
  _taxType = PFC_TYPE;
  _taxDescription = PFC;
  _legId = pfcItem.legId();
  _carrierCode = pfcItem.carrierCode();
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int TaxRecord::ticketBoxCompression
//
//
// </PRE>
// ----------------------------------------------------------------------------

void
TaxRecord::ticketBoxCompression(PricingTrx& trx, TaxResponse& taxResponse)
{
  size_t reserveBoxForTE = trx.getRequest()->isExemptSpecificTaxes() ? 1 : 0;

  if (trx.getRequest()->numberTaxBoxes() <
      static_cast<int16_t>(taxResponse.taxRecordVector().size() + reserveBoxForTE))
  {
    std::vector<TaxRecord*> taxRecordsForXT;

    std::vector<TaxRecord*>::iterator taxRecordI;

    for (taxRecordI = taxResponse.taxRecordVector().begin();
         taxRecordI != taxResponse.taxRecordVector().end();
         taxRecordI++)
    {
      if (UNLIKELY((*taxRecordI)->rollXTNotAllowedInd() == YES))
        continue;

      if ((*taxRecordI)->getTaxAmount() < EPSILON)
        continue;

      taxRecordsForXT.push_back(*taxRecordI);
    }

    size_t addToTaxBox = 0;

    reserveBoxForTE = trx.getRequest()->isExemptSpecificTaxes() ? 1 : 0;

    if (trx.getRequest()->numberTaxBoxes() > 0 &&
        (taxRecordsForXT.size() > trx.getRequest()->numberTaxBoxes() - reserveBoxForTE))
    {
      addToTaxBox = trx.getRequest()->numberTaxBoxes() - 1;

      if (addToTaxBox >= 0 && addToTaxBox < taxRecordsForXT.size())
      {
        for (taxRecordI = taxRecordsForXT.begin() + addToTaxBox;
             taxRecordI != taxRecordsForXT.end();
             taxRecordI++)
        {
          (*taxRecordI)->taxRolledXTInd() = YES;
        }
      }
    }
  }
}
