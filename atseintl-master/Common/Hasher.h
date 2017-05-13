//----------------------------------------------------------------------------
//
//  File:    Hasher.h
//
//  Description:
//     Helper class for calculating hash values.
//     Based on the hashpjw algorithm by PJ Weinberger
//     (see the dragon book for more details).
//
//  Copyright (c) Sabre 2004
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

#pragma once

#include "Common/TseEnums.h"

#include <boost/container/string.hpp>
#include "Util/BranchPrediction.h"

#include <cctype>
#include <cstdint>
#include <cstring>
#include <string>

namespace tse
{

class Hasher
{
private:
  uint32_t _h = 0;
  const HasherMethod _method;

  void rehash(uint8_t c);

  void rehash(const void* p, std::size_t n);

  void superFastHash(const char* data, std::size_t len);

public:
  Hasher(const HasherMethod method) noexcept : _method(method) {}

  uint32_t hash() noexcept { return _h; }

  friend Hasher& operator<<(Hasher& hasher, const uint8_t i)
  {
    hasher.rehash(i);
    return hasher;
  }

  friend Hasher& operator<<(Hasher& hasher, const uint16_t i)
  {
    hasher.rehash(&i, 2);
    return hasher;
  }

  friend Hasher& operator<<(Hasher& hasher, const uint32_t i)
  {
    hasher.rehash(&i, 4);
    return hasher;
  }

  friend Hasher& operator<<(Hasher& hasher, const uint64_t i)
  {
    hasher.rehash(&i, 8);
    return hasher;
  }

  friend Hasher& operator<<(Hasher& hasher, const int8_t i)
  {
    hasher.rehash(&i, 1);
    return hasher;
  }

  friend Hasher& operator<<(Hasher& hasher, const int16_t i)
  {
    hasher.rehash(&i, 2);
    return hasher;
  }

  friend Hasher& operator<<(Hasher& hasher, const int32_t i)
  {
    hasher.rehash(&i, 4);
    return hasher;
  }

  friend Hasher& operator<<(Hasher& hasher, const int64_t i)
  {
    hasher.rehash(&i, 8);
    return hasher;
  }

  friend Hasher& operator<<(Hasher& hasher, const long long i)
  {
    hasher.rehash(&i, 8);
    return hasher;
  }
  
  friend Hasher& operator<<(Hasher& hasher, const char* s);

  friend Hasher& operator<<(Hasher& hasher, const std::string& s);

  friend Hasher& operator<<(Hasher& hasher, const boost::container::string& s);
};

} // namespace tse

