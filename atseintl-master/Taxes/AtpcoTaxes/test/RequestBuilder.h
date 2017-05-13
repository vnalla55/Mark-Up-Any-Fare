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
#include "DomainDataObjects/Request.h"

namespace tax
{
class GeoPath;
class Itin;
class Flight;

class RequestBuilder
{
  Request* _request;

public:
  RequestBuilder() : _request(new Request) {}

  RequestBuilder& addGeoPathMappings(GeoPathMapping* geoPathMapping)
  {
    _request->geoPathMappings().push_back(*geoPathMapping);
    return *this;
  }

  RequestBuilder& addGeoPaths(GeoPath*& geoPath)
  {
    _request->geoPaths().push_back(*geoPath);
    geoPath = &_request->geoPaths().back();
    return *this;
  }

  RequestBuilder& addItin(Itin*& itin)
  {
    _request->allItins().push_back(*itin);
    _request->itins().push_back(&_request->allItins().back());
    itin = &_request->allItins().back();
    return *this;
  }

  RequestBuilder& addFlight(Flight*& flight)
  {
    _request->flights().push_back(*flight);
    flight = &_request->flights().back();
    return *this;
  }

  RequestBuilder& addTaxPoint(const type::Nation& nation, const type::TaxPointTag& tag)
  {
    Geo geo;
    geo.loc().tag() = tag;
    geo.loc().nation() = nation;
    geo.id() = _request->posTaxPoints().size();

    _request->posTaxPoints().push_back(geo);
    return *this;
  }

  Request* build() { return _request; }
};
}

