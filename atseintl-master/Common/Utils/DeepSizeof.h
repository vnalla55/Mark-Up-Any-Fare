//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include <cstddef>
#include <type_traits>
#include <vector>

namespace tse
{

namespace tools
{


template <class T>
struct DeepSizeofAsClass
{
  size_t operator()(const T& t) const
  {
    return deep_sizeof_impl(t);
  }
};

template <class T>
struct DeepSizeofAsArithmetic
{
  size_t operator()(const T& t) const
  {
    return sizeof(t);
  }
};

template <class T>
struct DeepSizeof : std::conditional<std::is_arithmetic<T>::value,
                                     DeepSizeofAsArithmetic<T>,
                                     DeepSizeofAsClass<T>>::type {};

template <class T>
size_t deep_sizeof(const T& t)
{
  return DeepSizeof<T>{}(t);
}


template <class T>
struct DeepSizeof<T*>
{
  size_t operator()(T* p) const
  {
    if (p == nullptr)
    {
      return sizeof(p);
    }
    return sizeof(p) + deep_sizeof(*p);
  }
};


template <class T>
struct DeepSizeof<std::vector<T>>
{
  size_t operator()(const std::vector<T>& v) const
  {
    size_t sz = sizeof(v);
    for (const auto& elem: v)
    {
      sz += deep_sizeof(elem);
    }
    return sz;
  }
};


} // namespace tools

} // namespace tse

