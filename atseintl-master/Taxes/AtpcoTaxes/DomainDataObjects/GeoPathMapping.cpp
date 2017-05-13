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
#include "DomainDataObjects/GeoPathMapping.h"

namespace tax
{

GeoPathMapping::GeoPathMapping(void) {}

GeoPathMapping::~GeoPathMapping(void) {}

std::ostream&
GeoPathMapping::print(std::ostream& out, int indentLevel /* = 0 */, char indentChar /* = ' ' */)
    const
{
  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "MAPPINGS\n";
  for (Mapping const & mapping : _mappings)
    mapping.print(out, indentLevel + 1);

  return out;
}

} // namespace tax
