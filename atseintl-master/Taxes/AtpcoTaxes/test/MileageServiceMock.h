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

#include <vector>

#include "DomainDataObjects/MileageGetterServer.h"
#include "ServiceInterfaces/MileageService.h"

namespace tax
{

class FlightUsage;

class MileageServiceMock : public MileageService
{
public:
  MileageServiceMock();
  virtual ~MileageServiceMock();
  virtual MileageServiceMock& pushIndex(type::Index newIndex);
  virtual MileageServiceMock& pushPair(GeoIdMile newPair);
  virtual MileageServiceMock& pushData(type::Index newIndex, type::Miles miles, type::GlobalDirection globDir);
  virtual void setDistance(type::Index first, type::Index last, type::Miles miles);
  void clear();

  const MileageGetter& getMileageGetter(const GeoPath&,
                                        const std::vector<FlightUsage>&,
                                        const type::Timestamp&) const
  {
    return _mileageGetterServer;
  }

  type::Miles getDistance(const GeoPath& /*geoPath*/,
                          const std::vector<tax::FlightUsage>& /*flightUsages*/,
                          const type::Index first,
                          const type::Index last,
                          const type::Timestamp& /*travelDate*/) const override
  {
    std::pair<type::Index, type::Index> key(first, last);
    if (_distanceMap.find(key) != _distanceMap.end())
      return _distanceMap[key];

    throw std::runtime_error("MileageService::getDistance() should not be called!");
  }

  bool isRtw() const { return _isRtw; }
  void setRtw(bool rtw) { _isRtw = rtw; }

protected:
  virtual GeoIdMileMap::mapped_type
  calculateMilesInGeoPath(const GeoPath & /*geoPath*/,
                          const std::vector<FlightUsage> &,
                          const type::Index first, const type::Index last,
                          const type::Timestamp &, bool) const;

private:
  bool _isRtw;
  mutable std::vector<GeoIdMile> _miles;
  mutable std::map<std::pair<type::Index, type::Index>, type::Miles> _distanceMap;
  MileageGetterServer _mileageGetterServer;
};

} // namespace tax
