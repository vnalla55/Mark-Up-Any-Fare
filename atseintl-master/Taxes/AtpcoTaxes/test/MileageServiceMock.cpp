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
#include "test/MileageServiceMock.h"

namespace tax
{

MileageServiceMock::MileageServiceMock() : _isRtw(false)
{
}

MileageServiceMock::~MileageServiceMock()
{
}

MileageServiceMock& MileageServiceMock::pushPair(GeoIdMile newPair)
{
  _miles.push_back(newPair);
  return *this;
}

MileageServiceMock& MileageServiceMock::pushIndex(type::Index newOrder)
{
  return pushPair(GeoIdMile(newOrder, 0));
}

MileageServiceMock& MileageServiceMock::pushData(type::Index newIndex, type::Miles miles, type::GlobalDirection /*gdir*/)
{
  _miles.push_back(GeoIdMile(newIndex, miles));
  return *this;
}

void MileageServiceMock::setDistance(type::Index first, type::Index last, type::Miles miles)
{
  _distanceMap[{first, last}] = miles;
}

class IsOutOfRange
{
public:
  IsOutOfRange(const type::Index lower, const type::Index upper) : _lower(lower), _upper(upper)
  {
  }

  bool operator()(MileageServiceMock::GeoIdMile value)
  {
    return ((value.first < _lower) || (_upper < value.first));
  }

private:
  const type::Index _lower;
  const type::Index _upper;
};

MileageService::GeoIdMileMap::mapped_type
MileageServiceMock::calculateMilesInGeoPath(
    const GeoPath & /*geoPath*/, const std::vector<FlightUsage> &,
    const type::Index first, const type::Index last, const type::Timestamp &,
    bool) const
{
  std::vector<GeoIdMile> miles = _miles;
  if ((first != 0) || (last != 0))
  {
    const bool isForward = (first <= last);
    const type::Index& smaller = isForward ? first : last;
    const type::Index& bigger = isForward ? last : first;

    // remove points outside the range


    miles.erase(std::remove_if(miles.begin(), miles.end(), IsOutOfRange(smaller, bigger)),
                miles.end());
  }
  return miles;
}

void MileageServiceMock::clear()
{
  _miles.clear();
}

} // namespace tax
