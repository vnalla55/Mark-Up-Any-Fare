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
#pragma once

#include "Common/Timestamp.h"
#include "DataModel/Common/Types.h"

#include <map>
#include <tuple>
#include <vector>

namespace tax
{

class FlightUsage;
class GeoPath;
class MileageGetter;

class MileageService
{
public:
  typedef std::tuple<type::Index, type::Index, type::Index> GeoMapKey;
  typedef std::pair<type::Index, type::Miles> GeoIdMile;
  typedef std::map<GeoMapKey, std::vector<GeoIdMile> > GeoIdMileMap;
  typedef std::pair<type::Index, type::GlobalDirection> GeoIdGlobDir;
  typedef std::map<GeoMapKey, std::vector<GeoIdGlobDir> > GeoIdGlobDirMap;
  typedef std::map<GeoMapKey, type::Miles> DistanceMap;

  MileageService();
  virtual ~MileageService();

  virtual const std::vector<GeoIdMile>&
  getMiles(const GeoPath& geoPath,
           const std::vector<tax::FlightUsage>& flightUsages,
           const type::Index first,
           const type::Index last,
           const type::Timestamp& travelDate) const;

  virtual type::Miles
  getDistance(const GeoPath& geoPath,
              const std::vector<tax::FlightUsage>& flightUsages,
              const type::Index first,
              const type::Index last,
              const type::Timestamp& travelDate) const;

  virtual const std::vector<GeoIdGlobDir>&
  getGlobDir(const GeoPath& geoPath,
             const std::vector<tax::FlightUsage>& flightUsages,
             const type::Index first,
             const type::Index last,
             const type::Timestamp& travelDate) const;

  virtual const std::vector<GeoIdMile>&
  getAllMilesFromStart(const GeoPath& geoPath,
                       const std::vector<tax::FlightUsage>& flightUsages,
                       const type::Timestamp& travelDate) const;

  virtual const MileageGetter& getMileageGetter(const GeoPath& geoPath,
                                                const std::vector<FlightUsage>& flightUsages,
                                                const type::Timestamp& travelDate) const = 0;

  virtual bool isRtw() const { return false; }

protected:
  virtual GeoIdMileMap::mapped_type
  calculateMilesInGeoPath(const GeoPath& geoPath,
                          const std::vector<tax::FlightUsage>& flightUsages,
                          const type::Index first,
                          const type::Index last,
                          const type::Timestamp& travelDate) const;

  virtual GeoIdMileMap::mapped_type
  calculateMilesInGeoPath(const GeoPath& geoPath,
                          const std::vector<tax::FlightUsage>& flightUsages,
                          const type::Index first,
                          const type::Index last,
                          const type::Timestamp& travelDate,
                          bool sort = true) const;

  virtual GeoIdGlobDirMap::mapped_type
  calculateGlobalDirectionInGeoPath(const GeoPath& geoPath,
                                    const std::vector<tax::FlightUsage>& flightUsages,
                                    const type::Index first,
                                    const type::Index last,
                                    const type::Timestamp& travelDate) const;

private:
  mutable GeoIdMileMap _geoIdMileMap;
  mutable DistanceMap _distanceMap;
  mutable GeoIdGlobDirMap _geoIdGlobDirMap;
};

} // namespace tax
