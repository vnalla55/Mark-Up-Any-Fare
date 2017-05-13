//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
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

#include "Common/Assert.h"
#include "Pricing/Shopping/Utils/StreamFormat.h"

#include <boost/container/flat_set.hpp>

#include <iomanip>
#include <map>
#include <vector>

namespace tse
{

namespace swp
{

// Allows to calculate easily the rank of every stored element
// Rank for element E is the number of elements less than E.
template <typename T>
class RankingSet
{
public:
  typedef boost::container::flat_set<T> SetImpl;

  // Inserts a new element to this set
  void insert(const T& element) { _setImpl.insert(element); }

  // Returns the rank for a given element
  // Raises an exception if the element is not in the set
  unsigned int getRank(const T& element) const
  {
    typename SetImpl::const_iterator f = _setImpl.find(element);
    TSE_ASSERT(f != _setImpl.end());
    return static_cast<unsigned int>(f - _setImpl.begin());
  }

  // Returns the number of elements
  unsigned int getSize() const { return static_cast<unsigned int>(_setImpl.size()); }

  // Returns the begin iterator for stored elements
  typename SetImpl::const_iterator getBegin() const { return _setImpl.begin(); }

  // Returns the end iterator for stored elements
  typename SetImpl::const_iterator getEnd() const { return _setImpl.end(); }

private:
  SetImpl _setImpl;
};

template <typename T>
void
formatRankingSet(std::ostream& out, const RankingSet<T>& s)
{
  out << "RANKING SET with " << s.getSize() << " elements:";

  for (typename RankingSet<T>::SetImpl::const_iterator it = s.getBegin(); it != s.getEnd(); ++it)
  {
    out << " " << *it;
  }
  out << std::endl;
}

template <typename T>
std::ostream& operator<<(std::ostream& out, const RankingSet<T>& s)
{
  out << utils::format(s, formatRankingSet<T>);
  return out;
}

} // namespace swp

} // namespace tse

