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
#include "DataModel/RequestResponse/InputOptionalServicePath.h"
#include "DomainDataObjects/OptionalServicePath.h"
#include "Factories/FactoryUtils.h"
#include "Factories/OptionalServiceUsageFactory.h"
#include "Factories/OptionalServicePathFactory.h"

namespace tax
{

OptionalServicePath
OptionalServicePathFactory::createFromInput(
    const InputOptionalServicePath& inputOptionalServicePath)
{
  OptionalServicePath result;
  result.index() = inputOptionalServicePath._id;

  create<OptionalServiceUsageFactory>(
      inputOptionalServicePath._optionalServiceUsages, result.optionalServiceUsages());
  return result;
}

} // namespace tax
