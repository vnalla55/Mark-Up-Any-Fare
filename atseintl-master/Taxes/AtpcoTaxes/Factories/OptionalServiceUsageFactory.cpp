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
#include "DataModel/RequestResponse/InputOptionalServiceUsage.h"
#include "DomainDataObjects/OptionalServiceUsage.h"
#include "Factories/OptionalServiceUsageFactory.h"

namespace tax
{

OptionalServiceUsage
OptionalServiceUsageFactory::createFromInput(
    const InputOptionalServiceUsage& inputOptionalServiceUsage)
{
  OptionalServiceUsage result;
  result.index() = inputOptionalServiceUsage._optionalServiceRefId;
  result.optionalService() = nullptr;
  return result;
}

} // namespace tax
