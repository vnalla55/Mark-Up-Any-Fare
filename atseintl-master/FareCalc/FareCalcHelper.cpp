//----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------

#include "FareCalc/FareCalcHelper.h"

#include "Common/LocUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/FarePath.h"
#include "FareCalc/CalcTotals.h"
#include "FareCalc/FareCalcConsts.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/AdjustedSellingCalcData.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/FareRetailerRuleInfo.h"

namespace tse {


SpanishFamilyDiscountDesignator::SpanishFamilyDiscountDesignator(const SLFUtil::DiscountLevel discountLvl,
                                                                 const Loc& agentLocation,
                                                                 const std::vector<TravelSeg*>& travelSegs,
                                                                 const size_t maxFareBasisSize)
  : _discountLvl(discountLvl),
   _agentLocation(agentLocation),
   _travelSegs(travelSegs),
   _maxFareBasisSize(maxFareBasisSize)
{
}

void SpanishFamilyDiscountDesignator::operator()(std::string& fareBasis)
{
  if (_discountLvl == SLFUtil::DiscountLevel::NO_DISCOUNT)
    return;

  if (!LocUtil::isSpain(_agentLocation) || !LocUtil::isWholeTravelInSpain(_travelSegs))
    return;

  const char slash = '/';

  if (fareBasis.find(slash) == std::string::npos)
    fareBasis += slash;

  if (_discountLvl == SLFUtil::DiscountLevel::LEVEL_1)
    fareBasis += "F1";
  else if (_discountLvl == SLFUtil::DiscountLevel::LEVEL_2)
    fareBasis += "F2";

  if (fareBasis.size() > _maxFareBasisSize)
    fareBasis.erase(_maxFareBasisSize, fareBasis.size());
}

SpanishFamilyDiscountDesignator
spanishFamilyDiscountDesignatorBuilder(const PricingTrx& trx,
                                       const CalcTotals& calcTotals,
                                       const size_t maxFareBasisSize)
{
  const Itin* itin = calcTotals.farePath->itin() ?
      calcTotals.farePath->itin() : trx.itin().front();

  const std::vector<TravelSeg*>& travelSegs = itin->travelSeg();
  const SLFUtil::DiscountLevel discountLvl = trx.getOptions()->getSpanishLargeFamilyDiscountLevel();
  const Loc& agentLocation = *(trx.getRequest()->ticketingAgent()->agentLocation());

  return SpanishFamilyDiscountDesignator(discountLvl,
                                         agentLocation,
                                         travelSegs,
                                         maxFareBasisSize);
}

std::string
AdjustedSellingUtil::getADJSellingLevelMessage(PricingTrx& trx, CalcTotals& calcTotals)
{
  if (!calcTotals.farePath)
    return "";

  if (calcTotals.farePath->isAdjustedSellingFarePath() ||
      calcTotals.farePath->adjustedSellingFarePath())
    return trx.getOptions()->isMslRequest() ? "MSL" : "ASL";

  return "";
}

std::string
AdjustedSellingUtil::getADJSellingLevelOrgMessage(PricingTrx& trx, CalcTotals& calcTotals)
{
  if (!trx.getOptions() || !trx.getOptions()->isPDOForFRRule())
    return "";

  if (!calcTotals.adjustedCalcTotal  ||
      calcTotals.adjustedCalcTotal->adjustedSellingDiffInfo.empty())
    return "";

  std::ostringstream ss;
  auto it = calcTotals.adjustedCalcTotal->adjustedSellingDiffInfo.begin();
  ss << it->description << "  " << it->amount;

  for (++it; it != calcTotals.adjustedCalcTotal->adjustedSellingDiffInfo.end(); ++it)
    if (it->typeCode != "M")
      ss << "/" <<  it->amount << it->description;

  return ss.str();
}

std::string 
AdjustedSellingUtil::getFareRetailerCodeForNet(const FareUsage& fareUsage)
{
  const PaxTypeFare* paxTypeFare = fareUsage.paxTypeFare();

  if (paxTypeFare->hasCat35Filed())
  {
    const PaxTypeFareRuleData* ptfRuleData = paxTypeFare->paxTypeFareRuleData(NEGOTIATED_RULE);
    const NegPaxTypeFareRuleData* negRuleData =
        dynamic_cast<const NegPaxTypeFareRuleData*>(ptfRuleData);

    if (negRuleData && !negRuleData->sourcePseudoCity().empty() && !negRuleData->fareRetailerCode().empty())
      return negRuleData->fareRetailerCode();
  }

  return "";
}

std::string
AdjustedSellingUtil::getFareRetailerCodeForAdjusted(const FareUsage& fareUsage)
{
  const PaxTypeFare* paxTypeFare = fareUsage.paxTypeFare();

  if (paxTypeFare->getAdjustedSellingCalcData())
  {
    const AdjustedSellingCalcData* adjSellingCalcData  = paxTypeFare->getAdjustedSellingCalcData();
    if (adjSellingCalcData->getFareRetailerRuleInfo() &&
        !adjSellingCalcData->getSourcePcc().empty() &&
        !adjSellingCalcData->getFareRetailerRuleInfo()->fareRetailerCode().empty())
      return adjSellingCalcData->getFareRetailerRuleInfo()->fareRetailerCode();
  }

  return "";
}

void
AdjustedSellingUtil::getAllRetailerCodeFromFRR(CalcTotals& calcTotals,
                                               std::set<std::string>& retailerCodes)
{
  if (!calcTotals.farePath)
    return;

  for (const PricingUnit* pricingUnit : calcTotals.farePath->pricingUnit())
  {
    for (const FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      std::string retailerCode = getFareRetailerCodeForNet(*fareUsage);
      if (!retailerCode.empty())
        retailerCodes.insert(retailerCode);

      retailerCode = getFareRetailerCodeForAdjusted(*fareUsage);
      if (!retailerCode.empty())
        retailerCodes.insert(retailerCode);
    }
  }
}

std::string 
AdjustedSellingUtil::getRetailerCodeFromFRR(CalcTotals& calcTotals)
{
  if (!calcTotals.farePath)
    return "";

  std::set<std::string> retailerCodes;

  for (auto pricingUnit : calcTotals.farePath->pricingUnit())
  {
    for (auto fareUsage : pricingUnit->fareUsage())
    {
      std::string rc = getFareRetailerCodeForNet(*fareUsage);
      if (!rc.empty())
        retailerCodes.insert(rc);

      rc = getFareRetailerCodeForAdjusted(*fareUsage);
      if (!rc.empty())
        retailerCodes.insert(rc);

      if (retailerCodes.size() > 1)
        return " ";
    }
  }

  if (retailerCodes.size() == 1)
    return *retailerCodes.begin();

  return "";
}

}  // namespace tse
