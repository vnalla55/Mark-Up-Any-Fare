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
#include "DomainDataObjects/YqYrPath.h"

namespace tax
{

YqYrPath::YqYrPath(void) : _totalAmount(0) {}

YqYrPath::~YqYrPath(void) {}

std::ostream&
YqYrPath::print(std::ostream& out, int indentLevel /* = 0 */, char indentChar /* = ' ' */) const
{
  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "YQYRUSAGES\n";
  for (YqYrUsage const & yqYrUsage : _yqYrUsages)
    yqYrUsage.print(out, indentLevel + 1);

  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "TOTALAMOUNT: " << _totalAmount << "\n";

  return out;
}

} // namespace tax
