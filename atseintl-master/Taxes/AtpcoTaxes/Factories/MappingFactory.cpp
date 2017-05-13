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
#include "DataModel/RequestResponse/InputMapping.h"
#include "DomainDataObjects/Mapping.h"
#include "Factories/FactoryUtils.h"
#include "Factories/MapFactory.h"
#include "Factories/MappingFactory.h"

namespace tax
{

Mapping
MappingFactory::createFromInput(const InputMapping& inputMapping)
{
  Mapping result;
  create<MapFactory>(inputMapping.maps(), result.maps());
  return result;
}

} // namespace tax
