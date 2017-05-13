//-------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "Common/SpanishResidentFaresEnhancementUtil.h"


#include "Common/Config/ConfigurableValue.h"
#include "Common/ConfigList.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/FallbackUtil.h"
#include "Common/GoverningCarrier.h"
#include "Common/LocUtil.h"
#include "Common/SpanishLargeFamilyUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/CustomerActivationControl.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/Diag666Collector.h"
#include "Diagnostic/DiagManager.h"
#include "Pricing/FareMarketPath.h"
#include "Pricing/MergedFareMarket.h"
#include "Pricing/PUPath.h"
#include "Pricing/YFlexiValidator.h"
#include "Util/BranchPrediction.h"

#include <set>

namespace tse
{
FALLBACK_DECL(fixSpanishLargeFamilyForSRFE);
FALLBACK_DECL(srfeSsdsp2040DifferentDiscountTypes);
FALLBACK_DECL(srfeFareBasisSuffix);
FALLBACK_DECL(srfeFareBasisSuffixOldWay);

namespace
{
ConfigurableValue<ConfigSet<StateCode>>
SRFDTerritories("SRFE", "SPANISH_TERRITORIES");
}

namespace SRFEUtil
{
StateCode
mapCityToState(LocCode residencyCity)
{
  if (residencyCity == LOC_TFN || residencyCity == LOC_TFS)
    residencyCity = LOC_TCI;

  if ((residencyCity == LOC_PMI) || (residencyCity == LOC_IBZ) || (residencyCity == LOC_MAH))
    return ST_RBP;
  else if ((residencyCity == LOC_TCI) || (residencyCity == LOC_LPA) || (residencyCity == LOC_ACE) ||
           (residencyCity == LOC_FUE) || (residencyCity == LOC_SPC))
    return ST_RRC;
  else if (residencyCity == LOC_MLN)
    return ST_RRM;
  else if (residencyCity == LOC_JCU)
    return ST_RCE;
  return "";
}

bool
isPassengerApplicable(LocCode residencyCity, StateCode& residencyState)
{
  residencyState = mapCityToState(residencyCity);
  return SRFDTerritories.getValue().has(residencyState);
}

bool
isApplicableForPOS(const Agent& agent)
{
  return LocUtil::isSpain(*agent.agentLocation());
}

bool
isItinApplicable(const Itin& itin)
{
  return !itin.travelSeg().empty() && std::all_of(
      itin.travelSeg().begin(),
      itin.travelSeg().end(),
      [](const auto* ts)
      { return ts->origin()->nation() == SPAIN && ts->destination()->nation() == SPAIN; });
}

StateCode
mapCityToStateItin(const LocCode& airportCity)
{
  if (airportCity == LOC_AGP || airportCity == LOC_XRY || airportCity == LOC_SVQ)
    return ST_RCE;
  return mapCityToState(airportCity);
}

bool
hasSpanishGovArunk(const FareMarketPath& fmp)
{
  for (const MergedFareMarket* mfm : fmp.fareMarketPath())
  {
    TravelSeg* prev = nullptr;
    for (TravelSeg* const travelSeg : mfm->travelSeg())
    {
      if (!travelSeg->isAir() ||
          (prev && prev->destination() != travelSeg->origin()))
        return true;
      prev = travelSeg;
    }
  }

  return false;
}

std::set<CarrierCode>
getGoverningCarriers(const FareMarketPath& fmp)
{
  std::set<CarrierCode> carriers;

  for (const MergedFareMarket* mfm : fmp.fareMarketPath())
    carriers.insert(mfm->governingCarrier().begin(), mfm->governingCarrier().end());

  return carriers;
}

bool
hasGovCarriersActive(PricingTrx& trx, const FareMarketPath& fmp)
{
  auto governingCarriers = getGoverningCarriers(fmp);
  return std::all_of(governingCarriers.begin(),
                     governingCarriers.end(),
                     [&trx](auto carrier)
                     {
                       return trx.isCustomerActivatedByFlag("SRF", &carrier);
                     });
}

bool
hasLongConnection(const FareMarketPath& fmp)
{
  constexpr int64_t secondsIn12Hours = 12*60*60;

  for (const MergedFareMarket* mfm : fmp.fareMarketPath())
    for (auto itTS = mfm->travelSeg().begin(); itTS != mfm->travelSeg().end(); ++itTS)
    {
      auto nextTS = std::next(itTS);
      if (nextTS != mfm->travelSeg().end())
      {
        if (DateTime::diffTime((*nextTS)->departureDT(), (*itTS)->arrivalDT())
            > secondsIn12Hours)
          return true;
      }
    }
  return false;
}

bool
hasValidResidency(const PricingTrx& trx, const FareMarketPath& fmp, StateCode residencyState)
{
  if (residencyState.empty())
    return false;

  bool withinResidency = false;
  bool fromOrToResidency = false;

  for (const MergedFareMarket* mfm : fmp.fareMarketPath())
  {
    const StateCode& stateOff = mapCityToStateItin(mfm->offMultiCity());
    const StateCode& stateBoard = mapCityToStateItin(mfm->boardMultiCity());

    if (!fallback::srfeSsdsp2040DifferentDiscountTypes(&trx))
    {
      if (stateOff == residencyState && stateBoard == residencyState)
        withinResidency = true;
      else if (stateOff == residencyState || stateBoard == residencyState)
        fromOrToResidency = true;
      else
        return false;
    }
    else
    {
      if (stateOff != residencyState && stateBoard != residencyState)
        return false;
    }
  }

  if (withinResidency && fromOrToResidency)
    return false;

  return true;
}

bool
isSolutionPatternApplicable(PricingTrx& trx, const FareMarketPath& fmp)
{
  return fmp.fareMarketPath().size() <= 2 &&
         hasValidResidency(trx, fmp, trx.residencyState()) &&
         hasGovCarriersActive(trx, fmp) &&
         !hasLongConnection(fmp) &&
         !hasSpanishGovArunk(fmp);
}

void
initSpanishResidentFares(PUPath& puPath, PricingTrx& trx, const Itin& itin)
{
  if (isSolutionPatternApplicable(trx, *puPath.fareMarketPath()))
  {
    YFlexiValidator yFlexiValidator(trx, puPath, itin.calculationCurrency());

    if(yFlexiValidator.updDiscAmountBoundary())
      puPath.setSRFApplicable(true);
  }

  DiagManager diag666(trx, DiagnosticTypes::Diagnostic666);
  if (diag666.isActive())
  {
    Diag666Collector& dc = static_cast<Diag666Collector&>(diag666.collector());
    dc.printSolutionPatternInfo(trx, puPath, itin);
  }
}

MoneyAmount
calculateDiscount(const PricingTrx& trx,
                  const PricingOptions& pricingOptions,
                  const MoneyAmount referenceAmount,
                  const MoneyAmount fareAmount)
{
  double overallDiscount = SPANISH_DISCOUNT;

  if (!fallback::fixSpanishLargeFamilyForSRFE(&trx))
    overallDiscount += SLFUtil::getDiscountPercent(pricingOptions) / 100;

  return std::min(referenceAmount * overallDiscount, fareAmount * overallDiscount);
}

MoneyAmount
calculateDiscount(const PricingTrx& trx,
                  const PricingOptions& pricingOptions,
                  const PUPath& puPath,
                  const FareUsage& fu,
                  const CarrierCode& govCrx,
                  const CarrierCode& valCrx)
{
  const FareMarket& fm = *fu.paxTypeFare()->fareMarket();

  MoneyAmount srAmt = puPath.findSpanishResidentAmount(fm, govCrx, valCrx);
  MoneyAmount nucFareAmount = fu.paxTypeFare()->nucFareAmount();

  return calculateDiscount(trx, pricingOptions, srAmt, nucFareAmount);
}

MoneyAmount
recalculateDiscount(const PricingTrx& trx,
                    const PricingOptions& pricingOptions,
                    const YFlexiValidator& yFlexiValidator,
                    const FareUsage& fu,
                    const CarrierCode& valCrx)
{
  const FareMarket& fm = *fu.paxTypeFare()->fareMarket();

  MoneyAmount srAmt = yFlexiValidator.getAmount(fm, valCrx);
  MoneyAmount nucFareAmount = fu.paxTypeFare()->nucFareAmount();

  return calculateDiscount(trx, pricingOptions, srAmt, nucFareAmount);
}

void
applyDiscountUpperBound(const PricingTrx& trx,
                        const PricingOptions& pricingOptions,
                        FarePath& farePath,
                        const PUPath& puPath)
{
  MoneyAmount srMaxUpperBoundDiscAmt = 0.0;

  for(const CarrierCode& valCrx : puPath.getSpanishData()->puPathValCrxList)
  {
    MoneyAmount srUpperBoundDiscAmt = 0.0;

    for(const PricingUnit* pu : farePath.pricingUnit())
    {
      for(const FareUsage* fu : pu->fareUsage())
        srUpperBoundDiscAmt += calculateDiscount(trx, pricingOptions, puPath, *fu, ANY_CARRIER, valCrx);
    }
    srMaxUpperBoundDiscAmt = std::max(srMaxUpperBoundDiscAmt, srUpperBoundDiscAmt);
  }

  farePath.setSpanishResidentUpperBoundDiscAmt(srMaxUpperBoundDiscAmt);
  farePath.decreaseTotalNUCAmount(srMaxUpperBoundDiscAmt);
}

void
restoreFarePathForSpanishResident(FarePath& farePath)
{
  farePath.increaseTotalNUCAmount(farePath.getSpanishResidentUpperBoundDiscAmt());
}

void
clearSpanishResidentDiscountAmt(FarePath& fp)
{
  for (PricingUnit* pu : fp.pricingUnit())
    for (FareUsage* fu : pu->fareUsage())
      fu->setSpanishResidentDiscountAmt(0.0);
}

void
addFareBasisSuffix(PaxTypeFare& ptf, const PricingTrx& trx)
{
  if (fallback::srfeFareBasisSuffix(&trx))
    return;

  const StateCode& residency = trx.residencyState();

  const bool intraIsland = mapCityToState(ptf.origin()) == residency &&
                           mapCityToState(ptf.destination()) == residency;

  std::string suffix;
  if (residency == ST_RBP)
  {
    if (intraIsland)
    {
      if (!fallback::srfeFareBasisSuffixOldWay(&trx))
        suffix = "BI";
      else
        suffix = "IB";
    }
    else
    {
      if (!fallback::srfeFareBasisSuffixOldWay(&trx))
        suffix = "BP";
      else
        suffix = "RB";
    }
  }
  else if (residency == ST_RRC)
  {
    if (intraIsland)
    {
      if (!fallback::srfeFareBasisSuffixOldWay(&trx))
        suffix = "DC";
      else
        suffix = "IC";
    }
    else
      suffix = "RC";
  }
  else if (residency == ST_RRM)
    suffix = "RM";
  else if (residency == ST_RCE)
    suffix = "RE";

  ptf.setSpanishResidentFareBasisSuffix(std::move(suffix));
}

void
applySpanishResidentDiscount(PricingTrx& trx, FarePath& fp, const PUPath& puPath, bool isDomestic)
{
  MoneyAmount totalSRDiscountAmt = 0.0;

  const CarrierCode validatingCarrier = fp.getValidatingCarrier();

  const CurrencyCode fareCurrency =
      fp.pricingUnit().front()->fareUsage().front()->paxTypeFare()->currency();

  PUPath puP;
  YFlexiValidator yFlexiValidator(trx, puP, fareCurrency);

  for (PricingUnit* pu : fp.pricingUnit())
  {
    for (FareUsage* fu : pu->fareUsage())
    {
      if (fu->getSpanishResidentDiscountAmt() != 0.0)
        fu->setSpanishResidentDiscountAmt(0.0);

      if(fu->paxTypeFare()->fareMarket()->useDummyFare() &&
          fu->paxTypeFare()->totalFareAmount() == 0.0)
        continue;

      MoneyAmount srDiscountAmt;
      if (UNLIKELY(isDomestic))
        srDiscountAmt = recalculateDiscount(trx, *trx.getOptions(), yFlexiValidator,
                                            *fu, validatingCarrier);
      else
        srDiscountAmt = calculateDiscount(trx, *trx.getOptions(), puPath, *fu,
                                          fu->paxTypeFare()->fareMarket()->governingCarrier(),
                                          validatingCarrier);

      if (srDiscountAmt == 0.0)
      {
        clearSpanishResidentDiscountAmt(fp);
        return;
      }
      fu->setSpanishResidentDiscountAmt(srDiscountAmt);
      totalSRDiscountAmt += srDiscountAmt;

      addFareBasisSuffix(*fu->paxTypeFare(), trx);
    }
  }
  fp.decreaseTotalNUCAmount(totalSRDiscountAmt);
}

void
applyDomesticSpanishResidentDiscount(PricingTrx& trx, FarePath& fp)
{
  PUPath puP;
  applySpanishResidentDiscount(trx, fp, puP, true);
}

bool
isDiscountApplied(const FarePath& farePath)
{
  return farePath.getSpanishResidentUpperBoundDiscAmt() > EPSILON;
}

void
applySpanishLargeFamilyDiscount(const PricingTrx& trx, FarePath& farePath)
{
  if (SLFUtil::isSpanishFamilyDiscountApplicable(trx))
  {
    const MoneyAmount discountPercent = SLFUtil::getDiscountPercent(*trx.getOptions());

    MoneyAmount totalDiscountAmt = 0.0;

    for (PricingUnit* pu : farePath.pricingUnit())
    {
      for (FareUsage* fu : pu->fareUsage())
      {
        const PaxTypeFare& ptf = *fu->paxTypeFare();

        const MoneyAmount discountAmt = ptf.nucFareAmount() * discountPercent / 100.0;

        fu->setSpanishResidentDiscountAmt(discountAmt);
        totalDiscountAmt += discountAmt;
      }
    }
    farePath.decreaseTotalNUCAmount(totalDiscountAmt);
  }
}

/*
 * Check if at least one fare is Cat 25 Spanish resident fare and
 * does not combine with other Cat 25 non-Spanish resident fare
 */
bool
isSpanishResidentDiscountAppliesOld(const std::vector<PricingUnit*>& puCol)
{
  bool ret = false;
  FBRPaxTypeFareRuleData* fbrPaxTypeFare = nullptr;
  const FareByRuleItemInfo* fbrItemInfo = nullptr;

  for (PricingUnit* pricingUnit : puCol)
  {
    if (!pricingUnit)
      continue;
    for (FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      if (!fareUsage)
        continue;
      const PaxTypeFare* ptf = fareUsage->paxTypeFare();
      if (ptf && ptf->isFareByRule())
      {
        fbrPaxTypeFare = ptf->getFbrRuleData(RuleConst::FARE_BY_RULE);
        if (fbrPaxTypeFare)
        {
          fbrItemInfo = dynamic_cast<const FareByRuleItemInfo*>(fbrPaxTypeFare->ruleItemInfo());

          if (fbrItemInfo && (fbrItemInfo->fareInd() == 'C' || fbrItemInfo->fareInd() == 'G') &&
              fbrPaxTypeFare->isSpanishResidence() &&
              PaxTypeUtil::isSpanishPaxType(ptf->fcasPaxType()))
            ret = true;
          else
            return false;
        }
        else
          return false;
      }
    }
  }
  return ret;
}

} // SRFEUtil
} // tse
