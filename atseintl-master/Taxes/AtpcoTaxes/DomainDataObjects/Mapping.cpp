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
#include "DomainDataObjects/Mapping.h"
#include "Factories/MapFactory.h"

namespace tax
{

Mapping::Mapping(void) {}

Mapping::~Mapping(void) {}

std::ostream&
Mapping::print(std::ostream& out, int indentLevel /* = 0 */, char indentChar /* = ' ' */) const
{
  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "MAPS\n";
  for (const Map & map : _maps)
    map.print(out, indentLevel + 1);

  return out;
}

} // namespace tax
