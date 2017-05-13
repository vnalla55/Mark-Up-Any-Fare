// ----------------------------------------------------------------
//
//   Copyright Sabre 2015
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#pragma once

#include "Common/YQYR/NGSYQYRCalculator.h"
#include "Common/YQYR/YQYRUtils.h"
#include "DataModel/ShoppingTrx.h"

namespace tse
{
namespace YQYR
{

template <class IteratorType>
inline const Loc*
NGSYQYRCalculator::determineFurthestPoint(IteratorType begin,
                                          IteratorType end,
                                          const AirSeg* journeyOrigin,
                                          const Itin* itin,
                                          const bool isOutbound) const
{
  static const TimeAndUnit EMPTY_TIME;

  const AirSeg* furthestPoint = 0;
  const TravelSeg* nextSegment = 0;

  IteratorType segIt(begin);
  while (furthestPoint == 0 && segIt != end)
  {
    nextSegment = *segIt;
    furthestPoint = (*segIt)->toAirSeg();

    ++segIt;
  }

  if (!furthestPoint)
    return 0;

  uint32_t highestMiles =
      YQYRUtils::journeyMileage(journeyOrigin->origin(),
                                isOutbound ? furthestPoint->destination() : furthestPoint->origin(),
                                itin->travelSeg(),
                                itin->travelDate(),
                                _trx);

  for (; segIt != end; nextSegment = *segIt++)
  {
    const AirSeg* airSeg((*segIt)->toAirSeg());
    if (!airSeg)
      continue;

    if (airSeg->isStopOver(nextSegment, itin->geoTravelType(), EMPTY_TIME))
    {
      const uint32_t mileage =
          YQYRUtils::journeyMileage(journeyOrigin->origin(),
                                    isOutbound ? airSeg->destination() : airSeg->origin(),
                                    itin->travelSeg(),
                                    itin->travelDate(),
                                    _trx);

      if (mileage > highestMiles)
      {
        highestMiles = mileage;
        furthestPoint = airSeg;
      }
    }
  }

  if (furthestPoint)
    return isOutbound ? furthestPoint->destination() : furthestPoint->origin();

  return 0;
}

}
}
