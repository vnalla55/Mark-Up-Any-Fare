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
#pragma once

#include "Common/SpanishLargeFamilyEnum.h"

#include <set>
#include <string>
#include <vector>

namespace tse
{
class CalcTotals;
class FareUsage;
class Loc;
class PricingTrx;
class TravelSeg;

class SpanishFamilyDiscountDesignator
{
  const SLFUtil::DiscountLevel _discountLvl;
  const Loc& _agentLocation;
  const std::vector<TravelSeg*>& _travelSegs;
  const size_t _maxFareBasisSize;

public:
  SpanishFamilyDiscountDesignator(const SLFUtil::DiscountLevel discountLvl,
                                  const Loc& agentLocation,
                                  const std::vector<TravelSeg*>& travelSegs,
                                  const size_t maxFareBasisSize);

  void operator()(std::string& fareBasis);
};

SpanishFamilyDiscountDesignator
spanishFamilyDiscountDesignatorBuilder(const PricingTrx& trx,
                                       const CalcTotals& calcTotals,
                                       const size_t maxFareBasisSize);

class AdjustedSellingUtil
{
public:
  static std::string getADJSellingLevelMessage(PricingTrx& trx, CalcTotals& calcTotals);
  static std::string getADJSellingLevelOrgMessage(PricingTrx& trx, CalcTotals& calcTotals);

  static std::string getFareRetailerCodeForNet(const FareUsage& fareUsage);
  static std::string getFareRetailerCodeForAdjusted(const FareUsage& fareUsage);
  static std::string getRetailerCodeFromFRR(CalcTotals& calcTotals);
  static void getAllRetailerCodeFromFRR(CalcTotals& calcTotals,
                                        std::set<std::string>& retailerCodes);

};

} // namespace tse
