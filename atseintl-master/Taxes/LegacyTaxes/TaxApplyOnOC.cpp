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

#include "Taxes/LegacyTaxes/TaxApplyOnOC.h"

#include "Common/Global.h"
#include "DataModel/AirSeg.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/TaxCodeReg.h"
#include "Rules/RuleUtil.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/ServiceFeesGroup.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxMap.h"
#include "Taxes/LegacyTaxes/TaxOnTax.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"

using namespace tse;


namespace
{
static const Indicator OPT_SERCICE_INFO_BAGGABE_CHARGES = 'C';
static const Indicator OPT_SERCICE_INFO_ALLOWANCE = 'A';
static const Indicator OPT_SERCICE_INFO_EMBARGOS = 'E';

static const Indicator FREE_SERVICE_F = 'F';
static const Indicator FREE_SERVICE_G = 'G';
}

TaxApplyOnOC::TaxApplyOnOC() {}

TaxApplyOnOC::~TaxApplyOnOC() {}

Tax*
TaxApplyOnOC::findSpecialTax(TaxMap& taxMap, TaxCodeReg& taxCodeReg)
{
  if (taxCodeReg.specialProcessNo())
    return taxMap.getSpecialTax(taxCodeReg.specialProcessNo());
  else
    return taxMap.getTaxOnOC();
}

// ----------------------------------------------------------------------------
// Description:  TaxApply
// ----------------------------------------------------------------------------

void
TaxApplyOnOC::applyTax(PricingTrx& trx,
                       TaxResponse& taxResponse,
                       TaxMap& taxMap,
                       TaxCodeReg& taxCodeReg,
                       const CountrySettlementPlanInfo* cspi)
{

  std::vector<ServiceFeesGroup*>::const_iterator sfgIter =
      taxResponse.farePath()->itin()->ocFeesGroup().begin();
  std::vector<ServiceFeesGroup*>::const_iterator sfgIterEnd =
      taxResponse.farePath()->itin()->ocFeesGroup().end();
  for (; sfgIter != sfgIterEnd; ++sfgIter)
  {
    ServiceFeesGroup* sfg = (*sfgIter);
    std::map<const FarePath*, std::vector<OCFees*> >& ocFeesMap = sfg->ocFeesMap();

    FarePath* farePath = taxResponse.farePath();
    std::vector<OCFees*> ocFees = ocFeesMap[farePath];

    std::vector<OCFees*>::iterator iOcFees = ocFees.begin();
    std::vector<OCFees*>::iterator iEOcFees = ocFees.end();

    for (; iOcFees != iEOcFees; ++iOcFees)
    {
      OCFees::Memento memento = (*iOcFees)->saveToMemento();
      for (size_t segCnt=0; segCnt < (*iOcFees)->segCount(); ++segCnt)
      {
        (*iOcFees)->setSeg(segCnt);

         if ((*iOcFees)->subCodeInfo()->fltTktMerchInd() == OPT_SERCICE_INFO_BAGGABE_CHARGES ||
            (*iOcFees)->subCodeInfo()->fltTktMerchInd() == OPT_SERCICE_INFO_ALLOWANCE ||
            (*iOcFees)->subCodeInfo()->fltTktMerchInd() == OPT_SERCICE_INFO_EMBARGOS)
            continue;

        if ((*iOcFees)->optFee()->taxInclInd() == TaxApplyOnOC::TAXINCLIND ||
            (*iOcFees)->optFee()->taxExemptInd() == TaxApplyOnOC::TAXEXEMPTIND)
          continue;

        if ((*iOcFees)->optFee()->notAvailNoChargeInd() == FREE_SERVICE_F ||
            (*iOcFees)->optFee()->notAvailNoChargeInd() == FREE_SERVICE_G)
          continue;

        Tax* tax = findSpecialTax(taxMap, taxCodeReg);

        if (tax == nullptr)
        {
          TaxDiagnostic::collectErrors(
              trx, taxCodeReg, taxResponse, TaxDiagnostic::NO_SPECIAL_TAX, Diagnostic809);

          return;
        }

        TaxOnOC* taxOnOC = dynamic_cast<TaxOnOC*>(tax);

        if (taxOnOC == nullptr)
        {
          TaxDiagnostic::collectErrors(
              trx, taxCodeReg, taxResponse, TaxDiagnostic::NO_SPECIAL_TAX, Diagnostic809);

          return;
        }

        taxOnOC->setOcFees(*iOcFees);

        validateTaxSeq(trx, taxResponse, *taxOnOC, taxCodeReg, cspi);
      }
      (*iOcFees)->restoreFromMemento(memento);
    }
  }
}

void
TaxApplyOnOC::initializeTaxItem(PricingTrx& trx,
                                Tax& tax,
                                TaxResponse& taxResponse,
                                TaxCodeReg& taxCodeReg)
{

  OCFees::TaxItem taxItem;

  taxItem.setTaxCode(taxCodeReg.taxCode());
  taxItem.setTaxType(taxCodeReg.taxType());

  taxItem.setTaxAmount(tax.taxAmount());
  taxItem.setNumberOfDec(tax.paymentCurrencyNoDec());
  taxItem.setCurrency(tax.paymentCurrency());

  taxItem.setTaxAmountPub(taxCodeReg.taxAmt());
  taxItem.setCurrencyPub(taxCodeReg.taxCur());
  taxItem.setSeqNo(taxCodeReg.seqNo());

  TaxOnOC& taxOnOC = static_cast<TaxOnOC&>(tax);
  addTaxResponseToItin(trx, taxResponse, taxOnOC);
  taxOnOC.getOcFees()->addTax(taxItem);
}

void
TaxApplyOnOC::addTaxResponseToItin(PricingTrx& trx, TaxResponse& taxResponse, TaxOnOC& taxOnOC) const
{
  if (taxResponse.taxItemVector().empty() &&
      taxOnOC.getOcFees()->getTaxes().empty() &&
      trx.diagnostic().diagnosticType() == Diagnostic817 &&
      dynamic_cast<AncillaryPricingTrx*>(&trx) != nullptr)
  {
    {
      boost::lock_guard<boost::mutex> g(trx.mutexTaxResponse());
      if(std::find(trx.taxResponse().begin(), trx.taxResponse().end(), &taxResponse)
         == trx.taxResponse().end())
      {
        trx.taxResponse().push_back(&taxResponse);
      }
    }
    std::vector<TaxResponse*>& taxResponsesOnItin = taxResponse.farePath()->itin()->mutableTaxResponses();
    if(std::find(taxResponsesOnItin.begin(), taxResponsesOnItin.end(), &taxResponse)
       == taxResponsesOnItin.end())
    {
      taxResponsesOnItin.push_back(&taxResponse);
    }
  }
}
