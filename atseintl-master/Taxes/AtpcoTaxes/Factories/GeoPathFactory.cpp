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
#include "DataModel/RequestResponse/InputGeoPath.h"
#include "DomainDataObjects/GeoPath.h"
#include "Factories/GeoFactory.h"
#include "Factories/GeoPathFactory.h"

namespace tax
{

GeoPath
GeoPathFactory::createFromInput(const InputGeoPath& inputGeoPath)
{
  GeoPath result;
  result.id() = inputGeoPath._id;

  for (const InputGeo& inputGeo : inputGeoPath._geos)
  {
    result.geos().push_back(GeoFactory::createFromInput(inputGeo));
  }
  return result;
}

} // namespace tax

