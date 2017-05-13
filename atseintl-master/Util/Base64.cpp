
//*********************************************************************
//* Base64 - a simple base64 encoder and decoder.
//*
//*     Copyright (c) 1999, Bob Withers - bwit@pobox.com
//*
//* This code may be freely used for any purpose, either personal
//* or commercial, provided the authors copyright notice remains
//* intact.
//*
//* Enhancements by Stanley Yamane:
//*     o reverse lookup table for the decode function
//*     o reserve string buffer space in advance
//*
//* Further refactored by Gustaw Smolarczyk:
//*     o clean up
//*     o optimize
//*     o throw on wrong input to decode
//*
//*********************************************************************

#include "Util/Base64.h"

#include <stdint.h>

static const char fillchar = '=';
static const uint8_t np = 0xFF;
static const uint8_t mask = 0x3F;

namespace tse
{

static const char* encodeTable =
//   0000000000111111111122222222223333333333444444444455555555556666
//   0123456789012345678901234567890123456789012345678901234567890123
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// Decode Table gives the index of any valid base64 character in the Base64 table]
// 65 == A, 97 == a, 48 == 0, 43 == +, 47 == /
// For invalid base64 characters, it gives np.

static const uint8_t decodeTable[256] =
{
//x0  x1  x2  x3  x4  x5  x6  x7  x8  x9  xA  xB  xC  xD  xE  xF
  np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, // 0x
  np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, // 1x
  np, np, np, np, np, np, np, np, np, np, np, 62, np, np, np, 63, // 2x
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, np, np, np, np, np, np, // 3x
  np,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, // 4x
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, np, np, np, np, np, // 5x
  np, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, // 6x
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, np, np, np, np, np, // 7x
  np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, // 8x
  np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, // 9x
  np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, // Ax
  np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, // Bx
  np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, // Cx
  np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, // Dx
  np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, // Ex
  np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, np, // Fx
};

Base64::DecodeError::DecodeError() : runtime_error("Wrong base64 input string") {}

std::string
Base64::encode(const std::string& data)
{
  const uint8_t* const input = reinterpret_cast<const uint8_t*> (data.data());
  const std::string::size_type len = data.size();

  if (len == 0)
    return std::string();

  std::string ret(((len-1)/3 + 1)*4, '\0');

  std::string::size_type i = 0, j = 0;
  for (i = 0, j = 0; i+3 <= len; i += 3, j += 4)
  {
    // Input: AAAAAAAA BBBBBBBB CCCCCCCC                                       Output:
    ret[j+0] = encodeTable[                     (input[i+0] >> 2)  & mask]; // 00AAAAAA
    ret[j+1] = encodeTable[((input[i+0] << 4) | (input[i+1] >> 4)) & mask]; // 00AABBBB
    ret[j+2] = encodeTable[((input[i+1] << 2) | (input[i+2] >> 6)) & mask]; // 00BBBBCC
    ret[j+3] = encodeTable[  input[i+2]                            & mask]; // 00CCCCCC
  }

  if (i+1 <= len)
  {
    // There are 1 or 2 bytes remaining. This means that we have to insert
    // 1 or 2 padding characters.

    const uint8_t input_1 = (i+2 <= len ? input[i+1] : uint8_t(0));

    // Input: AAAAAAAA BBBBBBBB or AAAAAAAA 00000000                       Output:
    ret[j+0] = encodeTable[                   (input[i] >> 2)  & mask]; // 00AAAAAA
    ret[j+1] = encodeTable[((input[i] << 4) | (input_1  >> 4)) & mask]; // 00AABBBB
    ret[j+2] = encodeTable[ (input_1  << 2)                    & mask]; // 00BBBB00

    if (i+2 > len)
      ret[j+2] = fillchar;
    ret[j+3] = fillchar;
  }

  return ret;
}

std::string
Base64::decode(const std::string& data)
{
  const uint8_t* const input = reinterpret_cast<const uint8_t*> (data.data());
  const std::string::size_type len = data.size();
  const std::string::size_type fullBlockLength = len - 4;

  if (len == 0)
    return std::string();
  if ((len % 4) != 0)
    throw DecodeError();

  std::string ret((len/4)*3, '\0');

  uint8_t decoded[4];
  uint8_t cumulative = 0; // This variable will be or'ed with every decoded input byte.
  std::string::size_type i = 0, j = 0;
  for (i = 0, j = 0; i+4 <= fullBlockLength; i += 4, j += 3)
  {
    for (int k = 0; k < 4; ++k)
    {
      decoded[k] = decodeTable[input[i + k]];
      cumulative |= decoded[k];
    }

    // Input: AAAAAA BBBBBB CCCCCC DDDDDD                      Output:
    ret[j+0] = char((decoded[0] << 2) | (decoded[1] >> 4)); // AAAAAABB
    ret[j+1] = char((decoded[1] << 4) | (decoded[2] >> 2)); // BBBBCCCC
    ret[j+2] = char((decoded[2] << 6) |  decoded[3]);       // CCDDDDDD
  }

  // Handle the last block separately, because it can contain fill characters.
  decoded[0] =                                       decodeTable[input[i+0]];
  decoded[1] =                                       decodeTable[input[i+1]];
  decoded[2] = (data[i+2] == fillchar ? uint8_t(0) : decodeTable[input[i+2]]);
  decoded[3] = (data[i+3] == fillchar ? uint8_t(0) : decodeTable[input[i+3]]);
  cumulative |= (decoded[0] | decoded[1] | decoded[2] | decoded[3]);

  // Input: AAAAAA BBBBBB CCCCCC DDDDDD                      Output:
  ret[j+0] = char((decoded[0] << 2) | (decoded[1] >> 4)); // AAAAAABB
  ret[j+1] = char((decoded[1] << 4) | (decoded[2] >> 2)); // BBBBCCCC
  ret[j+2] = char((decoded[2] << 6) |  decoded[3]);       // CCDDDDDD
  j += 3;

  // Check if any input character was invalid.
  if ((cumulative & ~mask) != 0)
    throw DecodeError();

  // Trim output in case input ended with fill characters.
  std::string::size_type trimCount = 0;
  if (data[i+3] == fillchar)
  {
    if (data[i+2] == fillchar)
      trimCount = 2;
    else
      trimCount = 1;
  }
  else if (data[i+2] == fillchar)
  {
    throw DecodeError();
  }

  // The trimmed characters must already be null.
  for (i = 0; i < trimCount; ++i)
    if (ret[j-i-1] != '\0')
      throw DecodeError();

  ret.resize(j - trimCount);

  return ret;
}
}
