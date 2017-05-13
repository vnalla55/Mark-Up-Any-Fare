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
#include "DomainDataObjects/FareUsage.h"
#include "DomainDataObjects/Fare.h"

namespace tax
{

FareUsage::FareUsage()
  : _index(0),
    _fare(nullptr)
{
}

FareUsage::~FareUsage() {}

std::ostream&
FareUsage::print(std::ostream& out, int indentLevel /* = 0 */, char indentChar /* = ' ' */) const
{
  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "INDEX: " << _index << "\n";

  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "FARE\n";
  if (_fare != nullptr)
    _fare->print(out, indentLevel + 1);
  else
    out << " NULL\n";

  return out;
}

} // namespace tax
