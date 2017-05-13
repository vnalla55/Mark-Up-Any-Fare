// ----------------------------------------------------------------
//
//   Copyright Sabre 2015
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
//  Description:
//    A class that precomputes division by variable. It is efficient
//    when you perform many divisions by the same divisor that is
//    not known statically.
//
// ----------------------------------------------------------------
#pragma once

#include "Util/Algorithm/Integer.h"

#include <type_traits>

#include <assert.h>
#include <stdint.h>

namespace tse
{
template <typename T>
class Divisor
{
public:
  using value_type = T;

  static_assert(std::is_integral<value_type>::value, "T must be integral");
  static_assert(std::is_unsigned<value_type>::value, "T must be unsigned");

  static constexpr unsigned Width = 8 * sizeof(value_type);

  Divisor() = default;
  Divisor(value_type divisor)
  {
    assert(divisor != 0);

    unsigned zeros = alg::trailingZeros(divisor);
    divisor >>= zeros;

    if (divisor == 1)
    {
      // divisor is a power of two.
      _divisor = (big_uint(1) << Width) - 1;
      _scale = zeros;
      _correction = 1;
      return;
    }

    const unsigned b = alg::significantBits(divisor) - 1;
    const unsigned r = Width + b;

    const value_type f = (big_uint(1) << r) / divisor;
    const value_type fr = (big_uint(1) << r) % divisor;

    if (fr <= divisor / 2)
    {
      _divisor = f;
      _scale = b + zeros;
      _correction = 1;
    }
    else
    {
      _divisor = f + 1;
      _scale = b + zeros;
      _correction = 0;
    }
  }

  friend value_type operator/(value_type dividend, Divisor divisor)
  {
    return alg::mulHigh(big_uint(big_uint(dividend) + divisor._correction),
                        big_uint(divisor._divisor)) >>
           divisor._scale;
  }

  value_type operator()(value_type dividend) const { return dividend / *this; }

private:
  using big_uint =
      typename std::conditional<sizeof(value_type) == 1, uint16_t,
        typename std::conditional<sizeof(value_type) == 2, uint32_t,
          typename std::conditional<sizeof(value_type) == 4, uint64_t,
            uint128_t
          >::type
        >::type
      >::type;

  value_type _divisor = 0;
  uint8_t _scale = 0;
  uint8_t _correction = 0;
};
}
