// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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
#include <exception>

#include "Common/MoneyUtil.h"
#include "DataModel/Common/CodeIO.h"
#include "DomainDataObjects/TicketingFee.h"
#include "Rules/BusinessRule.h"

namespace tax
{

std::ostream&
TicketingFee::print(std::ostream& out, int indentLevel, char indentChar) const
{
  const std::string indent(indentLevel, indentChar);
  out << indent << "INDEX: " << _index << '\n'
      << indent << "AMOUNT: " << amountToDouble(_amount) << '\n'
      << indent << "TAXAMOUNT: " << amountToDouble(_taxAmount) << '\n'
      << indent << "SUBCODE: " << _subCode << '\n';

  return out;
}

} // namespace tax
