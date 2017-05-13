// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include "DataModel/Common/TaxPointProperties.h"
#include "DomainDataObjects/Geo.h"
#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/Itin.h"
#include "Rules/TimeStopoverChecker.h"
#include "Rules/TurnaroundCalculator.h"
#include "Util/BranchPrediction.h"

namespace tax
{

namespace
{
  const type::Nation US("US");

  bool
  isEndOfSurfaceOutsideUS(const Geo& turnaround, const TaxPointProperties& tpp)
  {
    const Geo* prevNextPoint = turnaround.isArrival()
        ? turnaround.next()
        : turnaround.prev();

    return tpp.isSurface() &&
           (turnaround.getNation() != US) &&
           (prevNextPoint != nullptr) &&
           (prevNextPoint->getNation() == US);
  }

  bool
  isDomesticTurnaroundOnInternational(const GeoPath& geoPath, const Geo& turnaround)
  {
    return geoPath.isJourneyInternational() && turnaround.getNation() == geoPath.getOriginNation();
  }
} // namespace


TurnaroundCalculator::TurnaroundCalculator(const GeoPath& geoPath,
                                           const MileageService& mileageService,
                                           const TaxPointsProperties& taxPointsProperties,
                                           bool specUS_RTOJLogic,
                                           bool alternateTurnaroundDeterminationLogic)
  : _geoPath(geoPath),
    _mileageService(mileageService),
    _taxPointsProperties(taxPointsProperties),
    _specUS_RTOJLogic(specUS_RTOJLogic),
    _alternateTurnaroundDeterminationLogic(alternateTurnaroundDeterminationLogic)
{
}

const Geo*
TurnaroundCalculator::getTurnaroundPoint(const Itin& itin) const
{
  if (_mileageService.isRtw())
  {
    const std::vector<MileageService::GeoIdMile>& geoIdMiles = _mileageService.getMiles(
        _geoPath, itin.flightUsages(), 0, _geoPath.geos().size() - 1,
        type::Timestamp(itin.travelOriginDate(), type::Time(0, 0)));

    if (geoIdMiles.empty())
      return nullptr;

    return &_geoPath.geos()[geoIdMiles[0].first];
  }

  DomesticTimeStopoverChecker timeDomesticUS4hStopoverChecker(itin, 4 * 60, US);
  SpecificTimeStopoverChecker time24hStopoverChecker(24 * 60);
  Stopovers domesticUS4hStopovers(_geoPath.geos().size(), false);
  Stopovers international24hStopovers(_geoPath.geos().size(), false);

  for (const Geo& geo : _geoPath.geos())
  {
    const type::Index& current = geo.id();

    // Check stopovers
    if ((0 < current) && (current < _geoPath.geos().size() - 1))
    {
      const type::Index flightId = (current / 2);

      const FlightUsage* flightUsageBefore = nullptr;
      const FlightUsage* flightUsageAfter = nullptr;

      if ((current % 2) == 0) // departure
      {
        flightUsageBefore = &itin.flightUsages()[flightId - 1];
        flightUsageAfter = &itin.flightUsages()[flightId];
      }
      else // arrival
      {
        flightUsageBefore = &itin.flightUsages()[flightId];
        flightUsageAfter = &itin.flightUsages()[flightId + 1];
      }

      if (LIKELY(flightUsageBefore->forcedConnection() == type::ForcedConnection::Blank))
      {
        if (LIKELY(flightUsageBefore != nullptr && flightUsageAfter != nullptr))
        {
          if (LIKELY(flightUsageBefore->openSegmentIndicator() == type::OpenSegmentIndicator::Fixed &&
              flightUsageAfter->openSegmentIndicator() == type::OpenSegmentIndicator::Fixed))
          {
            domesticUS4hStopovers[current] =
                timeDomesticUS4hStopoverChecker.isStopover(*flightUsageBefore, *flightUsageAfter);
            international24hStopovers[current] =
                time24hStopoverChecker.isStopover(*flightUsageBefore, *flightUsageAfter);
          }
        }
      }
    }
  }

  const std::vector<MileageService::GeoIdMile>& geoIdMiles = _mileageService.getAllMilesFromStart(
      _geoPath, itin.flightUsages(), type::Timestamp(itin.travelOriginDate(), type::Time(0, 0)));

  const Geo* turnaround{nullptr};
  if (_alternateTurnaroundDeterminationLogic &&
      journeyHasConnectionAtFareBreakAndStopover(international24hStopovers, domesticUS4hStopovers))
  {
    // Find furthest stopover
    turnaround = findTurnaround(geoIdMiles,
                                [&](type::Index geoId,
                                    const TaxPointProperties& tpp)
                                {
                                  return international24hStopovers[geoId] ||
                                         domesticUS4hStopovers[geoId] ||
                                         tpp.isSurfaceStopover() ||
                                         tpp.isOpen;
                                });
  }
  else
  {
    // Find furthest stopover or fare break
    turnaround = findTurnaround(geoIdMiles,
                                [&](type::Index geoId,
                                    const TaxPointProperties& tpp)
                                {
                                  return international24hStopovers[geoId] ||
                                         domesticUS4hStopovers[geoId] ||
                                         tpp.isFareBreak ||
                                         tpp.isSurfaceStopover() ||
                                         tpp.isOpen;
                                });

  }

  return turnaround;
}

const Geo*
TurnaroundCalculator::findTurnaround(const GeoIdMiles& geoIdMiles,
                                     IsAllowedFunc isAllowed) const
{
  for (const MileageService::GeoIdMile& geoIdMile : geoIdMiles)
  {
    const TaxPointProperties& taxPointProperties = _taxPointsProperties[geoIdMile.first];

    if(isAllowed(geoIdMile.first, taxPointProperties))
    {
      const Geo& turnaroundPoint = _geoPath.geos()[geoIdMile.first];
      if (isDomesticTurnaroundOnInternational(_geoPath, turnaroundPoint) ||
          (_specUS_RTOJLogic && isEndOfSurfaceOutsideUS(turnaroundPoint, taxPointProperties)))
        continue;

      return &turnaroundPoint;
    }
  }

  return nullptr;
}

bool
TurnaroundCalculator::journeyHasConnectionAtFareBreakAndStopover(
    Stopovers international24hStopovers,
    Stopovers domesticUS4hStopovers) const
{
  bool hasConnectionAtFareBreak = false;
  bool hasStopover = false;
  // skip checking origin/destination
  for (type::Index geoId = 1 ; geoId < _geoPath.geos().size() - 1 ; ++geoId)
  {
    bool stopover = (international24hStopovers[geoId] ||
                     domesticUS4hStopovers[geoId] ||
                     _taxPointsProperties[geoId].isSurfaceStopover() ||
                     _taxPointsProperties[geoId].isOpen);

    if (stopover)
      hasStopover = true;
    else if (_taxPointsProperties[geoId].isFareBreak)
      hasConnectionAtFareBreak = true;
  }

  return hasConnectionAtFareBreak && hasStopover;
}

} // namespace tax
