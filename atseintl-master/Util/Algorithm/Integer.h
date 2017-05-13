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
// ----------------------------------------------------------------
#pragma once

#include <stdint.h>

namespace tse
{
using uint128_t = unsigned __int128;
using int128_t = __int128;

namespace alg
{
constexpr inline unsigned
significantBits(uint8_t x)
{
  return 8 * sizeof(int) - __builtin_clz(x);
}
constexpr inline unsigned
significantBits(uint16_t x)
{
  return 8 * sizeof(int) - __builtin_clz(x);
}
constexpr inline unsigned
significantBits(uint32_t x)
{
  return 8 * sizeof(int) - __builtin_clz(x);
}
constexpr inline unsigned
significantBits(uint64_t x)
{
  return 8 * sizeof(long long) - __builtin_clzll(x);
}

constexpr inline unsigned
trailingZeros(uint8_t x)
{
  return __builtin_ctz(x);
}
constexpr inline unsigned
trailingZeros(uint16_t x)
{
  return __builtin_ctz(x);
}
constexpr inline unsigned
trailingZeros(uint32_t x)
{
  return __builtin_ctz(x);
}
constexpr inline unsigned
trailingZeros(uint64_t x)
{
  return __builtin_ctzll(x);
}

constexpr inline uint8_t
mulHigh(uint16_t a, uint16_t b)
{
  return (a * b) >> 8;
}
constexpr inline int8_t
mulHigh(int16_t a, int16_t b)
{
  return (a * b) >> 8;
}

constexpr inline uint16_t
mulHigh(uint32_t a, uint32_t b)
{
  return (a * b) >> 16;
}
constexpr inline int16_t
mulHigh(int32_t a, int32_t b)
{
  return (a * b) >> 16;
}

constexpr inline uint32_t
mulHigh(uint64_t a, uint64_t b)
{
  return (a * b) >> 32;
}
constexpr inline int32_t
mulHigh(int64_t a, int64_t b)
{
  return (a * b) >> 32;
}

constexpr inline uint64_t
mulHigh(uint128_t a, uint128_t b)
{
  return (a * b) >> 64;
}
constexpr inline int64_t
mulHigh(int128_t a, int128_t b)
{
  return (a * b) >> 64;
}

// WARNING The rotate is undefined if bits >= 8*sizeof value.
inline uint8_t
rotateLeft(uint8_t value, uint8_t bits)
{
  asm("rolb %%cl, %0" :
      "=r"(value) :
      "0"(value), "c"(bits)
  );
  return value;
}
inline uint16_t
rotateLeft(uint16_t value, uint8_t bits)
{
  asm("rolw %%cl, %0" :
      "=r"(value) :
      "0"(value), "c"(bits)
  );
  return value;
}
inline uint32_t
rotateLeft(uint32_t value, uint8_t bits)
{
  asm("roll %%cl, %0" :
      "=r"(value) :
      "0"(value), "c"(bits)
  );
  return value;
}
inline uint64_t
rotateLeft(uint64_t value, uint8_t bits)
{
  asm("rolq %%cl, %0" :
      "=r"(value) :
      "0"(value), "c"(bits)
  );
  return value;
}

inline uint8_t
rotateRight(uint8_t value, uint8_t bits)
{
  asm("rorb %%cl, %0" :
      "=r"(value) :
      "0"(value), "c"(bits)
  );
  return value;
}
inline uint16_t
rotateRight(uint16_t value, uint8_t bits)
{
  asm("rorw %%cl, %0" :
      "=r"(value) :
      "0"(value), "c"(bits)
  );
  return value;
}
inline uint32_t
rotateRight(uint32_t value, uint8_t bits)
{
  asm("rorl %%cl, %0" :
      "=r"(value) :
      "0"(value), "c"(bits)
  );
  return value;
}
inline uint64_t
rotateRight(uint64_t value, uint8_t bits)
{
  asm("rorq %%cl, %0" :
      "=r"(value) :
      "0"(value), "c"(bits)
  );
  return value;
}
}
}
