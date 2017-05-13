// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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
#include "DataModel/Common/Types.h"
#include "DomainDataObjects/GeoPath.h"
#include "ServiceInterfaces/MileageGetter.h"
#include "ServiceInterfaces/MileageService.h"

#include <algorithm>

namespace tax
{

MileageService::MileageService(void) {}

MileageService::~MileageService(void) {}

const std::vector<MileageService::GeoIdMile>&
MileageService::getMiles(const GeoPath& geoPath,
                         const std::vector<tax::FlightUsage>& flightUsages,
                         const type::Index first,
                         const type::Index last,
                         const type::Timestamp& travelDate) const
{
  GeoIdMileMap::key_type mapKey(geoPath.id(), first, last);

  GeoIdMileMap::const_iterator it = _geoIdMileMap.find(mapKey);
  if (it == _geoIdMileMap.end())
  {
    return _geoIdMileMap.insert({mapKey,
      calculateMilesInGeoPath(geoPath, flightUsages, first, last, travelDate, true)}).first->second;
  }

  return it->second;
}

type::Miles
MileageService::getDistance(const GeoPath& geoPath,
                            const std::vector<tax::FlightUsage>& flightUsages,
                            const type::Index first,
                            const type::Index last,
                            const type::Timestamp& travelDate) const
{
  DistanceMap::key_type mapKey(geoPath.id(), first, last);
  DistanceMap::const_iterator it = _distanceMap.find(mapKey);
  if (it == _distanceMap.end())
  {
    const MileageGetter& mileage = getMileageGetter(geoPath, flightUsages, travelDate);
    return _distanceMap.insert({mapKey,
      mileage.getSingleDistance(std::min(first, last), std::max(first, last))}).first->second;
  }

  return it->second;
}

const std::vector<MileageService::GeoIdGlobDir>&
MileageService::getGlobDir(const GeoPath& geoPath,
                           const std::vector<tax::FlightUsage>& flightUsages,
                           const type::Index first,
                           const type::Index last,
                           const type::Timestamp& travelDate) const
{
  GeoIdGlobDirMap::key_type mapKey(geoPath.id(), first, last);

  GeoIdGlobDirMap::const_iterator it = _geoIdGlobDirMap.find(mapKey);
  if (it == _geoIdGlobDirMap.end())
  {
    return _geoIdGlobDirMap.insert({mapKey,
      calculateGlobalDirectionInGeoPath(geoPath, flightUsages, first, last, travelDate)}).first->second;
  }

  return it->second;
}

const std::vector<MileageService::GeoIdMile>&
MileageService::getAllMilesFromStart(const GeoPath& geoPath,
                                     const std::vector<tax::FlightUsage>& flightUsages,
                                     const type::Timestamp& travelDate) const
{
  assert(!geoPath.geos().empty());
  return getMiles(geoPath, flightUsages, 0, geoPath.geos().size() - 1, travelDate);
}

MileageService::GeoIdMileMap::mapped_type
MileageService::calculateMilesInGeoPath(const GeoPath& geoPath,
                                        const std::vector<tax::FlightUsage>& flightUsages,
                                        const type::Index first,
                                        const type::Index last,
                                        const type::Timestamp& travelDate) const
{
  return calculateMilesInGeoPath(geoPath, flightUsages, first, last, travelDate, true);
}

MileageService::GeoIdMileMap::mapped_type
MileageService::calculateMilesInGeoPath(const GeoPath& geoPath,
                                        const std::vector<tax::FlightUsage>& flightUsages,
                                        const type::Index first,
                                        const type::Index last,
                                        const type::Timestamp& travelDate,
                                        bool sort /* = true */) const
{
  const MileageGetter& mileage = getMileageGetter(geoPath, flightUsages, travelDate);

  GeoIdMileMap::mapped_type geoIdMileVector;
  for (type::Index i = std::min(first, last); i <= std::max(first, last); ++i)
  {
    geoIdMileVector.push_back(GeoIdMile(i, mileage.getSingleDistance(first, i)));
  }


  if ( !sort )
    return geoIdMileVector;

  std::sort(geoIdMileVector.begin(), geoIdMileVector.end(),
            [](const GeoIdMile& lhs, const GeoIdMile& rhs)
            {
              return lhs.second > rhs.second;
            });

  return geoIdMileVector;
}

MileageService::GeoIdGlobDirMap::mapped_type
MileageService::calculateGlobalDirectionInGeoPath(
    const GeoPath& geoPath,
    const std::vector<tax::FlightUsage>& flightUsages,
    const type::Index first,
    const type::Index last,
    const type::Timestamp& travelDate) const
{
  const MileageGetter& mileage = getMileageGetter(geoPath, flightUsages, travelDate);

  GeoIdGlobDirMap::mapped_type geoIdGlobDirVector;
  for (type::Index i = std::min(first, last); i <= std::max(first, last); i++)
  {
    geoIdGlobDirVector.push_back(GeoIdGlobDir(i, mileage.getSingleGlobalDir(first, i)));
  }

  return geoIdGlobDirVector;
}

} // namespace tax
