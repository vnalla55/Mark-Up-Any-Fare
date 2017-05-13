// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#pragma once

#include <boost/function.hpp>

#include <memory>

namespace tse
{
template <typename Key, typename Value>
class TransactionCache
{
public:
  typedef std::shared_ptr<const Value> SharedConstValue;
  typedef std::map<Key, SharedConstValue> CacheMap;
  typedef typename CacheMap::const_iterator CacheMapConstIterator;
  typedef boost::function<SharedConstValue(const Key&)> ReadFunctor;

  TransactionCache(ReadFunctor functor) : _functor(functor) {}

  SharedConstValue get(const Key&);

private:
  ReadFunctor _functor;
  CacheMap _cache;
};

template<typename K, typename V>
typename TransactionCache<K, V>::SharedConstValue
TransactionCache<K, V>::get(const K& key)
{
  CacheMapConstIterator it = _cache.find(key);
  if (it != _cache.end())
    return it->second;

  std::pair<CacheMapConstIterator, bool> insertResult =
      _cache.insert(std::make_pair(key, _functor(key)));

  return insertResult.first->second;
}

}
