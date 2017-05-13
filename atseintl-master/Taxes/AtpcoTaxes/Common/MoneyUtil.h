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
#pragma once

#include <cmath>
#include <limits>

#include "DataModel/Common/Types.h"

namespace tax
{
inline type::MoneyAmount
doubleToAmount(double inputAmount)
{
  const int factor = 1000000;
  int64_t value = int64_t(round(inputAmount * factor));
  return type::MoneyAmount(value, factor);
}

inline double
amountToDouble(const type::MoneyAmount& inputAmount)
{
  return boost::rational_cast<double>(inputAmount);
}

inline type::MoneyAmount
doubleToPercent(double inputAmount)
{
  return doubleToAmount(inputAmount);
}

inline double
percentToDouble(const type::MoneyAmount& inputAmount)
{
  return amountToDouble(inputAmount);
}
} // namespace tax
