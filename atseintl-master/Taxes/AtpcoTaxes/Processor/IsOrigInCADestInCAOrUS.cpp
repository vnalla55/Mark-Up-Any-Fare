// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "Processor/IsOrigInCADestInCAOrUS.h"
#include "DomainDataObjects/ItinsPayments.h"
#include "DomainDataObjects/Request.h"

namespace tax
{
IsOrigInCADestInCAOrUS::IsOrigInCADestInCAOrUS(const Request& request, const type::Index itinId)
  : _geoPath(getGeoPath(request, itinId))
{
}

const GeoPath&
IsOrigInCADestInCAOrUS::getGeoPath(const Request& request, const type::Index itinId) const
{
  const Itin& itin = request.getItinByIndex(itinId);
  const type::Index& geoPathRefId = itin.geoPathRefId();
  return request.geoPaths()[geoPathRefId];
}

bool
IsOrigInCADestInCAOrUS::isGeoPathEmpty() const
{
  return _geoPath.geos().empty();
}

bool
IsOrigInCADestInCAOrUS::isFirstGeoInCanada() const
{
  return _geoPath.geos().front().getNation() == "CA";
}

bool
IsOrigInCADestInCAOrUS::isAnyGeoNotInCanadaOrUS() const
{
  for (const Geo& each : _geoPath.geos())
  {
    if (each.getNation() != "CA" && each.getNation() != "US")
      return true;
  }

  return false;
}

bool
IsOrigInCADestInCAOrUS::check() const
{
  return !isGeoPathEmpty() && isFirstGeoInCanada() && !isAnyGeoNotInCanadaOrUS();
}

} // end of tax namespace
