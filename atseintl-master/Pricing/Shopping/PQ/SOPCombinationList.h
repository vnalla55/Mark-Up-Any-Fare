// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
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

#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/PQ/SOPCombination.h"

#include <boost/intrusive/list.hpp>

#include <deque>
#include <vector>

namespace tse
{
namespace shpq
{

class SOPCombinationList
{
  friend class SOPCombinationListTest;

  typedef std::deque<SOPCombination> CombinationsStorage;
  typedef boost::intrusive::list<SOPCombination> Combinations;

  typedef std::deque<SOPCombination*> CombinationsCache;

  typedef std::vector<SOPCombination*> CombinationsForSop;

public:
  typedef Combinations::iterator iterator;
  typedef Combinations::const_iterator const_iterator;

  struct DiagDelegate
  {
    virtual void onErase(const SOPCombination& comb) = 0;
    virtual ~DiagDelegate() {}
  };

  explicit SOPCombinationList(const ShoppingTrx& trx) : _diagDelegate(nullptr)
  {
    _combinationsForSop.resize(trx.legs().size());
    for (unsigned legId = 0; legId < trx.legs().size(); ++legId)
    {
      _combinationsForSop[legId].resize(trx.legs()[legId].sop().size());
    }
  }

  void push_back(const SOPCombination& combinationToAdd)
  {
    _combinationsStorage.push_back(combinationToAdd);

    SOPCombination& combination = _combinationsStorage.back();
    _combinations.push_back(combination);
    combination._isLinked = true;
    for (unsigned legId = 0; legId < _combinationsForSop.size(); ++legId)
    {
      const int sopId = combination.oSopVec[legId];
      combination._next[legId] = _combinationsForSop[legId][sopId];
      _combinationsForSop[legId][sopId] = &combination;
    }
  }

  bool empty() const { return _combinations.empty(); }
  std::size_t size() const { return _combinations.size(); }
  iterator begin() { return _combinations.begin(); }
  iterator end() { return _combinations.end(); }
  const_iterator begin() const { return _combinations.begin(); }
  const_iterator end() const { return _combinations.end(); }

  iterator erase(iterator position)
  {
    SOPCombination& combination = *position;
    if (UNLIKELY(_diagDelegate))
    {
      _diagDelegate->onErase(combination);
    }

    combination._isLinked = false;
    return _combinations.erase(position);
  }

  template <class Predicate>
  void remove_if(const Predicate& predicate)
  {
    for (typename Combinations::iterator it = _combinations.begin(); it != _combinations.end();
         /*empty*/)
    {
      const SOPCombination& combination = *it;
      if (predicate(combination))
      {
        it = erase(it);
      }
      else
      {
        ++it;
      }
    }
  }

  void removeCombinations(unsigned legId, int sopId)
  {
    SOPCombination* combination = _combinationsForSop[legId][sopId];
    _combinationsForSop[legId][sopId] = nullptr;
    for (; combination; combination = combination->_next[legId])
    {
      if (combination->_isLinked)
      {
        erase(_combinations.iterator_to(*combination));
      }
    }
  }

  // install delegate
  void swapDelegate(DiagDelegate*& delegate) { std::swap(_diagDelegate, delegate); }

  void addCombinationToCache(SOPCombination& combination)
  {
    _combinationsCache.push_back(&combination);
  }

  void clearCombinationsCache() { _combinationsCache.clear(); }

  iterator takeCombinationFromCache()
  {
    while (!_combinationsCache.empty())
    {
      SOPCombination& combination = *_combinationsCache.front();
      _combinationsCache.pop_front();
      if (combination._isLinked)
      {
        return _combinations.iterator_to(combination);
      }
    }

    return _combinations.end();
  }

private:
  SOPCombinationList(const SOPCombinationList&);
  void operator=(const SOPCombinationList&);

private:
  CombinationsStorage _combinationsStorage;
  Combinations _combinations;
  CombinationsCache _combinationsCache;
  std::vector<CombinationsForSop> _combinationsForSop;
  DiagDelegate* _diagDelegate;
};

} // namespace shpq
} // namespace tse

