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
#include "Common/SafeEnumToString.h"
#include "DomainDataObjects/Loc.h"
#include "DataModel/Common/CodeIO.h"

namespace tax
{

Loc::Loc(void) : _taxPointTag{type::TaxPointTag::Sale}, _inBufferZone{false} {}

Loc::~Loc(void) {}

std::ostream&
Loc::print(std::ostream& out, int indentLevel /* = 0 */, char indentChar /* = ' ' */) const
{
  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "LOCCODE: " << _code << "\n";

  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "TAXPOINTTAG: " << _taxPointTag << "\n";

  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "NATION: " << _nation << "\n";

  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "CITYCODE: " << _cityCode << "\n";


  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "BUFFERZONE: " << _inBufferZone << "\n";

  return out;
}
} // namespace tax
