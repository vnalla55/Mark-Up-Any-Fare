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
#include "DomainDataObjects/Fare.h"
#include "Common/MoneyUtil.h"
#include "Common/SafeEnumToString.h"

namespace tax
{

std::ostream&
Fare::print(std::ostream& out, int indentLevel /* = 0 */, char indentChar /* = ' ' */) const
{
  const std::string indent(indentLevel, indentChar);

  out << indent << "FAREBASISCODE: " << _basis << "\n"
      << indent << "FARETYPECODE: " << _type << "\n"
      << indent << "ONEWAYROUNDTRIP: " << _oneWayRoundTrip << "\n"
      << indent << "DIRECTIONALITY: " << _directionality << "\n"
      << indent << "FAREAMOUNT: " << amountToDouble(_amount) << "\n"
      << indent << "FAREMARKUPAMOUNT: " << amountToDouble(_markupAmount) << "\n";

  return out;
}
}
