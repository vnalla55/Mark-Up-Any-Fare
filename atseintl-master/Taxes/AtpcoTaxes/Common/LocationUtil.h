// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

#include "Common/LocZone.h"
#include "DataModel/Common/Types.h"
#include "DomainDataObjects/Geo.h"
#include "DomainDataObjects/FlightUsage.h"

#include <vector>

namespace tax
{

namespace LocationUtil
{
  bool isDomestic(const type::Nation& nation1, const type::Nation& nation2);
  bool isInternational(const type::Nation& nation1, const type::Nation& nation2);
  bool isBothRussia(const type::Nation& nation1, const type::Nation& nation2);

  bool doTaxLoc1AndLoc2NationMatch(LocZone loc1, LocZone loc2);

  template<class StopoverChecker>
  std::vector<bool> findStopovers(const std::vector<Geo>& geos,
                                  const std::vector<FlightUsage>& flightUsages,
                                  const StopoverChecker& stopoverChecker)
  {
    std::vector<bool> result(geos.size(), false);

    for (const Geo& geo : geos)
    {
      const type::Index& current = geo.id();

      // Check stopovers
      if ((0 < current) && (current < geos.size() - 1))
      {
        const FlightUsage* flightUsageBefore = nullptr;
        const FlightUsage* flightUsageAfter = nullptr;

        if (geo.isDeparture())
        {
          flightUsageBefore = geo.prev()->getFlightUsage(flightUsages);
          flightUsageAfter = geo.getFlightUsage(flightUsages);
        }
        else // arrival
        {
          flightUsageBefore = geo.getFlightUsage(flightUsages);
          flightUsageAfter = geo.next()->getFlightUsage(flightUsages);
        }

        if (LIKELY(flightUsageBefore->openSegmentIndicator() == type::OpenSegmentIndicator::Fixed &&
                   flightUsageAfter->openSegmentIndicator() == type::OpenSegmentIndicator::Fixed))
        {
          result[current] = stopoverChecker.isStopover(*flightUsageBefore, *flightUsageAfter);
        }
      }
    }

    return result;
  }
}

} // namespace tax
