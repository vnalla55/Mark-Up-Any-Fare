//-------------------------------------------------------------------
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

namespace tse
{
namespace alg
{
namespace impl
{
template <class T, class GetterRetType>
using GetterType = GetterRetType (T::*)() const;
}

// members_equal() simplifies object's comparison by members.
// NOTE: Currently it handles only getters but it may change in the future
//
// Example:
//    struct S {
//      int first_getter() { ... }
//      char second_getter() { .. }
//    };
//
//    bool areEqual(const S& o1, const S& o2) {
//      return members_equal(o1, o2, &S::first_getter, &S::second_getter);
//    };
template <class T, class... GetterRetTypes>
inline bool
members_equal(const T& o1, const T& o2)
{
  return true;
}

template <class T, class GT, class... GetterRetTypes>
inline bool
members_equal(const T& o1,
              const T& o2,
              impl::GetterType<T, GT> getter,
              impl::GetterType<T, GetterRetTypes>... getters)
{
  return (o1.*getter)() == (o2.*getter)() && members_equal(o1, o2, getters...);
}
}
}
