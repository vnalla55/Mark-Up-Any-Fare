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
#include "DataModel/RequestResponse/InputYqYrPath.h"
#include "DomainDataObjects/YqYrPath.h"
#include "Factories/FactoryUtils.h"
#include "Factories/YqYrUsageFactory.h"
#include "Factories/YqYrPathFactory.h"

namespace tax
{

YqYrPath
YqYrPathFactory::createFromInput(const InputYqYrPath& inputYqYrPath)
{
  YqYrPath result;
  result.totalAmount() = inputYqYrPath._totalAmount;

  create<YqYrUsageFactory>(inputYqYrPath._yqYrUsages, result.yqYrUsages());
  return result;
}

} // namespace tax
