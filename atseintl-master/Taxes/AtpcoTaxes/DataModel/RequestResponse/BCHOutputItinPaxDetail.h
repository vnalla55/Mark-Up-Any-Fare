// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include <set>

namespace tax
{
class BCHOutputItinPaxDetail
{
public:
  BCHOutputItinPaxDetail() {}

  BCHOutputItinPaxDetail& operator=(const BCHOutputItinPaxDetail&) = delete;

  void setPtc(const type::PassengerCode& ptc) { _ptc = ptc; }
  const type::PassengerCode& getPtc() const { return _ptc; }

  void setPtcNumber(type::Index ptcNumber) { _ptcNumber = ptcNumber; }
  const type::Index& getPtcNumber() const { return _ptcNumber; }

  void setTotalAmount(const type::MoneyAmount& amount) { _totalAmount = amount; }
  void increaseTotalAmount(const type::MoneyAmount& amount) { _totalAmount += amount; }
  const type::MoneyAmount& getTotalAmount() const { return _totalAmount; }

  std::set<type::Index>& mutableTaxIds() { return _taxIds; }
  const std::set<type::Index>& constTaxIds() const { return _taxIds; }

private:
  type::PassengerCode _ptc;
  type::Index _ptcNumber{1};
  type::MoneyAmount _totalAmount{0};
  std::set<type::Index> _taxIds;
};
}
