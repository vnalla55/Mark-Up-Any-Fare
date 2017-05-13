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

#include <bitset>

namespace tse
{
namespace alg
{

// Given a range: [first, last) s.t. 0 <= first <= last <= 64
// set or clear first through last bit in the bitset.

template <size_t N>
inline void
fill_bitset(std::bitset<N>& bset, size_t first, size_t last, bool val = true)
{
  if (first == last)
    return;

  // Shift by >= 64 bits is undefined so we need to handle this case separately.
  const uint64_t mask = ((last != 64) ? 1ULL << last : 0) - (1ULL << first);

  if (val)
    bset |= std::bitset<N>(mask);
  else
    bset &= ~std::bitset<N>(mask);
}

template <size_t N>
inline std::bitset<N>
create_bitset(size_t first, size_t last)
{
  std::bitset<N> bset;
  fill_bitset(bset, first, last);
  return bset;
}

}
}
