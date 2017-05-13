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
#include "Common/LocationUtil.h"
#include "DataModel/RequestResponse/InputGeoPath.h"
#include "DomainDataObjects/GeoPath.h"
#include "DomainDataObjects/Geo.h"
#include "Factories/GeoFactory.h"
#include "Util/BranchPrediction.h"

namespace tax
{

GeoPath::GeoPath(void)
  : _id(0)
{
}

GeoPath::~GeoPath(void) {}

bool
GeoPath::isJourneyDomestic() const
{
  if (UNLIKELY(_geos.size() == 0))
    return true;

  if (!_isJourneyDomestic)
  {
    type::Nation nation = _geos[0].loc().nation();
    for (size_t i = 1; i < _geos.size(); ++i)
    {
      if (!LocationUtil::isDomestic(nation, _geos[i].getNation()))
      {
        _isJourneyDomestic = false;
        return *_isJourneyDomestic;
      }
    }
    _isJourneyDomestic = true;
  }

  return *_isJourneyDomestic;
}

bool
GeoPath::isJourneyInternational() const
{
  return !isJourneyDomestic();
}

type::Nation
GeoPath::getOriginNation() const
{
  return _geos.size() ? _geos[0].loc().nation() : type::Nation(UninitializedCode);
}

type::Nation
GeoPath::getDestinationNation() const
{
  return _geos.size() ? _geos.back().loc().nation() : type::Nation(UninitializedCode);
}

type::CityCode
GeoPath::getOriginCity() const
{
  return _geos.size() ? _geos[0].loc().cityCode() : type::CityCode(UninitializedCode);
}

type::CityCode
GeoPath::getDestinationCity() const
{
  return _geos.size() ? _geos.back().loc().cityCode() : type::CityCode(UninitializedCode);
}

std::ostream&
GeoPath::print(std::ostream& out, int indentLevel /* = 0 */, char indentChar /* = ' ' */) const
{
  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "ID: " << _id << "\n";

  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "GEOS\n";
  for (Geo const & geo : _geos)
    geo.print(out, indentLevel + 1);

  return out;
}
}
