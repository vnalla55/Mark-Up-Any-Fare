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
#include "DataModel/RequestResponse/InputChangeFee.h"
#include "DomainDataObjects/ChangeFee.h"
#include "Factories/ChangeFeeFactory.h"

namespace tax
{
ChangeFee
ChangeFeeFactory::createFromInput(const InputChangeFee& inputChangeFee)
{
  ChangeFee result;
  result.amount() = inputChangeFee._amount;
  return result;
}

} // namespace tax
