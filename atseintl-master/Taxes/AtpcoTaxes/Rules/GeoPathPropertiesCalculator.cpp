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

#include "Common/LocationUtil.h"
#include "DataModel/Common/GeoPathProperties.h"
#include "DomainDataObjects/Request.h"
#include "Rules/GeoPathPropertiesCalculator.h"
#include "Rules/TimeStopoverChecker.h"
#include "ServiceInterfaces/MileageService.h"
#include "Util/BranchPrediction.h"

namespace tax
{

GeoPathPropertiesCalculator::GeoPathPropertiesCalculator(const Request& request,
                                                         const MileageService& mileageService)
  : _request(request),
    _mileageService(mileageService)
{
}

void
GeoPathPropertiesCalculator::calculate(const Itin& itin, GeoPathProperties& geoPathProperties) const
{
  type::Nation nationUS("US");
  std::unique_ptr<TaxPointsProperties> temporaryProperties(new TaxPointsProperties);
  TaxPointsProperties& taxPointsProperties = *temporaryProperties;
  const GeoPath& geoPath = _request.geoPaths()[itin.geoPathRefId()];
  taxPointsProperties.resize(geoPath.geos().size());
  CompactOptional<type::Index> fpathGeoIndex = itin.farePathGeoPathMappingRefId();
  assert (fpathGeoIndex.has_value());
  assert (fpathGeoIndex.value() < _request.geoPathMappings().size());

  for (const Mapping& mapping : _request.geoPathMappings()[fpathGeoIndex.value()].mappings())
  {
    if (LIKELY(mapping.maps().size() > 0))
    {
      taxPointsProperties[mapping.maps().front().index()].isFareBreak = true;
      taxPointsProperties[mapping.maps().back().index()].isFareBreak = true;
    }
  }

  SpecificTimeStopoverChecker timeStopoverChecker(24 * 60);
  SpecificTimeStopoverChecker timeUSStopoverChecker(12 * 60);
  SpecificTimeStopoverChecker timeUS4hrStopoverChecker(4 * 60);

  const type::Index lastGeoId = geoPath.geos().back().id();
  for (const Geo& geo : geoPath.geos())
  {
    const type::Index current = geo.id();

    // Reset all
    taxPointsProperties[current].isFirst = false;
    taxPointsProperties[current].isLast = false;
    taxPointsProperties[current].setIsSurface(false);
    taxPointsProperties[current].setIsCASurfaceException(false);
    taxPointsProperties[current].isTimeStopover = boost::none;
    taxPointsProperties[current].isUSTimeStopover = boost::none;
    taxPointsProperties[current].isUSLimitStopover = boost::none;

    if (0 == current || current == lastGeoId)
      continue;

    // Set surface
    const type::Index toCheck = ((current % 2) == 0) ? (current - 1) : (current + 1);
    taxPointsProperties[current].setIsSurface(
        geo.loc().cityCode() != geoPath.geos()[toCheck].loc().cityCode());

    // Set stopovers
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
          taxPointsProperties[current].isTimeStopover =
              timeStopoverChecker.isStopover(*flightUsageBefore, *flightUsageAfter);
          taxPointsProperties[current].isUSTimeStopover =
              timeUSStopoverChecker.isStopover(*flightUsageBefore, *flightUsageAfter);

          if (geo.getNation() == nationUS &&
              geo.prev() && geo.prev()->getNation() == nationUS &&
              geo.next() && geo.next()->getNation() == nationUS) // 4 hour stop
          {
            taxPointsProperties[current].isUSLimitStopover =
                timeUS4hrStopoverChecker.isStopover(*flightUsageBefore, *flightUsageAfter);
          }
          else // 12 hour stop
          {
            taxPointsProperties[current].isUSLimitStopover =
                timeUSStopoverChecker.isStopover(*flightUsageBefore, *flightUsageAfter);
          }
        }
        else
        {
          taxPointsProperties[current].isOpen = true;
        }
      }
    }
    else if (flightUsageBefore->forcedConnection() == type::ForcedConnection::Stopover)
    {
      taxPointsProperties[current].isTimeStopover = true;
      taxPointsProperties[current].isUSTimeStopover = true;
      taxPointsProperties[current].isUSLimitStopover = true;
    }
    else if (flightUsageBefore->forcedConnection() == type::ForcedConnection::Connection)
    {
      taxPointsProperties[current].isTimeStopover = false;
      taxPointsProperties[current].isUSTimeStopover = false;
      taxPointsProperties[current].isUSLimitStopover = false;
    }
  }

  if (LIKELY(geoPath.geos().size() != 0))
  {
    taxPointsProperties.front().isFirst = true;
    taxPointsProperties.back().isLast = true;
  }

  geoPathProperties.taxPointsProperties.reset(temporaryProperties.release());
  geoPathProperties.roundTripOrOpenJaw =
      (geoPath.isJourneyDomestic()) ? (geoPath.getOriginCity() == geoPath.getDestinationCity())
                                    : (LocationUtil::isDomestic(geoPath.getOriginNation(), geoPath.getDestinationNation()));
}

} // namespace tax
