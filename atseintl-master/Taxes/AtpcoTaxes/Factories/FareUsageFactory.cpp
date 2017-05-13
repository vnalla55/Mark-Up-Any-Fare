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
#include "DataModel/RequestResponse/InputFareUsage.h"
#include "DomainDataObjects/FareUsage.h"
#include "Factories/FareUsageFactory.h"

namespace tax
{

FareUsage
FareUsageFactory::createFromInput(const InputFareUsage& inputFareUsage)
{
  FareUsage result;
  result.index() = inputFareUsage._fareRefId;
  result.fare() = nullptr;
  return result;
}

} // namespace tax
