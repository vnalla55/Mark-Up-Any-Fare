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
#include "DataModel/RequestResponse/InputParameter.h"
#include "DomainDataObjects/Parameter.h"
#include "Factories/ParameterFactory.h"

namespace tax
{

Parameter*
ParameterFactory::createFromInput(const InputParameter& inputParameter)
{
  Parameter* result = new Parameter();
  result->name() = inputParameter.name();
  result->value() = inputParameter.value();
  return result;
}

} // namespace tax
