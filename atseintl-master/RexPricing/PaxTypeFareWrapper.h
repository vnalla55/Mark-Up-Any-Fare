//-------------------------------------------------------------------
//
//  Copyright Sabre 2008
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/TseConsts.h"
#include "DataModel/PaxTypeFare.h"

#include <cmath>

namespace tse
{
class PaxTypeFare;

struct PaxTypeFareWrapper
{
public:
  PaxTypeFareWrapper(PaxTypeFare* ptf) : _ptf(ptf), _amount(0.0) {}

  const MoneyAmount& getAmount() const { return _amount; }
  void setAmount(const MoneyAmount& amount) { _amount = amount; }

  PaxTypeFare* get() { return _ptf; }
  const PaxTypeFare* get() const { return _ptf; }

  bool isIndustry() const { return _ptf->fare()->isIndustry(); }

  double getVarianceOld(const MoneyAmount& baseAmount) const
  {
    return (_amount > EPSILON ? std::abs(baseAmount / _amount - 1.0) * HUNDRED : -2 * EPSILON);
  }

  double getVariance(const MoneyAmount& baseAmount) const
  {
    return (_amount > EPSILON ? std::abs(baseAmount / _amount - 1.0) : -2 * EPSILON);
  }

protected:
  PaxTypeFare* _ptf;
  MoneyAmount _amount;
};

} // tse

