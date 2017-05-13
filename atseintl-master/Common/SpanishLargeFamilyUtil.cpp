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

#include "Common/SpanishLargeFamilyUtil.h"

#include "Common/CurrencyRoundingUtil.h"
#include "Common/CurrencyUtil.h"
#include "Common/LocUtil.h"
#include "Common/PaxTypeUtil.h"
#include "Common/SpanishResidentFaresEnhancementUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/Fare.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeBucket.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/FareInfo.h"
#include "Fares/FareByRuleController.h"

namespace tse
{

namespace SLFUtil
{

namespace
{

MoneyAmount
getFareAfterDiscountPercent(const Percent discountPercent,
                            const MoneyAmount fareAmount)
{
  const MoneyAmount discount = fareAmount * discountPercent / 100.0;
  MoneyAmount newAmount = fareAmount - discount;
  // For the unlikely case that the percent discount is >= 100
  if (newAmount < 0.0)
    newAmount = 0.0;
  return newAmount;
}

bool
isSpanishFamilyDiscountAlreadyApplied(const PaxTypeFare* ptf)
{
  if (!ptf || !ptf->isFareByRule())
    return false;

  const FBRPaxTypeFareRuleData* fbrPaxTypeFare = ptf->getFbrRuleData(RuleConst::FARE_BY_RULE);
  if (fbrPaxTypeFare)
  {
    const FareByRuleItemInfo* fbrItemInfo =
        dynamic_cast<const FareByRuleItemInfo*>(fbrPaxTypeFare->ruleItemInfo());

    if (fbrItemInfo && (fbrItemInfo->fareInd() == FareByRuleController::CALCULATED ||
                        fbrItemInfo->fareInd() ==
                            FareByRuleController::SUBTRACT_SPECIFIED_FROM_BASE_CALC_PERCENTAGE) &&
        fbrPaxTypeFare->isSpanishResidence() && PaxTypeUtil::isSpanishPaxType(ptf->fcasPaxType()))
      return true;
  }

  return false;
}

void
applySpanishFamilyDisountToFaresForIS(PricingTrx& trx,
                                      PaxTypeFare* ptf,
                                      double percentDiscount)
{
  if (percentDiscount <= 0.0)
    return;

  if (!ptf || ptf->fareAmount() <= 0)
    return;

  CurrencyRoundingUtil curRoundingUtil;

  FareInfo* fareInfoClone = ptf->fare()->fareInfo()->clone(trx.dataHandle());
  fareInfoClone->fareAmount() = getFareAfterDiscountPercent(percentDiscount,
                                                            fareInfoClone->fareAmount());
  curRoundingUtil.round(fareInfoClone->fareAmount(), ptf->currency(), trx);

  fareInfoClone->fareAmount() = getFareAfterDiscountPercent(percentDiscount,
                                                            ptf->fare()->nucFareAmount());
  CurrencyUtil::truncateNUCAmount(ptf->fare()->nucFareAmount());
  ptf->fare()->setFareInfo(fareInfoClone);

}

} // empty namespace

void
applySpanishFamilyDisountToFares(PricingTrx& trx,
                                 FareMarket& fm,
                                 MoneyAmount percentDiscount)
{
  if (percentDiscount <= 0.0)
    return;

  CurrencyRoundingUtil curRoundingUtil;

  for (auto* ptf : fm.allPaxTypeFare())
  {
    if (ptf && !isSpanishFamilyDiscountAlreadyApplied(ptf) && ptf->fareAmount() > 0)
    {
      FareInfo* fareInfoClone = ptf->fare()->fareInfo()->clone(trx.dataHandle());
      MoneyAmount fareAmount = ptf->isRoundTrip() ? ptf->originalFareAmount() : ptf->fareAmount();
      MoneyAmount nucFareAmount =
          ptf->isRoundTrip() ? ptf->nucOriginalFareAmount() : ptf->nucFareAmount();
      fareAmount = getFareAfterDiscountPercent(percentDiscount, fareAmount);
      curRoundingUtil.round(fareAmount, ptf->currency(), trx);
      fareAmount = getFareAfterDiscountPercent(percentDiscount, nucFareAmount);
      CurrencyUtil::truncateNUCAmount(nucFareAmount);
      fareInfoClone->originalFareAmount() = fareAmount;
      ptf->fare()->nucOriginalFareAmount() = nucFareAmount;
      if (ptf->isRoundTrip())
      {
        fareInfoClone->fareAmount() = fareAmount / 2.0;
        ptf->fare()->nucFareAmount() = nucFareAmount / 2.0;
        CurrencyUtil::truncateNUCAmount(ptf->fare()->nucFareAmount());
      }
      else
      {
        fareInfoClone->fareAmount() = fareAmount;
        ptf->fare()->nucFareAmount() = nucFareAmount;
      }

      ptf->fare()->setFareInfo(fareInfoClone);
    }
  }
}

void
checkSpanishDiscountForIS(PricingTrx& trx,
                          FareMarket& fareMarket)
{
  const ShoppingTrx* st = dynamic_cast<ShoppingTrx*>(&trx);

  if (LIKELY(!st->isSpanishDiscountFM(&fareMarket)))
    return;

  const Percent percentDiscount = getDiscountPercent(*trx.getOptions());

  std::vector<PaxTypeFare*> tempPtfVec;
  tempPtfVec.reserve(std::accumulate(fareMarket.paxTypeCortege().cbegin(),
                                     fareMarket.paxTypeCortege().cend(),
                                     static_cast<size_t>(0),
                                     [](const size_t sum, const auto& paxTypeBucket)
                                     {
                                       return sum + paxTypeBucket.paxTypeFare().size();
                                     }));
  size_t tempPtfVecRefIndex = 0;

  for (auto& paxTypeBucket : fareMarket.paxTypeCortege())
  {
    for (auto* origPTFare : paxTypeBucket.paxTypeFare())
    {
      const Fare* oldFare = origPTFare->fare();
      Fare* newFare = oldFare->clone(trx.dataHandle(), false);
      PaxTypeFare* newPTFare = origPTFare->clone(trx.dataHandle(), true, nullptr, newFare);

      applySpanishFamilyDisountToFaresForIS(trx, newPTFare, percentDiscount);
      newPTFare->setSpanishDiscountEnabled();
      tempPtfVec.push_back(newPTFare);
    }

    paxTypeBucket.paxTypeFare().insert(
          paxTypeBucket.paxTypeFare().end(), tempPtfVec.begin() + tempPtfVecRefIndex, tempPtfVec.end());
    tempPtfVecRefIndex = tempPtfVec.size();
  }

  fareMarket.allPaxTypeFare().insert(
        fareMarket.allPaxTypeFare().end(), tempPtfVec.begin(), tempPtfVec.end());
}

Percent
getDiscountPercent(const PricingOptions& options)
{
  switch (options.getSpanishLargeFamilyDiscountLevel())
  {
  case DiscountLevel::LEVEL_1:
    return DISCOUNT_LEVEL_1;
  case DiscountLevel::LEVEL_2:
    return DISCOUNT_LEVEL_2;
  default:
    return 0.0;
  }
}

/*
 * Check if at least one fare is Cat 25 Spanish resident fare
 */
bool
isSpanishResidentAndLargeFamilyCombinedDiscountApplies(const std::vector<PricingUnit*>& puCol)
{
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
        if (!fbrPaxTypeFare)
          continue;

        fbrItemInfo = dynamic_cast<const FareByRuleItemInfo*>(fbrPaxTypeFare->ruleItemInfo());

        if (fbrItemInfo && (fbrItemInfo->fareInd() == 'C' || fbrItemInfo->fareInd() == 'G') &&
            fbrPaxTypeFare->isSpanishResidence() &&
            PaxTypeUtil::isSpanishPaxType(ptf->fcasPaxType()))
          return true;
      }
    }
  }
  return false;
}

std::string
getIndicator(const PricingTrx& trx,
             const Itin& itin,
             const FarePath& farePath)
{
  const DiscountLevel spanishDiscountLevel =
      trx.getOptions()->getSpanishLargeFamilyDiscountLevel();

  if (farePath.pricingUnit().size() && trx.getRequest() &&
      trx.getRequest()->ticketingAgent() &&
      LocUtil::isSpain(*(trx.getRequest()->ticketingAgent()->agentLocation())) &&
      LocUtil::isWholeTravelInSpain(itin.travelSeg()))
  {
    if (spanishDiscountLevel != DiscountLevel::NO_DISCOUNT)
    {
      if (SLFUtil::isSpanishResidentAndLargeFamilyCombinedDiscountApplies(
              farePath.pricingUnit()) || SRFEUtil::isDiscountApplied(farePath))
        return "B"; // Combined disc
      else
        return "A"; // Spanish Large Family disc only
    }
    else if (SRFEUtil::isSpanishResidentDiscountAppliesOld(farePath.pricingUnit()) ||
             SRFEUtil::isDiscountApplied(farePath))
    {
      return "C"; // Islander disc only
    }
  }
  return std::string();
}

DiscountLevel
getDiscountLevelFromInt(const int16_t value)
{
  switch (value)
  {
  case 1:
    return DiscountLevel::LEVEL_1;
  case 2:
    return DiscountLevel::LEVEL_2;
  default:
    return DiscountLevel::NO_DISCOUNT;
  }
}

bool
isSpanishFamilyDiscountApplicable(const PricingTrx& trx)
{
  return (trx.getOptions()->getSpanishLargeFamilyDiscountLevel() == DiscountLevel::LEVEL_1 &&
          trx.getOptions()->getSpanishLargeFamilyDiscountLevel() == DiscountLevel::LEVEL_2 &&
          LocUtil::isSpain(*(trx.getRequest()->ticketingAgent()->agentLocation())) &&
          LocUtil::isWholeTravelInSpain(trx.travelSeg()));
}

} // namespace SLFUtil

} // namespace tse
