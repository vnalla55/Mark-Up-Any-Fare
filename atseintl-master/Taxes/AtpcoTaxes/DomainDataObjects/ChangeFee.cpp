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
#include "DomainDataObjects/ChangeFee.h"
#include "Common/MoneyUtil.h"

namespace tax
{
ChangeFee::ChangeFee(void) : _amount(0)
{
}

ChangeFee::~ChangeFee(void)
{
}

std::ostream&
ChangeFee::print(std::ostream& out, int indentLevel /* = 0 */, char indentChar /* = ' ' */) const
{
  const std::string indent(indentLevel, indentChar);

  out << indent << "AMOUNT: " << amountToDouble(_amount) << "\n";

  return out;
}
}
