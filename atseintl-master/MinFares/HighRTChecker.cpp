//----------------------------------------------------------------------------
//
//  File:           HighRTChecker.cpp
//  Created:        2/28/2011
//  Authors:
//
//  Description:    A utility class to assist in checking exemption and calculating
//  HRT plus up for different trip type (OJ,RT, etc).
//
//
//  Updates:
//
//  Copyright Sabre 2011
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "MinFares/HighRTChecker.h"

#include "Common/CurrencyUtil.h"
#include "Common/Logger.h"
#include "DataModel/FareUsage.h"
#include "DataModel/MinFarePlusUp.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/CarrierPreference.h"
#include "MinFares/MinFareLogic.h"
#include "Rules/RuleConst.h"

namespace tse
{
namespace HighRTChecker
{
static Logger
logger("atseintl.MinFares.HighRTChecker");

bool
qualifyCat10Subcat101Byte13(const PricingUnit& pu)
{
  std::vector<FareUsage*>::const_iterator fuIt = pu.fareUsage().begin();
  std::vector<FareUsage*>::const_iterator fuEnd = pu.fareUsage().end();
  for (; fuIt != fuEnd; ++fuIt)
  {
    const FareUsage& fu = **fuIt;
    if (!fu.highRT())
      return false;

    if (fu.paxTypeFare()->isNegotiated())
    {
      if ('Y' != fu.paxTypeFare()->fareMarket()->governingCarrierPref()->applyHigherRTOJ())
        return false;
    }
  }
  return true;
}

bool
qualifyCat10Subcat102Byte13(const PricingUnit& pu)
{
  std::vector<FareUsage*>::const_iterator fuIt = pu.fareUsage().begin();
  std::vector<FareUsage*>::const_iterator fuEnd = pu.fareUsage().end();
  for (; fuIt != fuEnd; ++fuIt)
  {
    const FareUsage& fu = **fuIt;
    if (LIKELY(!fu.highRT()))
      return false;

    if (fu.paxTypeFare()->isNegotiated())
    {
      if ('Y' != fu.paxTypeFare()->fareMarket()->governingCarrierPref()->applyHigherRT())
        return false;
    }
  }
  return true;
}

bool
isSameCabin(const PricingUnit& pu)
{
  std::vector<FareUsage*>::const_iterator fuIt = pu.fareUsage().begin();
  std::vector<FareUsage*>::const_iterator fuEnd = pu.fareUsage().end();
  CabinType cabinType = (*fuIt++)->paxTypeFare()->cabin();
  for (; fuIt != fuEnd; ++fuIt)
  {
    CabinType nextCabinType = (*fuIt)->paxTypeFare()->cabin();
    if (cabinType == nextCabinType)
      continue;
    if ((cabinType.isFirstClass() && nextCabinType.isPremiumFirstClass()) ||
        (cabinType.isPremiumFirstClass() && nextCabinType.isFirstClass()) ||
        (cabinType.isBusinessClass() && nextCabinType.isPremiumBusinessClass()) ||
        (cabinType.isPremiumBusinessClass() && nextCabinType.isBusinessClass()) ||
        (cabinType.isEconomyClass() && nextCabinType.isPremiumEconomyClass()) ||
        (cabinType.isPremiumEconomyClass() && nextCabinType.isEconomyClass()))
      continue;
    return false;
  }
  return true;
}

const FareUsage*
getHighestRoundTripFare(const PricingUnit& pu, MoneyAmount& RT_FareAmt)
{
  std::vector<FareUsage*>::const_iterator fuIter = pu.fareUsage().begin();
  std::vector<FareUsage*>::const_iterator fuEnd = pu.fareUsage().end();
  const FareUsage* highestRtFu = *fuIter++;
  for (; fuIter != fuEnd; ++fuIter)
  {
    if ((*fuIter)->paxTypeFare()->nucFareAmount() > highestRtFu->paxTypeFare()->nucFareAmount())
      highestRtFu = *fuIter;
  }
  RT_FareAmt = highestRtFu->paxTypeFare()->nucFareAmount() * 2;
  return highestRtFu;
}

const FareUsage*
getHighestRoundTripNetFare(const PricingUnit& pu, MoneyAmount& RT_NetFareAmt)
{
  std::vector<FareUsage*>::const_iterator fuIter = pu.fareUsage().begin();
  std::vector<FareUsage*>::const_iterator fuEnd = pu.fareUsage().end();
  const FareUsage* highestRtFu = *fuIter;
  const PaxTypeFare* ptFare = (*fuIter++)->paxTypeFare();
  const NegPaxTypeFareRuleData* highestNetFare = ptFare->getNegRuleData();
  for (; fuIter != fuEnd; ++fuIter)
  {
    FareUsage& fu = (**fuIter);
    ptFare = fu.paxTypeFare();
    const NegPaxTypeFareRuleData* netFare = ptFare->getNegRuleData();
    if (netFare->nucNetAmount() > highestNetFare->nucNetAmount())
    {
      highestRtFu = *fuIter;
      highestNetFare = netFare;
    }
  }
  RT_NetFareAmt = highestNetFare->nucNetAmount() * 2;
  return highestRtFu;
}

bool
calculateFareAmt(const PricingUnit& pu, MoneyAmount& fareAmt)
{
  const PaxTypeFare* ptFare;
  std::vector<FareUsage*>::const_iterator fuIter = pu.fareUsage().begin();
  std::vector<FareUsage*>::const_iterator fuIterEnd = pu.fareUsage().end();
  for (; fuIter != fuIterEnd; ++fuIter)
  {
    const FareUsage& fu = (**fuIter);
    ptFare = fu.paxTypeFare();

    if (!ptFare)
    {
      LOG4CXX_ERROR(logger, "Highest RT : Internal Error: PaxTypeFare is NULL");
      return false;
    }

    fareAmt +=
        ptFare->nucFareAmount() + fu.minFarePlusUp().getSum(HIP) + ptFare->mileageSurchargeAmt();
  }
  return true;
}

bool
calculateNetFareAmt(const PricingUnit& pu, MoneyAmount& netFareAmt)
{
  for (const auto* fareUsage : pu.fareUsage())
  {
    const PaxTypeFare* ptFare = fareUsage->paxTypeFare();
    if (!ptFare->isNegotiated())
      return false;
    const NegPaxTypeFareRuleData* netFare = ptFare->getNegRuleData();
    if (!netFare)
      return false;

    MoneyAmount mileageSurchargeAmt =
        ptFare->mileageSurchargePctg() > 0
            ? (netFare->nucNetAmount() * ptFare->mileageSurchargePctg()) /
                  RuleConst::HUNDRED_PERCENTS
            : 0.0;
    CurrencyUtil::truncateNUCAmount(mileageSurchargeAmt);
    netFareAmt +=
        netFare->nucNetAmount() + fareUsage->minFarePlusUp().getSum(HIP) + mileageSurchargeAmt;
  }
  return true;
}

void
getHrtPlusUp(const MoneyAmount& HRT_FareAmt,
             const MoneyAmount& RT_FareAmt,
             const FareUsage* highestRtFu,
             MinFarePlusUpItem& hrtPlusUp)
{
  hrtPlusUp.baseAmount = HRT_FareAmt;
  hrtPlusUp.plusUpAmount = RT_FareAmt - HRT_FareAmt;

  if (highestRtFu->isOutbound())
  {
    hrtPlusUp.boardPoint = highestRtFu->paxTypeFare()->fareMarket()->boardMultiCity();
    hrtPlusUp.offPoint = highestRtFu->paxTypeFare()->fareMarket()->offMultiCity();
  }
  else
  {
    hrtPlusUp.boardPoint = highestRtFu->paxTypeFare()->fareMarket()->offMultiCity();
    hrtPlusUp.offPoint = highestRtFu->paxTypeFare()->fareMarket()->boardMultiCity();
  }
}
}
}
