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
#include "DomainDataObjects/Geo.h"

namespace tax
{

namespace LocUtil
{
bool
isMultiairport(const Geo& first, const Geo& second)
{
  return first.locCode() != second.locCode() && first.cityCode() == second.cityCode();
}
}

Geo::Geo(void)
  : _id(0),
    _unticketedTransfer(type::UnticketedTransfer::No),
    _isLast(false),
    _prev(nullptr),
    _next(nullptr)
{
}

Geo::~Geo(void) {}

void
Geo::makeLast()
{
  _isLast = true;
}

std::ostream&
Geo::print(std::ostream& out, int indentLevel /* = 0 */, char indentChar /* = ' ' */) const
{
  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "ID: " << _id << "\n";

  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "LOC\n";
  _loc.print(out, indentLevel + 1);

  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "UNTICKETEDTRANSFER: " << _unticketedTransfer << "\n";

  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "ISLAST: " << _isLast << "\n";

  return out;
}
}
