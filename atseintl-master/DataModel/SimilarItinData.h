//-------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//-------------------------------------------------------------------
#pragma once

#include "Common/ClassOfService.h"
#include "Util/FlatMap.h"

#include <vector>

namespace tse
{
class FareMarket;
class Itin;
class TravelSeg;

struct FareMarketData
{
  void initialize(std::vector<TravelSeg*> segments)
  {
    travelSegments = std::move(segments);
    classOfService = std::vector<std::vector<ClassOfService>>(travelSegments.size());
  }

  std::vector<TravelSeg*> travelSegments;
  std::vector<std::vector<ClassOfService>> classOfService;
  FareMarket* fareMarket = nullptr;
};

struct SimilarItinData
{
  explicit SimilarItinData(Itin* it) : itin(it) {}
  Itin* itin;

  FareMarket* getSimilarFareMarket(const FareMarket* fm) const
  {
    const auto fmDataIt = fareMarketData.find(fm);
    return fmDataIt != fareMarketData.cend() ? fmDataIt->second.fareMarket : nullptr;
  }

  // A map from mott represents how mother's fare markets
  // should be translated to similar itins
  FlatMap<const FareMarket*, FareMarketData> fareMarketData;
};
}

