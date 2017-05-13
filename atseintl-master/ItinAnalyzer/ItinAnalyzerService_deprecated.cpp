///-------------------------------------------------------------------------------
// Deprecated methods - code extracted directly from ItinAnalyzerService.cpp.
//
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "Common/ClassOfService.h"
#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/PaxTypeUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TSELatencyData.h"
#include "DataModel/PricingTrx.h"
#include "Diagnostic/DCFactory.h"
#include "ItinAnalyzer/ItinAnalyzerService.h"

namespace tse
{
void
ItinAnalyzerService::deprecated_setInboundOutbound(const Itin& itin,
                                                   FareMarket& fareMarket,
                                                   DataHandle& dataHandle) const
{
  if (itin.segmentOrder(fareMarket.travelSeg().front()) ==
      1) // start from origin point, always outbound
  {
    fareMarket.direction() = FMDirection::OUTBOUND;
    return;
  }

  // Retransit logic -
  // If the fare market is transiting back to a country that has been
  // visited previously, this called "retransited" and the directionality
  // cannot be determined.  The fare market cannot include the first
  // or last travel segment of the itinerary
  bool hasRetransit = false;
  std::vector<TravelSeg*>::iterator tvlSegs = fareMarket.travelSeg().begin();
  std::vector<TravelSeg*>::iterator tvlSegsEnd = fareMarket.travelSeg().end();
  for (; tvlSegs != tvlSegsEnd; tvlSegs++)
  {
    if ((*tvlSegs)->retransited())
    {
      hasRetransit = true;
      break;
    }
  }

  if (itin.travelSeg().size() > 1)
  {
    if (itin.geoTravelType() == GeoTravelType::Domestic || // GeoTravelType::Domestic Itin
        itin.geoTravelType() == GeoTravelType::ForeignDomestic)
    {

      if (fareMarket.destination() ==
          itin.travelSeg().front()->origin()) // going back to itin origin
      {
        fareMarket.direction() = FMDirection::INBOUND;
        return;
      }
    }
    else // Intl Itin
    {
      if ((fareMarket.destination()->nation() ==
           itin.travelSeg().front()->origin()->nation()) || // going back to same nation
          (LocUtil::isDomesticUSCA(*fareMarket.destination()) &&
           LocUtil::isDomesticUSCA(*itin.travelSeg().front()->origin())) ||
          (LocUtil::isScandinavia(*fareMarket.destination()) &&
           LocUtil::isScandinavia(*itin.travelSeg().front()->origin())) ||
          (LocUtil::isNetherlandsAntilles(*fareMarket.destination()) &&
           LocUtil::isNetherlandsAntilles(*itin.travelSeg().front()->origin())) ||
          (LocUtil::isRussia(*fareMarket.destination()) &&
           LocUtil::isRussia(*itin.travelSeg().front()->origin())))
      {
        fareMarket.direction() = FMDirection::INBOUND;
        return;
      }
    }
  }

  fareMarket.direction() = hasRetransit ? FMDirection::UNKNOWN : FMDirection::OUTBOUND;
  if (fareMarket.direction() != FMDirection::UNKNOWN &&
      LocUtil::isSameCity(
          fareMarket.destination()->loc(), itin.travelSeg().front()->origin()->loc(), dataHandle))
  {
    fareMarket.direction() = FMDirection::UNKNOWN;
  }
}

} // tse
