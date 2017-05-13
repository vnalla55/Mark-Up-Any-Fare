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
#include "DomainDataObjects/DiagnosticCommand.h"

namespace tax
{

DiagnosticCommand::DiagnosticCommand(void) : _number(0) {}

std::ostream&
DiagnosticCommand::print(std::ostream& out, int indentLevel, char indentChar /* = ' ' */) const
{
  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "NUMBER " << _number << "\n";

  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "PARAMETERS\n";
  for (Parameter const & parameter : _parameters)
    parameter.print(out, indentLevel + 1);

  return out;
}

} // namespace tax
