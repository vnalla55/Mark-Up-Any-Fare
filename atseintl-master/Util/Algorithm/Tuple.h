//-------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include <tuple>
#include <type_traits>

namespace tse
{
namespace alg
{
namespace helper
{
template <size_t Index>
using Index_t = std::integral_constant<size_t, Index>;

template <class TupleRef>
constexpr size_t tuple_size_v = std::tuple_size<std::remove_reference_t<TupleRef>>::value;

template <class Tuple, class Functor>
void
enumerate_tuple_impl(Tuple&&, Functor, Index_t<tuple_size_v<Tuple>>)
{}

template <class Tuple, class Functor, size_t Index>
std::enable_if_t<Index != tuple_size_v<Tuple>>
enumerate_tuple_impl(Tuple&& t, Functor f, Index_t<Index>)
{
  // No std::forward<>(t) - proces rvalue-ref like lvalue-ref as we refer to the tuple many times
  f(Index_t<Index>(), std::get<Index>(t));
  enumerate_tuple_impl(t, std::move(f), Index_t<Index + 1>());
}
} // ns helper

// =================================================================================================
//
// The functions below are tools that allow one to apply specified functor to each element of
// a tuple. Note that the functor should implement operator() for each type used in the tuple.
// Of course, one can take advantage of generalized lambdas as well.
//
// Example:
//     std::tuple<int, const char*> t(1, "ASDF");
//
//     alg::enumerate_tuple(t, [](const auto i, const auto& item)
//     {
//       std::cout << i() << ": " << item << std::endl;
//     });
//
//     alg::visit_tuple(t, [](const auto& item) { std::cout << item << std::endl; });
//
// =================================================================================================
template <class Tuple, class Functor>
inline void
enumerate_tuple(Tuple&& t, Functor f)
{
  // No std::forward<>(t) - proces rvalue-ref like lvalue-ref as we refer to the tuple many times
  helper::enumerate_tuple_impl(t, std::move(f), helper::Index_t<0>());
}

template <class Tuple, class Functor>
inline void
visit_tuple(Tuple&& t, Functor f)
{
  // No std::forward<>(t) - proces rvalue-ref like lvalue-ref as we refer to the tuple many times
  enumerate_tuple(t, [movedF = std::move(f)](auto, auto&& val)
  {
    movedF(val);
  });
}
} // ns alg
}
