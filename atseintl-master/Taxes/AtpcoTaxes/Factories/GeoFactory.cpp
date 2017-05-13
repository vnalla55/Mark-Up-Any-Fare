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
#include "DataModel/RequestResponse/InputGeo.h"
#include "DomainDataObjects/Geo.h"
#include "Factories/LocFactory.h"
#include "Factories/GeoFactory.h"

namespace tax
{

Geo
GeoFactory::createFromInput(const InputGeo& inputGeo)
{
  Geo result;
  result.id() = inputGeo._id;
  result.loc() = LocFactory::createFromInput(inputGeo._loc);
  result.unticketedTransfer() = inputGeo._unticketedTransfer;
  return result;
}

} // namespace tax
