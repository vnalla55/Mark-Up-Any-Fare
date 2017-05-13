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
#include "DomainDataObjects/OptionalServicePath.h"

namespace tax
{

OptionalServicePath::OptionalServicePath(void) : _index(0) {}

OptionalServicePath::~OptionalServicePath(void) {}

std::ostream&
OptionalServicePath::print(std::ostream& out,
                           int indentLevel /* = 0 */,
                           char indentChar /* = ' ' */) const
{
  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "OPTIONALSERVICEUSAGES\n";
  for (OptionalServiceUsage const & optionalServiceUsage : _optionalServiceUsages)
    optionalServiceUsage.print(out, indentLevel + 1);

  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "INDEX: " << _index << "\n";

  return out;
}

} // namespace tax
