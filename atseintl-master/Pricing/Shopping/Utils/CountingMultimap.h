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

#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>

namespace tse
{

namespace utils
{

class CountLimitPredicate
{
public:
  CountLimitPredicate(unsigned int limit)
  {
    TSE_ASSERT(limit > 0);
    _limit = limit;
  }

  bool operator()(unsigned int newCount, bool increased)
  {
    if (increased)
    {
      if (newCount > _limit)
      {
        return true;
      }
      return false;
    }

    if (newCount >= _limit)
    {
      return true;
    }
    return false;
  }

  unsigned int getLimit() const { return _limit; }

private:
  unsigned int _limit;
};

template <typename V>
class ICountingEventReceiver
{
public:
  virtual void eventForValue(const V& v, bool increased) = 0;
  virtual ~ICountingEventReceiver() {}
};

template <typename K, typename V, typename CountingEventPredicate>
class CountingMultimap
{
public:
  typedef ICountingEventReceiver<V> ImplICountingEventReceiver;
  typedef boost::unordered_set<V> ValueSet;

  CountingMultimap(CountingEventPredicate& predicate) : _receiver(nullptr), _predicate(predicate) {}

  void setReceiver(ImplICountingEventReceiver* receiver)
  {
    TSE_ASSERT(receiver != nullptr);
    _receiver = receiver;
  }

  void add(const K& k, const V& v)
  {
    ValueSet& vv = _multimap[k];
    // prevent (k, v) duplicates
    TSE_ASSERT(vv.find(v) == vv.end());
    vv.insert(v);
    if (_predicate(static_cast<unsigned int>(vv.size()), true))
    {
      TSE_ASSERT(_receiver != nullptr);
      for (const auto& elem : vv)
      {
        _receiver->eventForValue(elem, true);
      }
    }
  }

  void remove(const K& k, const V& v)
  {
    ValueSet& vv = _multimap[k];
    TSE_ASSERT(!vv.empty());
    TSE_ASSERT(vv.find(v) != vv.end());

    if (_predicate(static_cast<unsigned int>(vv.size() - 1), false))
    {
      TSE_ASSERT(_receiver != nullptr);
      for (const auto& elem : vv)
      {
        _receiver->eventForValue(elem, false);
      }
    }

    vv.erase(v);
    // Remove dangling
    if (vv.empty())
    {
      _multimap.erase(k);
    }
  }

  const ValueSet& getValues(const K& k) const
  {
    typename Multimap::const_iterator it = _multimap.find(k);
    TSE_ASSERT(it != _multimap.end());
    return it->second;
  }

private:
  typedef boost::unordered_map<K, ValueSet> Multimap;

  ImplICountingEventReceiver* _receiver;
  Multimap _multimap;
  CountingEventPredicate& _predicate;
};

} // namespace utils

} // namespace tse

