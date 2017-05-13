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
#include "DomainDataObjects/Itin.h"

namespace tax
{
class FlightUsage;

class ItinBuilder
{
  Itin* _itin;

public:
  ItinBuilder() : _itin(new Itin) {}

  ItinBuilder& setId(const type::Index& newId)
  {
    _itin->id() = newId;
    return *this;
  }

  ItinBuilder& setGeoPath(const GeoPath* geoPath)
  {
    _itin->geoPath() = geoPath;
    return *this;
  }

  ItinBuilder& setGeoPathRefId(type::Index id)
  {
    _itin->geoPathRefId() = id;
    return *this;
  }

  ItinBuilder& setFarePathGeoPathMappingRefId(type::Index id)
  {
    _itin->farePathGeoPathMappingRefId() = id;
    return *this;
  }

  ItinBuilder& addFlightUsage(FlightUsage*& fu)
  {
    _itin->flightUsages().push_back(*fu);
    fu = &_itin->flightUsages().back();
    return *this;
  }

  ItinBuilder& setTravelOriginDate(type::Date date)
  {
    _itin->travelOriginDate() = date;
    return *this;
  }

  ItinBuilder& computeTimeline()
  {
    _itin->computeTimeline();
    return *this;
  }

  Itin* build() { return _itin; }
};
}

