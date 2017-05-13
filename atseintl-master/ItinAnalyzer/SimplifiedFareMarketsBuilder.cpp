//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------

#include "ItinAnalyzer/SimplifiedFareMarketsBuilder.h"

#include "Common/TsePrimitiveTypes.h"
#include "Common/TravelSegUtil.h"
#include "Common/ShoppingUtil.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/Itin.h"
#include "DataModel/TravelSeg.h"

namespace tse {

using FareMarketParams = SimplifiedFareMarketsBuilder::FareMarketParams;

SimplifiedFareMarketsBuilder::SimplifiedFareMarketsBuilder(PricingTrx& trx, Itin& itin)
: _trx(trx), _itin(itin)
{
}

std::vector<FareMarketParams>
SimplifiedFareMarketsBuilder::build(std::vector<SegmentAttributes>& segmentAttributes)
{
  std::vector<FareMarketParams> fareMarkets;

  segmentAttributes.reserve(_itin.travelSeg().size());

  std::transform(_itin.travelSeg().begin(), _itin.travelSeg().end(),
                 std::back_inserter(segmentAttributes),
                 [&](TravelSeg* seg) { return SegmentAttributes({seg, 0}); });

  std::vector<bool> isStopover = TravelSegUtil::calculateStopOvers(_itin.travelSeg(),
                                                                   _itin.geoTravelType());

  int i = 0;
  for (auto beginIt = segmentAttributes.begin(); beginIt != segmentAttributes.end(); ++beginIt, ++i)
  {
    if (beginIt->tvlSeg->isAir())
    {
      fareMarkets.push_back({beginIt, beginIt + 1});
    }

    int j = i;
    for (auto endIt = beginIt + 1; endIt != segmentAttributes.end(); ++endIt, ++j)
    {
      if (isStopover[j])
        break;
      fareMarkets.push_back({beginIt, endIt + 1});
    }
  }

  return fareMarkets;
}

bool
SimplifiedFareMarketsBuilder::isComplexItin(const Itin& itin)
{
  int32_t segCounter = 0, currentLegID = 0;
  for (const TravelSeg* seg : itin.travelSeg())
  {
    if (seg->legId() == currentLegID)
      ++segCounter;
    else
    {
      segCounter = 1;
      currentLegID = seg->legId();
    }

    if (segCounter > 2)
      return true;
  }

  return false;
}

} //tse
