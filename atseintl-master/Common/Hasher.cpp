//----------------------------------------------------------------------------
//  Copyright (c) Sabre 2013
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//
//----------------------------------------------------------------------------
#include "Common/Hasher.h"

namespace tse
{

void
Hasher::rehash(uint8_t c)
{
  switch (_method)
  {
  case HasherMethod::METHOD_0:
  {
    uint32_t p;
    _h = (_h << 4) + c;
    if ((p = _h & 0xf0000000) != 0)
    {
      _h ^= p >> 24;
      _h ^= p;
    }
    break;
  }

  case HasherMethod::METHOD_1:
  {
    _h += c;
    _h += (_h << 10);
    _h ^= (_h >> 6);
    break;
  }

  case HasherMethod::METHOD_2:
  {
    _h *= 0x01000193;
    _h ^= (uint32_t)c;
    break;
  }

  case HasherMethod::METHOD_3:
  {
    const uint8_t ch = (std::toupper(c) - 0x1f) & 0x3f;
    uint32_t p = _h & 0xffffffe0;
    _h = (_h << 5) + ch;
    _h ^= p;
    break;
  }

  case HasherMethod::METHOD_4:
  {
    uint32_t p;
    const uint8_t ch = (std::toupper(c) - 0x1f) & 0x3f;
    p = _h & 0xf8000000;
    _h = (_h << 5) + ch;
    if (p != 0)
    {
      _h ^= p >> 22;
    }
    break;
  }

  case HasherMethod::METHOD_5:
  {
    uint32_t p;
    const uint8_t ch = (std::toupper(c) - 0x1f) & 0x3f;
    p = _h & 0xe0000000;
    _h = (_h << 3) + ch;
    if (p != 0)
    {
      _h ^= p >> 26;
    }
    break;
  }

  case HasherMethod::METHOD_6:
  {
    uint32_t p;
    const uint8_t ch = (std::toupper(c) - 0x20) & 0x3f;
    p = _h & 0xf0000000;
    _h = (_h << 4) + ch;
    if (p != 0)
    {
      _h ^= p >> 24;
    }
    break;
  }

  case HasherMethod::METHOD_7:
  {
    uint32_t p;
    _h = (_h << 4) + c;
    if ((p = _h & 0xf0000000) != 0)
    {
      _h ^= p >> 24;
      _h ^= p;
    }
    break;
  }
  }
}

void
Hasher::rehash(const void* p, std::size_t n)
{
  const char* s = (const char*)p;
  for (std::size_t i = 0; i < n; i++)
  {
    rehash(*s++);
  }
}

void
Hasher::superFastHash(const char* data, std::size_t len)
{

#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) || defined(_MSC_VER) ||       \
    defined(__BORLANDC__) || defined(__TURBOC__)
#define get16bits(d) (*((const uint16_t*)(d)))
#endif

#if !defined(get16bits)
#define get16bits(d)                                                                               \
  ((((uint32_t)(((const uint8_t*)(d))[1])) << 8) + (uint32_t)(((const uint8_t*)(d))[0]))
#endif

  uint32_t tmp;
  int rem;

  rem = len & 3;
  len >>= 2;

  /* Main loop */
  for (; len > 0; len--)
  {
    _h += get16bits(data);
    tmp = (get16bits(data + 2) << 11) ^ _h;
    _h = (_h << 16) ^ tmp;
    data += 2 * sizeof(uint16_t);
    _h += _h >> 11;
  }

  /* Handle end cases */
  switch (rem)
  {
  case 3:
    _h += get16bits(data);
    _h ^= _h << 16;
    _h ^= data[sizeof(uint16_t)] << 18;
    _h += _h >> 11;
    break;
  case 2:
    _h += get16bits(data);
    _h ^= _h << 11;
    _h += _h >> 17;
    break;
  case 1:
    _h += *data;
    _h ^= _h << 10;
    _h += _h >> 1;
  }

  /* Force "avalanching" of final 127 bits */
  _h ^= _h << 3;
  _h += _h >> 5;
  _h ^= _h << 4;
  _h += _h >> 17;
  _h ^= _h << 25;
  _h += _h >> 6;

#undef get16bits
}

Hasher& operator<<(Hasher& hasher, const char* s)
{
  if (hasher._method == HasherMethod::METHOD_7)
  {
    hasher.superFastHash(s, strlen(s));
  }
  else
  {
    while (*s != 0)
    {
      hasher.rehash(*s++);
    }
  }
  return hasher;
}

Hasher& operator<<(Hasher& hasher, const std::string& s)
{
  const char* p = s.c_str();
  if (hasher._method == HasherMethod::METHOD_7)
  {
    hasher.superFastHash(p, s.size());
  }
  else
  {
    while (*p != 0)
    {
      hasher.rehash(*p++);
    }
  }
  return hasher;
}

Hasher& operator<<(Hasher& hasher, const boost::container::string& s)
{
  const char* p = s.c_str();
  if (hasher._method == HasherMethod::METHOD_7)
  {
    hasher.superFastHash(p, s.size());
  }
  else
  {
    while (*p != 0)
    {
      hasher.rehash(*p++);
    }
  }
  return hasher;
}

} // namespace tse
