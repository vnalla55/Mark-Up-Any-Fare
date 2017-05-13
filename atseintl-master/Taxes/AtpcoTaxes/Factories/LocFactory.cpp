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
#include "DataModel/RequestResponse/InputLoc.h"
#include "DomainDataObjects/Loc.h"
#include "Factories/LocFactory.h"

namespace tax
{

Loc
LocFactory::createFromInput(const InputLoc& inputLoc)
{
  Loc result;
  result.code() = inputLoc.code();
  result.tag() = inputLoc.tag();
  result.nation() = inputLoc.nation();
  result.cityCode() = inputLoc.cityCode();
  result.inBufferZone() = inputLoc.inBufferZone();
  return result;
}

} // namespace tax

