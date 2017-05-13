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

#include "DBAccess/ChildCache.h"
#include "DBAccess/ChildCacheNotifier.h"
#include "Util/BranchPrediction.h"

#include <boost/function.hpp>
#include <boost/none.hpp>
#include <boost/optional.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/lock_types.hpp>
#include <boost/thread/shared_mutex.hpp>

#include <map>

namespace tse
{

template<typename Key, typename Value>
class ApplicationCache : public ChildCache<Key>
{
  typedef boost::upgrade_mutex Mutex;
  typedef std::shared_ptr<Mutex> SharedMutex;
  typedef boost::shared_lock<Mutex> SharedLock;
  typedef boost::upgrade_lock<Mutex> UpgradeLock;
  typedef boost::unique_lock<Mutex> UniqueLock;
public:
  typedef std::shared_ptr<const Value> SharedConstValue;
  typedef boost::optional<SharedConstValue> OptionalSharedConstValue;
  typedef std::pair<SharedMutex, OptionalSharedConstValue> ValuePair;
  typedef std::map<Key, ValuePair> CacheMap;
  typedef typename CacheMap::iterator CacheMapIterator;
  typedef boost::function<SharedConstValue(const Key&)> UpdateFunction;

  ApplicationCache() = default;
  explicit ApplicationCache(ChildCacheNotifier<Key>& cacheNotifier)
    : _cacheNotifier(&cacheNotifier)
  {
    _cacheNotifier->addListener(*this);
  }

  ~ApplicationCache()
  {
    if (_cacheNotifier)
      _cacheNotifier->removeListener(*this);
  }

  void clear();
  void deleteKey(const Key&);
  SharedConstValue get(const Key&, UpdateFunction);

  // ChildCache notifications
  void keyRemoved(const Key& key) override { deleteKey(key); }
  void cacheCleared() override { clear(); }
  void notifierDestroyed() override;
  bool hasCacheNotifier();
  void setCacheNotifier(ChildCacheNotifier<Key>* cacheNotifier);

private:
  CacheMap _cache;
  ChildCacheNotifier<Key>* _cacheNotifier = nullptr;

  Mutex _cacheMutex;
  Mutex _cacheNotifierMutex;
};

template<typename K, typename V>
void
ApplicationCache<K,V>::clear()
{
  UpgradeLock upgradeLock(_cacheMutex);
  for (typename CacheMap::value_type& cacheElement : _cache)
  {
    UniqueLock keyUniqueLock (*cacheElement.second.first);
    if (cacheElement.second.second)
      cacheElement.second.second.get().reset();
    cacheElement.second.second = boost::none;
  }
}

template<typename K, typename V>
void
ApplicationCache<K, V>::deleteKey(const K& key)
{
  SharedLock sharedLock(_cacheMutex);

  CacheMapIterator it = _cache.find(key);
  if (it != _cache.end())
  {
    UniqueLock keyUniqueLock(*it->second.first);
    if (it->second.second)
      it->second.second.get().reset();
    it->second.second = boost::none;
  }
}

template<typename K, typename V>
typename ApplicationCache<K, V>::SharedConstValue
ApplicationCache<K, V>::get(const K& key, UpdateFunction updateFunction)
{
  SharedLock sharedLock(_cacheMutex);
  CacheMapIterator it = _cache.find(key);
  if (it != _cache.end())
  {
    SharedLock keySharedLock(*it->second.first);
    if (LIKELY(it->second.second))
      return *it->second.second;
  }

  sharedLock.unlock();
  UpgradeLock upgradeLock(_cacheMutex);
  if (it == _cache.end())
    it = _cache.find(key);

  SharedMutex keyMutex(new Mutex());
  ValuePair newValuePair = std::make_pair(keyMutex, SharedConstValue());
  std::pair<CacheMapIterator, bool> insertResult;

  if (it == _cache.end())
  {
    UniqueLock uniqueLock(boost::move(upgradeLock));

    insertResult = _cache.insert(std::make_pair(key, newValuePair));
    it = insertResult.first;
    UniqueLock keyUniqueLock(*it->second.first);

    uniqueLock.unlock();

    *it->second.second = updateFunction(key);
    return *it->second.second;
  }
  UpgradeLock keyUpgradeLock(*it->second.first);

  upgradeLock.unlock();

  if (it->second.second)
    return *it->second.second;

  SharedConstValue newData = updateFunction(key);

  UniqueLock keyUniqueLock(boost::move(keyUpgradeLock));
  it->second.second = newData;

  return *it->second.second;
}

template<typename K, typename V>
void
ApplicationCache<K, V>::notifierDestroyed()
{
  UniqueLock lock(_cacheNotifierMutex);
  _cacheNotifier = nullptr;
}

template<typename K, typename V>
bool
ApplicationCache<K, V>::hasCacheNotifier()
{
  SharedLock lock(_cacheNotifierMutex);
  return _cacheNotifier != nullptr;
}

template<typename K, typename V>
void
ApplicationCache<K, V>::setCacheNotifier(ChildCacheNotifier<K>* cacheNotifier)
{
  UniqueLock lock(_cacheNotifierMutex);

  if (_cacheNotifier)
    _cacheNotifier->removeListener(*this);

  if (cacheNotifier)
    cacheNotifier->addListener(*this);

  _cacheNotifier = cacheNotifier;
}
}
