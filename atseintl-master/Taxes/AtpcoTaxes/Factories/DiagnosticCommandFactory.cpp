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
#include "DataModel/RequestResponse/InputDiagnosticCommand.h"
#include "DomainDataObjects/DiagnosticCommand.h"
#include "Factories/ParameterFactory.h"
#include "Factories/DiagnosticCommandFactory.h"

namespace tax
{

DiagnosticCommand
DiagnosticCommandFactory::createFromInput(const InputDiagnosticCommand& inputDiagnosticCommand)
{
  DiagnosticCommand result;
  result.number() = inputDiagnosticCommand.number();

  for (const InputParameter & inputParameter : inputDiagnosticCommand.parameters())
  {
    result.parameters().push_back(ParameterFactory::createFromInput(inputParameter));
  }
  return result;
}

} // namespace tax
