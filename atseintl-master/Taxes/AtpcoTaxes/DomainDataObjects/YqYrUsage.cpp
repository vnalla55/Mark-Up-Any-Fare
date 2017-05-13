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
#include "DomainDataObjects/YqYrUsage.h"
#include "DomainDataObjects/YqYr.h"

namespace tax
{

YqYrUsage::YqYrUsage() : _index(0), _yqYr(nullptr) {}

YqYrUsage::~YqYrUsage() {}

std::ostream&
YqYrUsage::print(std::ostream& out, int indentLevel /* = 0 */, char indentChar /* = ' ' */) const
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
  out << "YQYR\n";
  if (_yqYr != nullptr)
    _yqYr->print(out, indentLevel + 1);
  else
    out << " NULL\n";

  return out;
}

} // namespace tax
