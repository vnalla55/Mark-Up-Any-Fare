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
#include "DataModel/RequestResponse/InputMap.h"
#include "DomainDataObjects/Map.h"
#include "Factories/MapFactory.h"

namespace tax
{

Map
MapFactory::createFromInput(const InputMap& inputMap)
{
  Map result;
  result.index() = inputMap._geoRefId;
  return result;
}

} // namespace tax
