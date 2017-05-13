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

#include <functional>
#include <iterator>

namespace tse
{
namespace alg
{

// The function below is almost like std::find_if except that it should return
// iterator pointing to last element that satisfies the predicate.

template <class Iter, class Pred>
Iter find_last_if(Iter first, Iter last, Pred pred)
{
  typedef std::reverse_iterator<Iter> RevIter;

  const RevIter rend = RevIter(first);
  const RevIter rit = std::find_if(RevIter(last), rend, std::move(pred));

  return (rit != rend) ? rit.base() - 1 : last;
}

// Example:
//    const auto v = {1, 2, 2, 3, 2};
//    find_equal_range_around(v.begin(), v.end(), v.begin() + 1, std::equal_to<int>())
//
// Returns:
//    Pair of iterators that form the subrange: {2, 2}.

template <class Iter, class EqualPred>
std::pair<Iter, Iter> find_equal_range_around(Iter first, Iter last, Iter it, EqualPred pred)
{
  const auto notEqualToIt = [&](typename std::iterator_traits<Iter>::reference v)
  {
    return !pred(v, *it);
  };

  const auto lastNotEqual = alg::find_last_if(first, it, notEqualToIt);
  const auto subrangeFirst = (lastNotEqual == it) ? first : (lastNotEqual + 1);
  const auto subrangeLast = std::find_if(it + 1, last, notEqualToIt);
  return std::make_pair(subrangeFirst, subrangeLast);
}

}
}
