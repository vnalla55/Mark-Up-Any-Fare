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

#include <boost/functional/hash.hpp>

namespace tse
{

namespace tools
{

template<class T>
size_t calc_hash(const T& t)
{
  return boost::hash<T>()(t);
}

template <class T>
T* new_reserved(size_t sz)
{
  T* coll = new T;
  coll->reserve(sz);
  return coll;
}

template <typename T> T& non_const(const T& v)
{
  return const_cast<T&>(v);
}

} // namespace tools

} // namespace tse

