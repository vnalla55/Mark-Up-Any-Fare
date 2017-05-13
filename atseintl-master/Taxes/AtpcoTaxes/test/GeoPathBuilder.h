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

#pragma once
#include "DomainDataObjects/GeoPath.h"

namespace tax
{
class GeoPathBuilder
{
  GeoPath* _geoPath;

public:
  GeoPathBuilder() : _geoPath(new GeoPath) {}

  GeoPathBuilder& addGeo(const type::AirportCode& code,
                         const type::CityCode& city,
                         const type::Nation& nation,
                         const type::TaxPointTag& tag)
  {
    GeoPathBuilder& builder = addGeo(nation, tag);
    Geo& geo = builder._geoPath->geos().back();
    geo.loc().cityCode() = city;
    geo.loc().code() = code;

    return builder;
  }

  GeoPathBuilder& addGeo(const type::Nation& nation, const type::TaxPointTag& tag)
  {
    Geo geo;
    geo.loc().tag() = tag;
    geo.loc().nation() = nation;
    geo.id() = _geoPath->geos().size();

    Geo* prevGeo = (geo.id() > 0) ? &_geoPath->geos().back() : nullptr;

    _geoPath->geos().push_back(geo);

    if (prevGeo)
      prevGeo->setNext(&_geoPath->geos().back());
    _geoPath->geos().back().setPrev(prevGeo);

    return *this;
  }

  GeoPath* build() { return _geoPath; }
};
}
