// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include "Taxes/LegacyTaxes/TaxApplyOnChangeFee.h"

#include "Common/Global.h"
#include "Common/Logger.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/Common/PricingTrxOps.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxMap.h"

using namespace tse;

TaxApplyOnChangeFee::TaxApplyOnChangeFee()
{
}

TaxApplyOnChangeFee::~TaxApplyOnChangeFee() {}

Tax*
TaxApplyOnChangeFee::findSpecialTax(TaxMap& taxMap, TaxCodeReg& taxCodeReg)
{
  if (taxCodeReg.specialProcessNo())
    return taxMap.getSpecialTax(taxCodeReg.specialProcessNo());
  else
    return taxMap.getTaxOnChangeFee();
}

// ----------------------------------------------------------------------------
// Description:  TaxApply
// ----------------------------------------------------------------------------

void
TaxApplyOnChangeFee::applyTax(PricingTrx& trx,
                              TaxResponse& taxResponse,
                              TaxMap& taxMap,
                              TaxCodeReg& taxCodeReg,
                              const CountrySettlementPlanInfo* cspi)
{
  Tax* tax = findSpecialTax(taxMap, taxCodeReg);

  if (tax == nullptr)
  {
    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::NO_SPECIAL_TAX, Diagnostic809);

    return;
  }

  TaxOnChangeFee* taxOnChangeFee = dynamic_cast<TaxOnChangeFee*>(tax);

  if (taxOnChangeFee == nullptr)
  {
    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::NO_SPECIAL_TAX, Diagnostic809);

    return;
  }

  validateTaxSeq(trx, taxResponse, *taxOnChangeFee, taxCodeReg, cspi);
}

void
TaxApplyOnChangeFee::initializeTaxItem(PricingTrx& trx,
                                       Tax& tax,
                                       TaxResponse& taxResponse,
                                       TaxCodeReg& taxCodeReg)
{
  TaxItem* pTaxItem = nullptr;

  trx.dataHandle().get(pTaxItem);

  if (pTaxItem == nullptr)
  {
    LOG4CXX_WARN(_logger, "NO MEMORY AVAILABLE ***** TaxApplyOnChangeFee::initializeTaxItem *****");

    TaxDiagnostic::collectErrors(
        trx, taxCodeReg, taxResponse, TaxDiagnostic::BAD_DATA_HANDLER_POINTER, Diagnostic809);

    return;
  }

  if (taxResponse.taxItemVector().empty())
    addUniqueTaxResponse(taxResponse, trx);

  //probably static_cast would be better
  TaxOnChangeFee* taxOnChangeFee = dynamic_cast<TaxOnChangeFee*>(&tax);

  if (taxOnChangeFee->getOldItin())
  {
    const std::vector<tse::TravelSeg*>& travelSeg = taxOnChangeFee->getTravelSeg(taxResponse);
    const uint16_t idxStart = pTaxItem->travelSegStartIndex();
    const uint16_t idxEnd = pTaxItem->travelSegEndIndex();

    pTaxItem->setTravelSegStartIndex(0);
    pTaxItem->setTravelSegEndIndex(0);
    pTaxItem->buildTaxItem(trx, tax, taxResponse, taxCodeReg, true);

    pTaxItem->setTaxLocalBoard( travelSeg[idxStart]->origin()->loc() );
    pTaxItem->setTaxLocalOff( travelSeg[idxEnd]->destination()->loc() );
  }
  else
  {
    pTaxItem->buildTaxItem(trx, tax, taxResponse, taxCodeReg, true);
  }

  taxResponse.changeFeeTaxItemVector().push_back(pTaxItem);
}

