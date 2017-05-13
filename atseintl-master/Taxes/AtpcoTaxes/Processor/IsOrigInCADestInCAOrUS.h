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
#pragma once

#include "Processor/ExcItinTaxesSelector.h"
#include "DataModel/Common/Types.h"

namespace tax
{
class GeoPath;
class Request;

class IsOrigInCADestInCAOrUS
{
  const GeoPath& _geoPath;

  const GeoPath& getGeoPath(const Request& request, type::Index itinId) const;
  bool isGeoPathEmpty() const;
  bool isFirstGeoInCanada() const;
  bool isAnyGeoNotInCanadaOrUS() const;

public:
  IsOrigInCADestInCAOrUS(const Request& request, type::Index itinId);
  bool check() const;
};

} // end of tax namespace
