// ----------------------------------------------------------------
//
//   Copyright Sabre 2014
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#ifndef _GLIBCXX_DEBUG

#include "Util/Vector.h"

namespace tse
{
namespace detail
{
uint32_t
nearestPowerOfTwo(uint32_t number)
{
  --number;

  number |= (number >> 1);
  number |= (number >> 2);
  number |= (number >> 4);
  number |= (number >> 8);
  number |= (number >> 16);

  return ++number;
}

uint32_t
nextSize(const uint32_t currentSize, const uint32_t neededSize)
{
  if (neededSize == 0)
    return 0;

  if (neededSize > currentSize)
  {
    if (neededSize <= 2 * currentSize)
      return currentSize * 2;
    return nearestPowerOfTwo(neededSize);
  }

  if (neededSize * 4 > currentSize || currentSize <= 16)
    return currentSize;

  if (neededSize * 8 > currentSize)
    return currentSize / 2;
  return nearestPowerOfTwo(neededSize);
}
}
}

#endif // _GLIBCXX_DEBUG
