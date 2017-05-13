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

#include "Common/Utils/CommonUtils.h"

#include <unordered_map>

namespace tse
{

namespace tools
{

template<class Ptr>
struct PointerDerefHash
{
  size_t operator()(const Ptr& ptr) const noexcept
  {
    return calc_hash(*ptr);
  }
};

template<class Ptr>
struct PointerDerefEq
{
  bool operator()(const Ptr& a, const Ptr& b) const noexcept
  {
    return (*a) == (*b);
  }
};

// Stores pointers but uses dereferenced values for equality check and hash
// calculation.
template<class Ptr, class T> using PtrHashMap =
    std::unordered_map<Ptr, T, PointerDerefHash<Ptr>, PointerDerefEq<Ptr>>;

} // namespace tools

} // namespace tse

