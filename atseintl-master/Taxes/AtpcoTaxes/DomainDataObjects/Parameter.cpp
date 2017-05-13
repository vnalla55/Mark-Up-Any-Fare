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
#include "DomainDataObjects/Parameter.h"

namespace tax
{

std::ostream&
Parameter::print(std::ostream& out, int indentLevel, char indentChar /* = ' ' */) const
{
  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "NAME " << _name << "\n";

  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "VALUE " << _value << "\n";

  return out;
}

} // namespace tax
