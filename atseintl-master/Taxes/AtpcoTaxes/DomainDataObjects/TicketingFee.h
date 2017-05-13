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
#pragma once

#include "DataModel/Common/Types.h"

namespace tax
{
class TicketingFee
{
public:
  TicketingFee(const type::Index& index,
               const type::MoneyAmount& amount,
               const type::MoneyAmount& taxAmount,
               const type::TktFeeSubCode& subCode)
      : _index(index), _amount(amount), _taxAmount(taxAmount), _subCode(subCode)
  {
  }

  type::Index& index() { return _index; }
  const type::Index& index() const { return _index; }

  type::MoneyAmount& amount() { return _amount; }
  const type::MoneyAmount& amount() const { return _amount; }

  type::MoneyAmount& taxAmount() { return _taxAmount; }
  const type::MoneyAmount& taxAmount() const { return _taxAmount; }

  type::TktFeeSubCode& subCode() { return _subCode; }
  const type::TktFeeSubCode& subCode() const { return _subCode; }

  std::ostream& print(std::ostream& out, int indentLevel = 0, char indentChar = ' ') const;

private:
  type::Index _index;
  type::MoneyAmount _amount;
  type::MoneyAmount _taxAmount;
  type::TktFeeSubCode _subCode;
};

}

