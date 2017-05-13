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
#include "DomainDataObjects/PointOfSale.h"
#include "DataModel/Common/CodeIO.h"

namespace tax
{

PointOfSale::PointOfSale(void) : _id(0) {}

PointOfSale::~PointOfSale(void) {}

std::ostream&
PointOfSale::print(std::ostream& out, int indentLevel /* = 0 */, char indentChar /* = ' ' */) const
{
  for (int i = 0; i < indentLevel; ++i)
  {
    out << indentChar;
  }
  out << "LOC: " << _loc << "\n";

  return out;
}
} // namespace tax
