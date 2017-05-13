//-------------------------------------------------------------------
//
//  Copyright Sabre 2012
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#pragma once

#include <algorithm>

namespace tse
{

template <typename I, typename O, typename P>
inline void
copyIteratorIf(I begin, I end, O out, P pred)
{
  for (; begin != end; ++begin)
    if (pred(*begin))
      ++out = begin;
}

template <typename I, typename O, typename P>
inline void
copyIf(I begin, I end, O out, P pred)
{
  for (; begin != end; ++begin)
    if (pred(*begin))
      *(++out) = *begin;
}

template <typename I, typename P>
inline I
stablePartition(I begin, I end, P pred)
{
  while (true)
  {
    begin = std::find_if(begin, end, std::not1(pred));
    if (begin == end)
      return end;

    I current = std::find_if(begin, end, pred);
    if (current == end)
      return begin;

    typename std::iterator_traits<I>::value_type tmp = *current;
    std::copy_backward(begin, current, current + 1);
    *begin++ = tmp;
  }
}

template <typename P, typename I, typename C, unsigned size>
inline bool
sequentialSelect(I begin, I end, const double amt, const double (&sequence)[size], C& result)
{
  for (const auto elem : sequence)
  {
    I mid = stablePartition(begin, end, P(amt, elem));

    if (begin != mid)
    {
      result.assign(begin, mid);
      return true;
    }
  }
  return false;
}

} // tse

