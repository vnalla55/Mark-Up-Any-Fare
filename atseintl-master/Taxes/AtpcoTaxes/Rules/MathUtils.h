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

#include "DataModel/Common/Types.h"
#include "Util/BranchPrediction.h"

namespace tax
{
class MathUtils
{
public:
  static type::MoneyAmount adjustDecimal(const type::MoneyAmount& amount, type::CurDecimals noDec)
  {
    static type::MoneyAmount pow[19] = {
      1L,                 10L,                 100L,                1000L, // 1e3
      10000L,             100000L,             1000000L, // 1e6
      10000000L,          100000000L,          1000000000L, // 1e9
      10000000000L,       100000000000L,       1000000000000L, // 1e12
      10000000000000L,    100000000000000L,    1000000000000000L, // 1e15
      10000000000000000L, 100000000000000000L, 1000000000000000000L // 1e18
    };
    if (UNLIKELY(noDec >= 19))
      return 0;
    return amount / pow[noDec];
  }

  template <typename T>
  static void clamp(const T& lower, const T& upper, T& value)
  {
    if ((lower != 0) && (value < lower))
    {
      value = lower;
    }
    if ((upper != 0) && (upper < value))
    {
      value = upper;
    }
  }
};
}
