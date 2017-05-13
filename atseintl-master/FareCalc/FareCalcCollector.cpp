//-------------------------------------------------------------------
//
//  File:        FareCalcCollector.cpp
//  Created:     September 10, 2004
//  Authors:     Mike Carroll
//
//  Description:
//
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "FareCalc/FareCalcCollector.h"

#include "Common/BSRCollectionResults.h"
#include "Common/BSRCurrencyConverter.h"
#include "Common/CommissionKeys.h"
#include "Common/CurrencyCollectionResults.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/ErrorResponseException.h"
#include "Common/FareCalcUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/FcConfig.h"
#include "Common/ItinUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/TrxUtil.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/ConsolidatorPlusUp.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FareUsage.h"
#include "DataModel/NetFarePath.h"
#include "DataModel/NetRemitFarePath.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxOverride.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/CustomerSecurityHandshakeInfo.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/PaxTypeInfo.h"
#include "DBAccess/TaxCodeReg.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag854Collector.h"
#include "Diagnostic/DiagManager.h"
#include "FareCalc/AltFareCalcCollector.h"
#include "FareCalc/AltFareCalculation.h"
#include "FareCalc/FareCalcConsts.h"
#include "FareCalc/FareCalculation.h"
#include "FareCalc/FareCalcHelper.h"
#include "FareCalc/FcCollector.h"
#include "FareCalc/FcMultiMessage.h"
#include "FareCalc/FcUtil.h"
#include "FareCalc/IETValidator.h"
#include "Pricing/PricingUtil.h"
#include "Rules/Commissions.h"
#include "Rules/RuleUtil.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"
#include "Util/FlatSet.h"

namespace tse
{
FALLBACK_DECL(fallbackRrmCmdPricFixErrorMsg);
FALLBACK_DECL(fallbackCommissionManagement);
FALLBACK_DECL(fallbackAMCPhase2);
FALLBACK_DECL(fallbackPriceByCabinActivation);
FALLBACK_DECL(fallbackFRROBFeesFixAsl);
static Logger
logger("atseintl.FareCalc.FareCalcCollector");

bool
FareCalcCollector::initialize(PricingTrx& pricingTrx,
                              Itin* itin,
                              const FareCalcConfig* fcConfig,
                              bool skip)
{
  if (!pricingTrx.validBrands().empty())
    _validBrandsVecPtr = &(pricingTrx.validBrands());

  _isFlexFare = pricingTrx.isFlexFare();

  reorderPaxTypes(pricingTrx);
  refreshTaxResponse(pricingTrx, *itin, *fcConfig);
  bool ietResult = true; // remove this variable when fallback removal for GSA
  std::string message;

  // start IET validation for pricing for the Itin except for non exchange/refund and
  // non direct ticketing transaction

  if(!pricingTrx.isValidatingCxrGsaApplicable())
  {
    if (!pricingTrx.getRequest()->ticketingAgent()->tvlAgencyPCC().empty() &&
        !TrxUtil::isExchangeOrTicketing(pricingTrx))
    {
      // is IET pricing active?
      if (fcConfig->ietPriceInterlineActive() != FareCalcConsts::FC_NO)
      {
        IETValidator iet(pricingTrx);
        ietResult = iet.validate(pricingTrx, *itin, message);
      }
    }
  }

  std::set<const PaxType*> paxTypeList;
  amc::VCFMCommissionPrograms vcfmCommProgs;
  amc::VCFMPTFCommissionRules vcfmptfCommRules;

  PseudoCityCode tvlAgencyPCC =
    (!fallback::fallbackAMCPhase2(&pricingTrx) &&
    pricingTrx.getRequest() &&
    pricingTrx.getRequest()->ticketingAgent()) ?
    pricingTrx.getRequest()->ticketingAgent()->tvlAgencyPCC() : "";

  const std::vector<CustomerSecurityHandshakeInfo*>& csHsInfoCol =
    !fallback::fallbackAMCPhase2(&pricingTrx) && !tvlAgencyPCC.empty() ?
    pricingTrx.dataHandle().getCustomerSecurityHandshake(tvlAgencyPCC, "CM", DateTime::localTime()):
    std::vector<CustomerSecurityHandshakeInfo*>();

  for (auto fp : itin->farePath())
  {
    if (fp == nullptr)
      continue;

    if (LIKELY(!skip))
    {
      Commissions comm(pricingTrx);
      if(!fallback::fallbackCommissionManagement(&pricingTrx))
        comm.getCommissions(*fp, csHsInfoCol, vcfmCommProgs, vcfmptfCommRules);
      else
        comm.getCommissions(*fp);
    }

    if ((pricingTrx.getRequest()->owPricingRTTaxProcess()) && (fp->duplicate()))
    {
      continue;
    }

    if (itin->isPlusUpPricing() && !skip)
      processConsolidatorPlusUp(pricingTrx, itin, fp);

    CalcTotals* totals = createCalcTotals(pricingTrx, fcConfig, fp);
    if (totals == nullptr)
      throw ErrorResponseException(ErrorResponseException::MEMORY_EXCEPTION);

    if(!pricingTrx.isValidatingCxrGsaApplicable())
    {
      if (!ietResult) // The following code will be removed when Validating Carrier project is
                      // implemented.
      {
        std::vector<FcMessage>::iterator it = totals->fcMessage.begin();
        std::vector<FcMessage>::iterator itEnd = totals->fcMessage.end();
        for (; it != itEnd; ++it)
        {
          if ((*it).messageContent() == FcMessage::CARRIER_MESSAGE)
            break;
        }
        if (it != itEnd)
        {
          totals->fcMessage.insert(
              it, (FcMessage(FcMessage::WARNING, 0, message, true, FcMessage::INTERLINE_MESSAGE)));
        }
        else
        {
          totals->fcMessage.push_back(
              FcMessage(FcMessage::WARNING, 0, message, true, FcMessage::INTERLINE_MESSAGE));
        }
      }
    }
    paxTypeList.insert(totals->farePath->paxType());

    if (fp->selectedNetRemitFareCombo())
    {
      if (!createNetRemitCalcTotal(pricingTrx, fcConfig, fp, totals))
        return false;
    }

    if (TrxUtil::isCat35TFSFEnabled(pricingTrx) && // check if cat35 tfsf
        nullptr != fp->netFarePath())
    {

      if (!createNetCalcTotal(pricingTrx, fcConfig, fp, totals))
        return false;
    }

    if (fp->adjustedSellingFarePath())
    {
      copyItemsToAdjustedFP(pricingTrx, fp);
      if (!createAdjustedCalcTotal(pricingTrx, fcConfig, *fp->adjustedSellingFarePath(), totals))
        return false;
    }

    totals->farePathInfo.IATASalesCode = getIataSalesCode(fp);
  }

  // Mixed booking - divided party
  collectBkgMessage(pricingTrx, itin);

  // Command Pricing message
  collectCmdPrcMessage(pricingTrx, fcConfig);

  // Create CalcTotals for XO
  for (std::vector<PaxType*>::iterator i = pricingTrx.paxType().begin(),
                                       iend = pricingTrx.paxType().end();
       i != iend;
       ++i)
  {
    PaxType* paxType = *i;
    if (paxTypeList.count(paxType) == 0)
      createCalcTotals(pricingTrx, fcConfig, itin, paxType);
  }

  return true;
}

void
FareCalcCollector::copyFcCommInfoColToAdjustedFP(FarePath& fp) const
{
  if (fp.adjustedSellingFarePath()->pricingUnit().size() == fp.pricingUnit().size())
  {
    for (size_t puId = 0; puId < fp.pricingUnit().size(); ++puId)
    {
      auto origPu = fp.pricingUnit()[puId];
      auto adjPu = fp.adjustedSellingFarePath()->pricingUnit()[puId];

      if (adjPu->fareUsage().size() == origPu->fareUsage().size())
      {
        for (size_t fuId = 0; fuId < origPu->fareUsage().size(); ++fuId)
          adjPu->fareUsage()[fuId]->fcCommInfoCol() = origPu->fareUsage()[fuId]->fcCommInfoCol();
      }
    }
  }
}

void
FareCalcCollector::copyItemsToAdjustedFP(PricingTrx& pricingTrx, FarePath* fp)
{
  fp->adjustedSellingFarePath()->baggageEmbargoesResponse() = fp->baggageEmbargoesResponse();
  fp->adjustedSellingFarePath()->baggageResponse() = fp->baggageResponse();
  fp->adjustedSellingFarePath()->defaultValidatingCarrier() = fp->defaultValidatingCarrier();
  fp->adjustedSellingFarePath()->commissionAmount() = fp->commissionAmount();
  fp->adjustedSellingFarePath()->commissionPercent() = fp->commissionPercent();
  fp->adjustedSellingFarePath()->brandIndex() = fp->brandIndex();

  // Copying Agency Commission information
  fp->adjustedSellingFarePath()->isAgencyCommissionQualifies() = fp->isAgencyCommissionQualifies();
  fp->adjustedSellingFarePath()->valCxrCommissionAmount() = fp->valCxrCommissionAmount();

  fp->adjustedSellingFarePath()->validatingCarriers() = fp->validatingCarriers();
  fp->adjustedSellingFarePath()->defaultValCxrPerSp() = fp->defaultValCxrPerSp();
  fp->adjustedSellingFarePath()->settlementPlanValidatingCxrs()  =    fp->settlementPlanValidatingCxrs();
  fp->adjustedSellingFarePath()->marketingCxrForDefaultValCxrPerSp() =    fp->marketingCxrForDefaultValCxrPerSp();

  if (!fallback::fallbackAMCPhase2(&pricingTrx) && !fp->valCxrCommissionAmount().empty())
  {
    copyFcCommInfoColToAdjustedFP(*fp);
  }

  if (!fallback::fallbackFRROBFeesFixAsl(&pricingTrx))
    fp->adjustedSellingFarePath()->collectedTktOBFees() = fp->collectedTktOBFees();

}

//--------------------------------------------------------------------------
// createCalcTotals
//--------------------------------------------------------------------------
CalcTotals*
FareCalcCollector::createCalcTotals(PricingTrx& pricingTrx,
                                    const FareCalcConfig* fcConfig,
                                    const FarePath* fp)
{
  CalcTotals* totals = getCalcTotals(&pricingTrx, fp, fcConfig);
  if (totals != nullptr && fp->processed() == true)
  {
    FareCalc::FcCollector collector(&pricingTrx, fp, fcConfig, this, totals);
    collector.collect();
  }

  return totals;
}

//--------------------------------------------------------------------------
// createCalcTotals
//      Overloaded version to handle calc totals for XO
//--------------------------------------------------------------------------
CalcTotals*
FareCalcCollector::createCalcTotals(PricingTrx& pricingTrx,
                                    const FareCalcConfig* fcConfig,
                                    const Itin* itin,
                                    const PaxType* paxType)
{
  CalcTotals* calcTotals = findCalcTotals(paxType);
  if (calcTotals != nullptr)
    return calcTotals;

  // Dummy fare path to go with the XO virtual fare path;
  FarePath* farePath = nullptr;
  pricingTrx.dataHandle().get(farePath);
  if (farePath == nullptr)
    throw ErrorResponseException(ErrorResponseException::MEMORY_EXCEPTION);

  farePath->paxType() = const_cast<PaxType*>(paxType);
  farePath->itin() = const_cast<Itin*>(itin);
  return createCalcTotals(pricingTrx, fcConfig, farePath);
}

//----------------------------------------------------------------------------
// getTotalPriceCurrency
//----------------------------------------------------------------------------
CurrencyCode
FareCalcCollector::getTotalPriceCurrency()
{
  if (_calcTotalsMap.size() > 0)
    return _calcTotalsMap.begin()->second->convertedBaseFareCurrencyCode;

  return "";
}

//----------------------------------------------------------------------------
// isMixedBaseFareCurrency
//----------------------------------------------------------------------------
bool
FareCalcCollector::isMixedBaseFareCurrency(bool forNetRemit)
{
  bool isMixedCC = false;
  CurrencyCode cc;

  for (auto& elem : _calcTotalsMap)
  {
    CalcTotals* calcTotals = (forNetRemit ? elem.second->netRemitCalcTotals : elem.second);

    if (calcTotals == nullptr || calcTotals->farePath == nullptr || !calcTotals->farePath->processed())
      continue;

    if (cc.empty())
    {
      cc = calcTotals->convertedBaseFareCurrencyCode;
    }
    else if (calcTotals->convertedBaseFareCurrencyCode != cc)
    {
      isMixedCC = true;
      break;
    }
  }
  return isMixedCC;
}

MoneyAmount
FareCalcCollector::getTotalInternal(PricingTrx& trx,
                                    uint16_t actionCode,
                                    CurrencyCode& currencyCodeUsed,
                                    CurrencyNoDec& noDecUsed,
                                    bool forNetRemit,
                                    const uint16_t brandIndex)
{
  MoneyAmount theTotal = 0.0;

  CalcTotalsMap::iterator iter = _calcTotalsMap.begin();
  CalcTotalsMap::iterator iterEnd = _calcTotalsMap.end();

  for (; iter != iterEnd; ++iter)
  {
    CalcTotals* calcTotals = (forNetRemit ? iter->second->netRemitCalcTotals : iter->second);

    if (!forNetRemit)
    {
      if (trx.getOptions() && !trx.getOptions()->isPDOForFRRule() && iter->second && iter->second->adjustedCalcTotal)
        calcTotals = iter->second->adjustedCalcTotal;
    }

    if (UNLIKELY(nullptr == calcTotals))
    {
      continue;
    }

    CalcTotals& totals = *calcTotals;

    bool brandMatch = false;

    if (UNLIKELY(_validBrandsVecPtr != nullptr &&
        brandIndex != INVALID_BRAND_INDEX)) // Interline Branded Fares Path
    {
      brandMatch = (totals.farePath->getBrandCode() == _validBrandsVecPtr->at(brandIndex));
    }
    else if (UNLIKELY(_isFlexFare))
    {
      brandMatch = totals.farePath->getFlexFaresGroupId() == brandIndex;
    }
    else
    {
      brandMatch = totals.farePath->brandIndex() == brandIndex;
    }

    if (UNLIKELY(!brandMatch))
    {
      continue;
    }

    switch (actionCode)
    {
    // Base fare amount
    case 0:
      theTotal += (totals.convertedBaseFare * totals.farePath->paxType()->number());

      // Some PAX may not be priced with XO.
      if (!totals.convertedBaseFareCurrencyCode.empty())
      {
        currencyCodeUsed = totals.convertedBaseFareCurrencyCode;
        noDecUsed = totals.convertedBaseFareNoDec;
      }
      break;

    // Equiv amount
    case 1:
      theTotal += (totals.equivFareAmount * totals.farePath->paxType()->number());

      // Some PAX may not be priced with XO.
      if (!totals.equivCurrencyCode.empty())
      {
        currencyCodeUsed = totals.equivCurrencyCode;
        noDecUsed = totals.taxNoDec();
      }
      break;

    // Tax amount
    case 2:
      if (!totals.farePath->processed())
      {
        continue;
      }

      theTotal += (totals.taxAmount() * totals.farePath->paxType()->number());
      currencyCodeUsed = totals.taxCurrencyCode();
      noDecUsed = totals.taxNoDec();
      break;

    default:
      LOG4CXX_ERROR(logger, "FareCalcCollector::getTotalInternal - Incorrect action code.");
      break;
    }
  }

  return theTotal;
}

MoneyAmount
FareCalcCollector::getBaseFareTotal(PricingTrx& trx,
                                    CurrencyCode& currencyCodeUsed,
                                    CurrencyNoDec& noDecUsed,
                                    bool forNetRemit,
                                    const uint16_t brandIndex)
{
  return getTotalInternal(trx, 0, currencyCodeUsed, noDecUsed, forNetRemit, brandIndex);
}

MoneyAmount
FareCalcCollector::getEquivFareAmountTotal(PricingTrx& trx,
                                           CurrencyCode& currencyCodeUsed,
                                           CurrencyNoDec& noDecUsed,
                                           bool forNetRemit,
                                           const uint16_t brandIndex)
{
  return getTotalInternal(trx, 1, currencyCodeUsed, noDecUsed, forNetRemit, brandIndex);
}

MoneyAmount
FareCalcCollector::getTaxTotal(PricingTrx& trx,
                               CurrencyCode& currencyCodeUsed,
                               CurrencyNoDec& noDecUsed,
                               bool forNetRemit,
                               const uint16_t brandIndex)
{
  return getTotalInternal(trx, 2, currencyCodeUsed, noDecUsed, forNetRemit, brandIndex);
}

//----------------------------------------------------------------------------
// isMixedEquivFareAmountCurrency
//----------------------------------------------------------------------------
bool
FareCalcCollector::isMixedEquivFareAmountCurrency()
{
  bool isMixedCC = false;

  for (auto& elem : _calcTotalsMap)
  {
    CalcTotals* calcTotals = elem.second;

    if (calcTotals == nullptr || calcTotals->farePath == nullptr || !calcTotals->farePath->processed())
      continue;

    LOG4CXX_DEBUG(logger, "BASE FARE CURRENCY: " << calcTotals->convertedBaseFareCurrencyCode);
    LOG4CXX_DEBUG(logger, "EQUIV AMT FARE CURRENCY: " << calcTotals->equivCurrencyCode);

    if (calcTotals->convertedBaseFareCurrencyCode == calcTotals->equivCurrencyCode)
    {
      isMixedCC = true;
      break;
    }
  }

  return isMixedCC;
}

FareCalculation*
FareCalcCollector::createFareCalculation(PricingTrx* trx, const FareCalcConfig* fcConfig)
{
  // For Abacus WP entry, check if RO applied, in which case we should process
  // the response thru AltFareCalculation to generate the correct response in
  // FareLine format.
  // For 1S the WPA entry only should follow this path
  if (trx->altTrxType() == PricingTrx::WP && !trx->getRequest()->ticketingAgent()->sabre1SUser() &&
      (fcConfig->wpNoMatchPermitted() == FareCalcConsts::FC_YES ||
       fcConfig->wpaPermitted() == FareCalcConsts::FC_YES) &&
      !trx->getRequest()->isLowFareRequested() && !trx->getRequest()->isLowFareNoAvailability())
  {
    for (FareCalcCollector::CalcTotalsMap::const_iterator i = _calcTotalsMap.begin(),
                                                          iend = _calcTotalsMap.end();
         i != iend;
         ++i)
    {
      if (hasNoMatch())
      {
        AltFareCalculation* fareCalculation = nullptr;
        fareCalculation = trx->dataHandle().create<AltFareCalculation>();

        if (fareCalculation != nullptr)
        {
          fareCalculation->initialize(trx, fcConfig, this);
        }

        trx->altTrxType() = PricingTrx::WP_NOMATCH;
        return fareCalculation;
      }
    }
  }

  if (trx->isRfbListOfSolutionEnabled() && FareCalcUtil::isOneSolutionPerPaxType(trx))
    trx->setRfblistOfSolution(false);

  if (trx->isRfbListOfSolutionEnabled())
  {
    AltFareCalculation* fareCalculation = 0;
    fareCalculation = trx->dataHandle().create<AltFareCalculation>();

    if (fareCalculation != 0)
    {
      fareCalculation->initialize(trx, fcConfig, this);
    }
    return fareCalculation;
  }


  FareCalculation* fareCalculation = 0;
  fareCalculation = trx->dataHandle().create<FareCalculation>();

  if (LIKELY(fareCalculation != nullptr))
    fareCalculation->initialize(trx, fcConfig, this);

  return fareCalculation;
}

CalcTotals*
FareCalcCollector::findCalcTotals(const FarePath* fp) const
{
  CalcTotalsMap::const_iterator i = _calcTotalsMap.find(fp);
  if (i == _calcTotalsMap.end())
  {
    return nullptr;
  }
  return i->second;
}

// This is needed to handle the XO case:
CalcTotals*
FareCalcCollector::findCalcTotals(const PaxType* paxType, const uint16_t brandIndex) const
{
  for (const auto& elem : _calcTotalsMap)
  {
    if ((_isFlexFare && (elem.first->paxType() == paxType) &&
         (elem.first->getFlexFaresGroupId() == brandIndex)) ||
        ((elem.first->paxType() == paxType) && (elem.first->brandIndex() == brandIndex)))
      return elem.second;
  }

  return nullptr;
}

void
FareCalcCollector::addCalcTotals(const FarePath* fp, CalcTotals* ct)
{
  if (LIKELY(ct != nullptr))
  {
    _calcTotalsMap.insert(std::make_pair(fp, ct));

    // FIXME: remove when done with refactoring - save it here so that
    // XML still see it in its old place.
    _passengerCalcTotals.push_back(ct);
  }
}

CalcTotals*
FareCalcCollector::getCalcTotals(PricingTrx* trx,
                                 const FarePath* fp,
                                 const FareCalcConfig* fcConfig)
{
  bool isNetRemitFp = (dynamic_cast<const NetRemitFarePath*>(fp) != nullptr);

  bool isNetFp = (dynamic_cast<const NetFarePath*>(fp) != nullptr);

  // Axess is using the 'normal FarePath".
  if (trx->getRequest()->ticketingAgent()->axessUser() && trx->getRequest()->isWpNettRequested() &&
      fp->originalFarePathAxess())
    isNetRemitFp = true;

  CalcTotals* calcTotals = nullptr;
  if (LIKELY(!isNetRemitFp && !isNetFp && !fp->isAdjustedSellingFarePath()))
  {
    calcTotals = findCalcTotals(fp);
  }

  if (calcTotals != nullptr)
  {
    return calcTotals;
  }
  else
  {
    _dataHandle.get(calcTotals);

    calcTotals->fcConfig = fcConfig;
    calcTotals->farePath = fp;
    const PaxType* paxType = fp->paxType();
    calcTotals->truePaxType = paxType->paxType();
    calcTotals->requestedPaxType = paxType->paxType();
    if (calcTotals->requestedPaxType.size() == 3 && calcTotals->requestedPaxType[1] == 'N' &&
        calcTotals->requestedPaxType[2] == 'N' && paxType->age() > 0 && paxType->age() < 100)
    {
      calcTotals->requestedPaxType[1] = '0' + (paxType->age() / 10);
      calcTotals->requestedPaxType[2] = '0' + (paxType->age() % 10);
    }

    calcTotals->formattedFareCalcLine.initialize(trx, fp, fcConfig, this, calcTotals);
    calcTotals->getMutableFcTaxInfo().initialize(trx, calcTotals, fcConfig, nullptr);

    if (LIKELY(!isNetRemitFp && !isNetFp && !fp->isAdjustedSellingFarePath()))
    {
      addCalcTotals(fp, calcTotals);
    }

    return calcTotals;
  }
}

const std::string&
FareCalcCollector::getIataSalesCode(const FarePath* fp)
{
  if (LIKELY(fp != nullptr))
  {
    switch (fp->intlSaleIndicator())
    {
    case Itin::SITI:
      _IATASalesCode = "SITI";
      break;
    case Itin::SITO:
      _IATASalesCode = "SITO";
      break;
    case Itin::SOTI:
      _IATASalesCode = "SOTI";
      break;
    case Itin::SOTO:
      _IATASalesCode = "SOTO";
      break;
    default:
      _IATASalesCode = "UNKNOWN";
      break;
    }
  }

  return _IATASalesCode;
}

bool
FareCalcCollector::isFareCalcTooLong()
{
  CalcTotalsMap::const_iterator i = _calcTotalsMap.begin();
  for (; i != _calcTotalsMap.end(); ++i)
  {
    if (i->second->fclToLong)
      return true;
  }

  return false;
}

void
FareCalcCollector::refreshTaxResponse(PricingTrx& trx, Itin& itin, const FareCalcConfig& fcConfig)
{
  if (LIKELY(trx.getTrxType() == PricingTrx::MIP_TRX))
  {
    return;
  }
  if ((fcConfig.taxPlacementInd() ==
       FareCalcConsts::FC_THREE) && // Per Sterling, only 1 and 3 exists for Tax Placement
      (trx.getRequest()->isExemptSpecificTaxes() || trx.getRequest()->isExemptAllTaxes()) &&
      trx.getRequest()->taxOverride().empty())
  {
    // Sort Tax Response including Tax Exempts based on origin nation taxes first followed
    // by next nation transited in the trx.

    // Clear TaxResponse in the trx so it can be rebuilt.
    std::vector<TaxResponse*>::const_iterator taxRespI = itin.getTaxResponses().begin();
    for (; taxRespI != itin.getTaxResponses().end(); ++taxRespI)

    {
      TaxRecord taxRecord;
      taxRecord.buildTicketLine(trx, **taxRespI, false, true);
    }
  }
}

bool
FareCalcCollector::hasNoMatch() const
{
  for (const auto elem : _calcTotalsMap)
  {
    if (!elem.second->farePath->processed())
    {
      return true;
    }
  }
  return false;
}

void
FareCalcCollector::collectBkgMessage(PricingTrx& trx, Itin* itin)
{
  if (LIKELY(((!trx.getRequest()->isLowFareRequested() && !trx.getRequest()->isLowFareNoAvailability()) ||
       (trx.getTrxType() == PricingTrx::MIP_TRX)) ||
      trx.getOptions()->returnAllData() == NCB))
  {
    if(trx.getOptions()->returnAllData() == NCB &&
       !fallback::fallbackPriceByCabinActivation(&trx) && !trx.getOptions()->cabin().isUndefinedClass())
      collectWPNCBPriceByCabinTrailerMessage(trx, itin);

    return;
  }

  std::set<PaxType*, PaxType::InputOrder> inOrderPaxType(trx.paxType().begin(),
                                                         trx.paxType().end());

  bool firstPaxType = true;
  CalcTotals* firstCalcTotals = nullptr;

  for (const auto& elem : inOrderPaxType)
  {

    if (trx.altTrxType() != PricingTrx::WP)
    {
      firstPaxType = true;
      firstCalcTotals = nullptr;
    }

    CalcTotals* calcTotals = nullptr;

    for (FareCalcCollector::CalcTotalsMap::const_iterator i = _calcTotalsMap.begin(),
                                                          iend = _calcTotalsMap.end();
         i != iend;
         ++i)
    {
      calcTotals = i->second;

      if (calcTotals->farePath->paxType() != elem || !calcTotals->farePath->processed() ||
          calcTotals->bookingCodeRebook.empty())
      {
        continue;
      }

      if (firstPaxType)
      {
        firstCalcTotals = i->second;
        firstPaxType = false;
      }

      calcTotals = i->second;

      if (!calcTotals || !calcTotals->farePath || !calcTotals->farePath->processed())
        continue;

      for (FareCalcCollector::CalcTotalsMap::const_iterator i = _calcTotalsMap.begin(),
                                                            iend = _calcTotalsMap.end();
           i != iend && trx.altTrxType() == PricingTrx::WP;
           ++i)
      {
        if (calcTotals != firstCalcTotals && !calcTotals->bookingCodeRebook.empty() &&
            calcTotals->bookingCodeRebook != i->second->bookingCodeRebook)
        {
          // if booking codes are not same
          calcTotals->fcMessage.push_back(FcMessage(FcMessage::WARNING,
                                                    0,
                                                    "LOW FARE REQS DIVIDE PARTY.  FOR *" +
                                                        calcTotals->farePath->paxType()->paxType() +
                                                        "* PSGR TYPE"));

          if (calcTotals->adjustedCalcTotal)
            calcTotals->adjustedCalcTotal->fcMessage.push_back(FcMessage(FcMessage::WARNING,
                                                    0,
                                                    "LOW FARE REQS DIVIDE PARTY.  FOR *" +
                                                        calcTotals->farePath->paxType()->paxType() +
                                                        "* PSGR TYPE"));

          break;
        }
      }

      if (!calcTotals->bookingCodeRebook.empty())
      {
        collectBkgRebookMessage(trx, itin, calcTotals);
        if (calcTotals->adjustedCalcTotal)
          collectBkgRebookMessage(trx, itin,  calcTotals->adjustedCalcTotal);
      }
    }
  }
}

void
FareCalcCollector::collectBkgRebookMessage(PricingTrx& trx, Itin* itin, CalcTotals* calcTotals)
{
  std::ostringstream bkCodes;

  for (int i = 0, n = calcTotals->bookingCodeRebook.size(); i < n; i++)
  {
    if (calcTotals->bookingCodeRebook[i].empty())
      continue;

    bkCodes << " " << itin->travelSeg()[i]->pnrSegment() << calcTotals->bookingCodeRebook[i][0];
  }

  if (!bkCodes.str().empty())
  {
    std::ostringstream outLine;

    if (trx.altTrxType() != PricingTrx::WP)
    {
      const FcConfig* fcConfig = FcConfig::create(&trx, FareCalcUtil::getFareCalcConfig(trx));

      // APPLICABLE BOOKING CLASS -  1x 2y
      // CHANGE BOOKING CLASS -  1x 2y
      std::string noMatchBookingClass;
      if (fcConfig->getMsgAppl(FareCalcConfigText::WPA_NO_MATCH_BOOKING_CLASS, noMatchBookingClass))
      {
        outLine << noMatchBookingClass << " - " << bkCodes.str();
        calcTotals->fcMessage.push_back(FcMessage(FcMessage::WARNING, 0, outLine.str()));
      }

      // REBOOK OPTION OF CHOICE BEFORE STORING FARE
      if (trx.altTrxType() != PricingTrx::WP && trx.getRequest()->lowFareRequested() == 'T')
      {
        std::string noMatchRebook;
        if ((trx.altTrxType() == PricingTrx::WPA &&
             fcConfig->getMsgAppl(FareCalcConfigText::WPA_NO_MATCH_REBOOK, noMatchRebook)) ||
            (fcConfig->getMsgAppl(FareCalcConfigText::WP_NO_MATCH_REBOOK, noMatchRebook)))
        {
          calcTotals->fcMessage.push_back(FcMessage(FcMessage::WARNING, 0, noMatchRebook));
        }
      }
    }
    else
    {
      if (trx.getRequest()->isLowFareNoAvailability()) // WPNCS
      {
        outLine << "APPLICABLE BOOKING CLASS -  " << bkCodes.str();
        calcTotals->fcMessage.push_back(FcMessage(FcMessage::WARNING, 0, outLine.str()));
      }
      else if (trx.getRequest()->isLowFareRequested()) // WPNC
      {
        if (trx.startShortCutPricingItin() == 0)
        {
          outLine << "CHANGE BOOKING CLASS -  " << bkCodes.str();
          calcTotals->fcMessage.push_back(FcMessage(FcMessage::WARNING, 0, outLine.str()));
        }
      }
    }
  }
  collectPriceByCabinMessage(trx, itin, calcTotals);
}

void
FareCalcCollector::collectCmdPrcMessage(PricingTrx& trx, const FareCalcConfig* fcConfig)
{
  if (trx.excTrxType() == PricingTrx::PORT_EXC_TRX)
  {
    if (!(static_cast<ExchangePricingTrx&>(trx)).exchangeOverrides().dummyFCSegs().empty())
      return; // for dummy fare cases, we do not collect cmd prc msg
  }

  FareCalcCollector::CalcTotalsMap::const_iterator i = calcTotalsMap().begin();
  FareCalcCollector::CalcTotalsMap::const_iterator iend = calcTotalsMap().end();

  if (!hasNoMatch() && calcTotalsMap().size() == trx.paxType().size())
  {
    PaxTypeFare::PaxTypeFareCPFailedStatus cpFStatus;
    for (; i != iend; ++i)
    {
      cpFStatus.combine(i->second->farePathInfo.cpFailedStatus);
    }

    // TODO: we need to work out where to store the message if the messages
    // (possibly other than in PXI) if the message applied to all pax types.
    // for now store on the last calc totals
    CalcTotals* calcTotals = findCalcTotals(trx.paxType().back());
    if (!calcTotals)
      calcTotals = calcTotalsMap().rbegin()->second;

    collectCmdPrcMessage(trx, fcConfig, cpFStatus, calcTotals);

    if (calcTotals->adjustedCalcTotal)
      collectCmdPrcMessage(trx, fcConfig, cpFStatus, calcTotals->adjustedCalcTotal);
  }
  else
  {
    for (; i != iend; ++i)

    {
      CalcTotals* calcTotals = i->second;
      collectCmdPrcMessage(trx, fcConfig, calcTotals->farePathInfo.cpFailedStatus, calcTotals);

      if (calcTotals->adjustedCalcTotal)
        collectCmdPrcMessage(trx, fcConfig, calcTotals->farePathInfo.cpFailedStatus,
                             calcTotals->adjustedCalcTotal);
    }
  }
}

void
FareCalcCollector::collectCmdPrcMessage(PricingTrx& trx,
                                        const FareCalcConfig* fcConfig,
                                        const PaxTypeFare::PaxTypeFareCPFailedStatus cpFStatus,
                                        CalcTotals* calcTotals)
{
  // if no rule Failed then do not display any warnings
  if (LIKELY(!(cpFStatus.isSet(PaxTypeFare::PTFF_ALL)) || calcTotalsMap().empty() ||
      trx.getRequest()->isSFR()))
    return;

  if (!fallback::fallbackRrmCmdPricFixErrorMsg(&trx) &&
      trx.getRequest()->prmValue())
  {
    std::set<std::string> retailerCodes;
    AdjustedSellingUtil::getAllRetailerCodeFromFRR(*calcTotals, retailerCodes);
    if (retailerCodes.empty())
      throw ErrorResponseException(ErrorResponseException::NO_RETAILER_RULE_QUALIFIER_FARES_EXISTS);
  }

  FareCalc::FcStream cpMsg;
  std::string warning(" ");
  if (fcConfig->warningMessages() == FareCalcConsts::FC_YES)
    warning = " ATTN*";

  cpMsg << '\n' << "**" << '\n' << "PRICED USING RULE OVERRIDE-FOLLOWING FARE RULES NOT MET"
        << '\n';

  if (cpFStatus.isSet(PaxTypeFare::PTFF_Cat1))
    cpMsg << warning << " ELIGIBILITY RESTRICTIONS\n";

  if (cpFStatus.isSet(PaxTypeFare::PTFF_Cat2))
    cpMsg << warning << " DAY/TIME RESTRICTIONS\n";

  if (cpFStatus.isSet(PaxTypeFare::PTFF_Cat3))
    cpMsg << warning << " SEASON RESTRICTIONS\n";

  if (cpFStatus.isSet(PaxTypeFare::PTFF_Cat4))
    cpMsg << warning << " FLIGHT APP RESTRICTIONS\n";

  if (cpFStatus.isSet(PaxTypeFare::PTFF_Cat5))
    cpMsg << warning << " ADVANCE RES/TICKETING RESTRICTIONS\n";

  if (cpFStatus.isSet(PaxTypeFare::PTFF_Cat6))
    cpMsg << warning << " MINSTAY RESTRICTIONS\n";

  if (cpFStatus.isSet(PaxTypeFare::PTFF_Cat7))
    cpMsg << warning << " MAXSTAY RESTRICTIONS\n";

  if (cpFStatus.isSet(PaxTypeFare::PTFF_Cat8))
    cpMsg << warning << " STOPOVERS RESTRICTIONS\n";

  if (cpFStatus.isSet(PaxTypeFare::PTFF_Cat9))
    cpMsg << warning << " TRANSFERS RESTRICTIONS\n";

  if (cpFStatus.isSet(PaxTypeFare::PTFF_Cat10))
    cpMsg << warning << " COMBINABILITY REQUIREMENTS\n";

  if (cpFStatus.isSet(PaxTypeFare::PTFF_Cat11))
    cpMsg << warning << " BLACKOUT DATES RESTRICTIONS\n";

  if (cpFStatus.isSet(PaxTypeFare::PTFF_Cat12))
    cpMsg << warning << " SURCHARGES RESTRICTIONS\n";

  if (cpFStatus.isSet(PaxTypeFare::PTFF_Cat13))
    cpMsg << warning << " ACCOMPNIED TRAVEL RESTRICTIONS\n";

  if (cpFStatus.isSet(PaxTypeFare::PTFF_Cat14))
    cpMsg << warning << " TRAVEL RESTRICTIONS\n";

  if (cpFStatus.isSet(PaxTypeFare::PTFF_Cat15))
    cpMsg << warning << " SALES RESTRICTIONS\n";

  if (cpFStatus.isSet(PaxTypeFare::PTFF_Cat16))
    cpMsg << warning << " PENALTIES RESTRICTIONS\n";

  if (cpFStatus.isSet(PaxTypeFare::PTFF_Cat17))
    cpMsg << warning << " CAT17 RESTRICTIONS\n";

  if (cpFStatus.isSet(PaxTypeFare::PTFF_Cat18))
    cpMsg << warning << " TICKET ENDORSEMENTS RESTRICTIONS\n";

  if (cpFStatus.isSet(PaxTypeFare::PTFF_Cat19))
    cpMsg << warning << " CHILDREN DISCOUNT RESTRICTIONS\n";

  if (cpFStatus.isSet(PaxTypeFare::PTFF_Cat20))
    cpMsg << warning << " TOUR CONDUCTOR DISCOUNT RESTRICTIONS\n";

  if (cpFStatus.isSet(PaxTypeFare::PTFF_Cat21))
    cpMsg << warning << " AGENT DISCOUNT RESTRICTIONS\n";

  if (cpFStatus.isSet(PaxTypeFare::PTFF_Cat22))
    cpMsg << warning << " OTHER DISCOUNTS RESTRICTIONS\n";

  if (cpFStatus.isSet(PaxTypeFare::PTFF_Cat23))
    cpMsg << warning << " MISC FARE TAGS RESTRICTIONS\n";

  if (cpFStatus.isSet(PaxTypeFare::PTFF_Cat24))
    cpMsg << warning << " CAT24 RESTRICTIONS\n";

  if (cpFStatus.isSet(PaxTypeFare::PTFF_Cat25))
    cpMsg << warning << " FARE BY RULE RESTRICTIONS\n";

  if (cpFStatus.isSet(PaxTypeFare::PTFF_Cat35))
    cpMsg << warning << " NEG FARE RESTRICTIONS\n";

  if (cpFStatus.isSet(PaxTypeFare::PTFF_RBD))
    cpMsg << warning << " INCORRECT BOOKING CLASS\n";

  if (cpFStatus.isSet(PaxTypeFare::PTFF_R1UNAVAIL))
    cpMsg << warning << " RULE RECORD NOT VALID\n";

  if (cpFStatus.isSet(PaxTypeFare::PTFF_CURR_SEL))
    cpMsg << warning << " CURRENCY SELECTION\n";

  if (cpFStatus.isSet(PaxTypeFare::PTFF_MISSED_FTNOTE))
    cpMsg << warning << " MISSED FOOTNOTE\n";

  cpMsg << "FARE NOT GUARANTEED IF TICKETED" << '\n' << "**" << '\n';

  std::vector<std::string> lines;
  for (int i = 0, n = cpMsg.split(lines); i < n; i++)
  {
    if (!lines[i].empty())
      calcTotals->fcMessage.push_back(FcMessage(FcMessage::WARNING, 0, lines[i], false));
  }
}

void
FareCalcCollector::addMultiMessage(PricingTrx& trx,
                                   const FarePath* farePath,
                                   const FcMessage& message)
{
  std::map<std::string, FcMultiMessage>::iterator iter = _multiMessage.find(message.messageText());

  if (iter == _multiMessage.end())
  {
    FcMultiMessage multiMsg(message.messageType(),
                            message.messageCode(),
                            message.airlineCode(),
                            message.messageText(),
                            message.messagePrefix());
    multiMsg.addFarePath(farePath);
    _multiMessage.insert(std::make_pair(message.messageText(), multiMsg));
  }
  else
  {
    FcMultiMessage& msg = iter->second;
    msg.addFarePath(farePath);
  }
}

void
FareCalcCollector::processConsolidatorPlusUp(PricingTrx& pricingTrx, Itin*& itin, FarePath*& fp)
{
  DiagManager diagManager(pricingTrx, Diagnostic864);
  DiagCollector* diag = diagManager.isActive() ? &diagManager.collector() : nullptr;

  itin->consolidatorPlusUp()->addPlusUpToFarePath(pricingTrx, fp, diag);

  if (diag)
    diag->flushMsg();
}

bool
FareCalcCollector::createNetRemitCalcTotal(PricingTrx& pricingTrx,
                                           const FareCalcConfig* fcConfig,
                                           FarePath* fp,
                                           CalcTotals* totals)
{
  NetRemitFarePath* netRemitFp = fp->netRemitFarePath();
  if (netRemitFp != nullptr)
  {
    refreshTaxResponse(pricingTrx, *netRemitFp->itin(), *fcConfig);
    CalcTotals* netRemitTotals = createCalcTotals(pricingTrx, fcConfig, netRemitFp);
    if (netRemitTotals == nullptr)
    {
      LOG4CXX_ERROR(logger, "Cannot find Net Remit fare path's calc totals");
      return false;
    }
    totals->netRemitCalcTotals = netRemitTotals;
  }
  // Jal/Axess
  else if (fp->axessFarePath())
  {
    FarePath* fpAxess = fp->axessFarePath();
    refreshTaxResponse(pricingTrx, *fpAxess->itin(), *fcConfig);
    CalcTotals* netRemitTotals = createCalcTotals(pricingTrx, fcConfig, fpAxess);
    if (netRemitTotals == nullptr)
    {
      LOG4CXX_ERROR(logger, "Cannot find Net Remit Axess fare path's calc totals");
      return false;
    }
    totals->netRemitCalcTotals = netRemitTotals;
  }
  return true;
}

bool
FareCalcCollector::createAdjustedCalcTotal(PricingTrx& pricingTrx,
                                           const FareCalcConfig* fcConfig,
                                           const FarePath& adjustedSellingFarePath,
                                           CalcTotals* totals)
{
  CalcTotals* adjustedCalcTotal = createCalcTotals(pricingTrx, fcConfig, &adjustedSellingFarePath);
  if (adjustedCalcTotal == nullptr)
  {
    LOG4CXX_ERROR(logger, "Cannot find adjusted selling fare path's calc totals");
    return false;
  }

  if (pricingTrx.getOptions()->isMslRequest())
  {
    adjustedCalcTotal->equivFareAmount = totals->equivFareAmount +
                                         totals->farePath->paxType()->mslAmount();

    if (totals->convertedBaseFareCurrencyCode == totals->equivCurrencyCode)
      adjustedCalcTotal->convertedBaseFare = totals->convertedBaseFare +
                                             totals->farePath->paxType()->mslAmount();
  }

  totals->adjustedCalcTotal = adjustedCalcTotal;

  processAdjustedSellingDiffInfo(pricingTrx, *totals);

  return true;
}

void
FareCalcCollector::processAdjustedSellingDiffInfo(PricingTrx& trx, CalcTotals& calcTotals)
{
  MoneyAmount diffAmount = getAdjustedSellingDifference(trx, calcTotals);
  Money moneyEquivA(diffAmount, calcTotals.equivCurrencyCode);
  int noDec = moneyEquivA.noDec(trx.ticketingDate());
  std::ostringstream oss;
  oss << std::left << std::fixed << std::setprecision(noDec) << diffAmount;

  CalcTotals& act = *calcTotals.adjustedCalcTotal;
  act.adjustedSellingDiffInfo.emplace_back("ADJT AMT", "J", oss.str());

  if (calcTotals.taxRecords().size() != act.taxRecords().size())
  {
    LOG4CXX_ERROR(logger, "Missing tax records for adjusted selling fare path");
    return;
  }

  std::vector<TaxRecord*>::const_iterator taxRecordIterOrig = calcTotals.taxRecords().begin();
  std::vector<TaxRecord*>::const_iterator taxRecordIterOrigEnd = calcTotals.taxRecords().end();
  std::vector<TaxRecord*>::const_iterator taxRecordIterAdj = act.taxRecords().begin();

  for (; taxRecordIterOrig != taxRecordIterOrigEnd; ++taxRecordIterOrig, ++taxRecordIterAdj)
  {
    if ((*taxRecordIterAdj)->gstTaxInd() &&
        (*taxRecordIterOrig)->gstTaxInd() &&
        ((*taxRecordIterAdj)->taxCode() == (*taxRecordIterOrig)->taxCode()))
    {
      diffAmount = (*taxRecordIterAdj)->getTaxAmount() - (*taxRecordIterOrig)->getTaxAmount();
      oss.str("");
      oss << diffAmount;

      std::string taxCodeAbbr = (*taxRecordIterAdj)->taxCode().substr(0,2);
      act.adjustedSellingDiffInfo.emplace_back(taxCodeAbbr, "G", oss.str());
    }
  }

  // For MSL, add the difference between ASL and MSL
  if (trx.getOptions()->isMslRequest() &&
      PricingUtil::adjustedSellingCalcDataExists(*act.farePath))
  {
    oss.str("");
    oss << std::left << std::fixed << std::setprecision(noDec) << act.farePath->aslMslDiffAmount();
    act.adjustedSellingDiffInfo.emplace_back("DIFF MS AS", "M", oss.str());
  }
}

MoneyAmount
FareCalcCollector::getAdjustedSellingDifference(PricingTrx& trx, CalcTotals& calcTotals)
{
  CalcTotals& act = *calcTotals.adjustedCalcTotal;

    return (act.equivFareAmount - calcTotals.equivFareAmount);
}

bool
FareCalcCollector::createNetCalcTotal(PricingTrx& pricingTrx,
                                      const FareCalcConfig* fcConfig,
                                      FarePath* fp,
                                      CalcTotals* totals)
{
  NetFarePath* netFp = fp->netFarePath();
  if (netFp != nullptr)
  {
    CalcTotals* netTotals = createCalcTotals(pricingTrx, fcConfig, netFp);
    if (netTotals == nullptr)
    {
      LOG4CXX_ERROR(logger, "Cannot find Net fare path's calc totals");
      return false;
    }
    totals->netCalcTotals = netTotals;
  }
  return true;
}

void
FareCalcCollector::reorderPaxTypes(PricingTrx& pricingTrx) const
{
  if (LIKELY(pricingTrx.getTrxType() != PricingTrx::PRICING_TRX))
    return;

  // FIXME: remove this code when we resolve the issue with PD
  //        record ptc ordering.
  if (pricingTrx.paxType().size() > 1 && pricingTrx.paxType().back() &&
      pricingTrx.paxType().back()->inputOrder() == 0)
  {
    for (int i = 0, n = pricingTrx.paxType().size(); i < n; i++)
    {
      pricingTrx.paxType()[i]->inputOrder() = i;
    }
  }
}

void
FareCalcCollector::collectPriceByCabinMessage(PricingTrx& trx, Itin* itin, CalcTotals* calcTotals)
{
  if (!fallback::fallbackPriceByCabinActivation(&trx) &&
      !trx.getRequest()->isjumpUpCabinAllowed())
  {
    CabinType cabin = trx.getOptions()->cabin();
    std::vector<std::string>segOrder;

    const std::vector<TravelSeg*>& ts = itin->travelSeg();
    for (const auto tvlSeg : ts)
    {
      if (tvlSeg && tvlSeg->isAir())
      {
        if (tvlSeg->bookedCabin() != cabin)
        {
          segOrder.push_back(boost::lexical_cast<std::string>(tvlSeg->pnrSegment()));
        }
      }
    }
    if (!segOrder.empty())
    {
      std::string lineToDisplay = "";

      if (cabin.isPremiumFirstClass())
        lineToDisplay = "PB ";
      else if (cabin.isFirstClass())
        lineToDisplay = "FB ";
      else if (cabin.isPremiumBusinessClass())
        lineToDisplay = "JB ";
      else if (cabin.isBusinessClass())
        lineToDisplay = "BB ";
      else if (cabin.isPremiumEconomyClass())
        lineToDisplay = "SB ";
      else if (cabin.isEconomyClass())
        lineToDisplay = "YB ";

      lineToDisplay += "CABIN NOT OFFERED - ";
      if (segOrder.size() > 1 )
        lineToDisplay += "SEGS ";
      else
        lineToDisplay += "SEG ";

      for (auto& seg : segOrder)
      {
        lineToDisplay += seg;
        lineToDisplay += "/";
      }
      lineToDisplay.erase(lineToDisplay.end() - 1);

      calcTotals->fcMessage.push_back(
               FcMessage(FcMessage::WARNING, 0, lineToDisplay, true, FcMessage::REMAINING_WARNING_MSG));
    }
  }
}

void
FareCalcCollector::collectWPNCBPriceByCabinTrailerMessage(PricingTrx& trx, Itin* itin)
{
  std::set<PaxType*, PaxType::InputOrder> inOrderPaxType(trx.paxType().begin(),
                                                         trx.paxType().end());
  for (const auto& elem : inOrderPaxType)
  {
    CalcTotals* calcTotals = nullptr;

    for (FareCalcCollector::CalcTotalsMap::const_iterator i = _calcTotalsMap.begin(),
                                                          iend = _calcTotalsMap.end();
         i != iend;
         ++i)
    {
      calcTotals = i->second;

      if (calcTotals->farePath->paxType() != elem || !calcTotals->farePath->processed() ||
          calcTotals->bookingCodeRebook.empty())
      {
        continue;
      }

      calcTotals = i->second;

      if (!calcTotals || !calcTotals->farePath || !calcTotals->farePath->processed())
        continue;

      if (!calcTotals->bookingCodeRebook.empty())
      {
        collectPriceByCabinMessage(trx, itin, calcTotals);
        if (calcTotals->adjustedCalcTotal)
          collectPriceByCabinMessage(trx, itin,  calcTotals->adjustedCalcTotal);
      }
    }
  }
}

} // tse namespace
