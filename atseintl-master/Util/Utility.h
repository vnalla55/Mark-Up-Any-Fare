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

#pragma once

#include "Util/BranchPrediction.h"

#include <algorithm>
#include <utility>

#include <boost/type_traits.hpp>

namespace tse
{
using std::swap;

// Use it if you want to perform ADL swap call, but have already a method called swap.
template <typename T> inline
void adlSwap(T& l, T& r)
{
  swap(l, r);
}

template <typename T>
struct IsPod : boost::is_pod<T> {};

template <typename Left, typename Right>
struct IsPod<std::pair<Left, Right> >
    : boost::integral_constant<bool, boost::is_pod<Left>::value && boost::is_pod<Right>::value> {};

template <int...>
struct Sequence {};

template <int N, int... S>
struct GenerateSequence : GenerateSequence<N - 1, N - 1, S...> {};

template <int... S>
struct GenerateSequence<0, S...>
{
  typedef Sequence<S...> type;
};

template <int N>
inline typename GenerateSequence<N>::type
makeSequence() { return {}; }

}

