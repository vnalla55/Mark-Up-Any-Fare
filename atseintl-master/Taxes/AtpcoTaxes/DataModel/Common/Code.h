// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include <cstring>
#include <string>
#include <boost/core/enable_if.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/conditional.hpp>
#include <boost/type_traits/integral_constant.hpp>
#include "Util/BranchPrediction.h"

namespace tax
{

// A meta-function for computing the shortest integer type capable of storing N bytes.
// It can be summarized as:
//
//  if (N < sizeof(int8_t))
//    return int8_t;
//  else if (N < sizeof(int16_t))
//    return int16_t
//  else if (N < sizeof(int32_t))
//    return int32_t;
//  else
//    return int64_t
template <int N>
struct ShortestFittingInt
{
  typedef
    typename boost::conditional<
      (N <= sizeof(int8_t)),
      int8_t,
      typename boost::conditional<
        (N <= sizeof(int16_t)),
        int16_t,
        typename boost::conditional<
          (N <= sizeof(int32_t)),
          int32_t,
          int64_t
        >::type
      >::type
    >::type type;

  BOOST_STATIC_ASSERT(N > 0);
  BOOST_STATIC_ASSERT(N <= sizeof(int64_t));
};

// A tag that signals that you want to create a nul-code
enum UninitializedCodeT { UninitializedCode };

// A combined tag that says our Code will be implicitly convertible from either of the operands
template <typename T1, typename T2>
struct CodeMultiTag
{
  typedef T1 type1;
  typedef T2 type2;
};

// A metafunction that says if ToT is a multi-tag with fromT as one of its arguments
template <typename ToT, typename FromT>
  struct CodeTagAccepts
  : boost::false_type {};

template <typename T1, typename T2>
  struct CodeTagAccepts<CodeMultiTag<T1, T2>, typename CodeMultiTag<T1, T2>::type1>
  : boost::true_type {};
  
template <typename T1, typename T2>
  struct CodeTagAccepts<CodeMultiTag<T1, T2>, typename CodeMultiTag<T1, T2>::type2>
  : boost::true_type {};

template <typename tag, int L, int H = L>
class Code
{
  BOOST_STATIC_ASSERT (L <= H);

  typedef typename ShortestFittingInt<H>::type int_t;
  union {
    char array[H];
    int_t as_int;
  };

public:
  explicit Code(UninitializedCodeT) : as_int() {}
  explicit Code() : as_int() {}

  template <int M>
  Code(const char (&literal)[M]) : as_int()
  {
    BOOST_STATIC_ASSERT (M >= L + 1);
    BOOST_STATIC_ASSERT (M <= H + 1);
    std::memcpy(array, literal, M - 1);
  }

  template <typename tag2, int L2, int H2>
    friend class Code;
  
  template <typename tag2>
  Code(Code<tag2, L, H> rhs, typename boost::enable_if<CodeTagAccepts<tag, tag2>, int>::type = 0)
    : as_int(rhs.as_int)
  {
  }

  template <typename tag2>
  void convertFrom(Code<tag2, L, H> rhs)
  {
    this->as_int = rhs.as_int;
  }

  template <int M>
  Code& operator=(const char (&literal)[M])
  {
    BOOST_STATIC_ASSERT (M >= L + 1);
    BOOST_STATIC_ASSERT (M <= H + 1);
    as_int = int_t();
    std::memcpy(array, literal, M - 1);
    return *this;
  }

  template <typename tag2>
  typename boost::enable_if<CodeTagAccepts<tag, tag2>, Code&>::type
  operator=(Code<tag2, L, H> rhs)
  {
    this->as_int = rhs.as_int;
    return *this;
  }


  std::string asString() const
  {
    if (BOOST_LIKELY(!empty()))
      return std::string(array, length());
    else
      return std::string();
  }

  int length() const
  {
    int ans = L;
    for (; ans != H; ++ans)
      if (array[ans] == 0)
        break;
    return ans;
  }

  bool empty() const
  {
    return as_int == int_t();
  }

  friend bool operator==(const Code& lhs, const Code& rhs)
  {
    return lhs.as_int == rhs.as_int;
  }

  friend bool operator!=(const Code& lhs, const Code& rhs)
  {
    return !(lhs == rhs);
  }

  friend bool operator<(const Code& lhs, const Code& rhs)
  {
    return std::memcmp(lhs.array, rhs.array, H) < 0;
  }

  int compare(const Code& rhs) const
  {
    return std::memcmp(array, rhs.array, H);
  }

  friend bool operator==(const Code& lhs, UninitializedCodeT) { return lhs.empty(); }
  friend bool operator==(UninitializedCodeT, const Code& rhs ) { return rhs.empty(); }
  friend bool operator!=(const Code& lhs, UninitializedCodeT) { return !lhs.empty(); }
  friend bool operator!=(UninitializedCodeT, const Code& rhs ) { return !rhs.empty(); }

  bool fromString(const char* str, size_t len)
  {
    if (BOOST_LIKELY(len >= L && len <= H)) {
      as_int = int_t();
      std::memcpy(array, str, len);
      return true;
    }
    else {
      return false;
    }
  }

  friend bool codeFromString(const std::string& str, Code& code)
  {
    return code.fromString(str.c_str(), str.length());
  }

  friend bool codeFromString(const char* str, Code& code)
  {
    return code.fromString(str, std::strlen(str));
  }

  typedef const char (array_type) [H];

  array_type& rawArray() const
  {
    return array;
  }

  template <int M>
  bool operator==(const char(&str)[M]) const
  {
    BOOST_STATIC_ASSERT (M >= L + 1);
    BOOST_STATIC_ASSERT (M <= H + 1);
    return std::memcmp(array, str, M - 1) == 0 && (M - 1 == H || array[M - 1] == 0);
  }

  template <int M>
  bool operator!=(const char(&str)[M]) const { return !(*this == str); }
};

} // namespace tax

