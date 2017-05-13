//-------------------------------------------------------------------
//
//  Copyright Sabre 2012
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#pragma once

#include "RexPricing/PaxTypeFareWrapper.h"

#include <cmath>
#include <functional>

namespace tse
{

class CheckAmount : public std::unary_function<PaxTypeFareWrapper, bool>
{
public:
  CheckAmount(const MoneyAmount& amount, double tolerance) : _tolerance(tolerance), _amount(amount)
  {
  }

  bool operator()(const PaxTypeFareWrapper& wrp) const
  {
    return std::abs(wrp.getAmount() - _amount) < _tolerance;
  }

private:
  const double _tolerance;
  MoneyAmount _amount;
};

class CheckVarianceAmount : public std::unary_function<PaxTypeFareWrapper, bool>
{
public:
  CheckVarianceAmount(const MoneyAmount& amount, const double& variance)
    : _variance(variance + EPSILON), _amount(amount)
  {
  }

  bool operator()(const PaxTypeFareWrapper& wrp) const
  {
    return std::abs(wrp.getAmount() - _amount) < _variance * std::abs(wrp.getAmount());
  }

protected:
  const double _variance;
  const MoneyAmount _amount;
};

} // tse

