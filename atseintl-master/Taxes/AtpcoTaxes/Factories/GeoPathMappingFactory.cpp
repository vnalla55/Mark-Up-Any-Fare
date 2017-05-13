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
#include "DataModel/RequestResponse/InputGeoPathMapping.h"
#include "DomainDataObjects/GeoPathMapping.h"
#include "Factories/FactoryUtils.h"
#include "Factories/GeoPathMappingFactory.h"
#include "Factories/MappingFactory.h"

namespace tax
{

GeoPathMapping
GeoPathMappingFactory::createFromInput(const InputGeoPathMapping& inputGeoPathMapping)
{
  GeoPathMapping result;
  create<MappingFactory>(inputGeoPathMapping._mappings, result.mappings());
  return result;
}

} // namespace tax
