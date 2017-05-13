// MirrorCache.h by David White. Copyright Sabre Holdings, 2006.
//
// This file contains a specialized cache implementation which is
// designed to be used in cases where a cache of a relatively small
// amount of data is read from at a high frequency from multiple
// threads, while it is written to very infrequently.
//
// The cache works by keeping a main cache with copies, or 'mirrors'
// of the cache. When an attempt is made to retrieve an item from
// the cache, a mirror is chosen and locked. The mirror is then
// checked to see if it contains the key. If it does contain the key,
// it is returned. Otherwise the master cache is queried, and the
// result will be placed in the mirror.
//
// This greatly reduces contention when reading, since threads will be
// getting data from different mirrors in most cases, and they only
// have to lock the mirror they are getting data from.
//
// Whenever a key in the cache is changed or removed, all the mirrors
// must be locked and the key removed from the mirror. This is therefore
// much slower than changing a normal cache, and so a MirrorCache is
// only efficient in cases where there are many more reads than writes.
//
// The MirrorCache also duplicates data, and so it is only suitable
// when the data set is relatively small (probably up to a few megabytes)
//
// The original motivation for this cache was for the multi airport
// city table, which is relatively small, frequently updated, but
// read from very frequently.

#pragma once

#include "Allocator/TrxMalloc.h"
#include "DBAccess/SimpleCache.h"

#include <map>

#include <pthread.h>

namespace tse
{

template <typename Key, typename Type>
class MirrorCache : public sfc::SimpleCache<Key, Type>
{
  typedef typename sfc::Cache<Key, Type>::pointer_type ValuePtr;
  typedef std::map<Key, ValuePtr> Map;
  typedef typename Map::iterator MapIterator;
  typedef typename Map::const_iterator MapConstIterator;

  struct Mirror
  {
    Map map;
    boost::mutex mutex;
  };

  Mirror* _mirrors;
  size_t _nmirrors;

  class MirrorGetter
  {
  public:
    MirrorGetter(Mirror* mirrors, size_t nmirrors) : _mirror(nullptr)
    {
      const size_t startIndex = pthread_self() % nmirrors;
      for (size_t tries = 0; tries != nmirrors; ++tries)
      {
        size_t index = (startIndex + tries) % nmirrors;
        const bool res = mirrors[index].mutex.try_lock();
        if (res)
        {
          _mirror = &mirrors[index];
          break;
        }
      }
    }

    ~MirrorGetter()
    {
      if (LIKELY(_mirror))
      {
        _mirror->mutex.unlock();
      }
    }

    Mirror* mirror() { return _mirror; }

  private:
    Mirror* _mirror;
  };

  void invalidateFromMirrors(const Key& key)
  {
    for (size_t n = 0; n != _nmirrors; ++n)
    {
      boost::lock_guard<boost::mutex> g(_mirrors[n].mutex);
      _mirrors[n].map.erase(key);
    }
  }

  void clearMirrors()
  {
    for (size_t n = 0; n != _nmirrors; ++n)
    {
      boost::lock_guard<boost::mutex> g(_mirrors[n].mutex);
      _mirrors[n].map.clear();
    }
  }

public:
  MirrorCache(sfc::KeyedFactory<Key, Type>& factory,
              const std::string& name,
              size_t version,
              size_t nmirrors = 11)
    : sfc::SimpleCache<Key, Type>(factory, name, 0, version), _nmirrors(nmirrors > 0 ? nmirrors : 1)
  {
    _mirrors = new Mirror[_nmirrors];
  }

  virtual ~MirrorCache() { delete[] _mirrors; }

  ValuePtr getIfResident(const Key& key) override
  {
    MirrorGetter getter(_mirrors, _nmirrors);
    if (getter.mirror() == nullptr)
    {
      return sfc::SimpleCache<Key, Type>::getIfResident(key);
    }

    Mirror& mirror = *getter.mirror();

    const MapConstIterator mirrorIt = mirror.map.find(key);
    if (mirrorIt != mirror.map.end())
    {
      return mirrorIt->second;
    }

    const MallocContextDisabler disableMalloc;

    ValuePtr value = sfc::SimpleCache<Key, Type>::getIfResident(key);
    if (value)
    {
      mirror.map.insert(std::pair<Key, ValuePtr>(key, value));
    }
    return value;
  }

  ValuePtr get(const Key& key) override
  {
    MirrorGetter getter(_mirrors, _nmirrors);
    if (UNLIKELY(getter.mirror() == nullptr))
    {
      return sfc::SimpleCache<Key, Type>::get(key);
    }

    Mirror& mirror = *getter.mirror();

    const MapConstIterator mirrorIt = mirror.map.find(key);
    if (mirrorIt != mirror.map.end())
    {
      return mirrorIt->second;
    }

    const MallocContextDisabler disableMalloc;

    ValuePtr value = sfc::SimpleCache<Key, Type>::get(key);
    mirror.map.insert(std::pair<Key, ValuePtr>(key, value));
    return value;
  }

  void put(const Key& key, Type* object, bool updateLDC = true) override
  {
    this->put(key, ValuePtr(object), updateLDC);
  }

  void put(const Key& key, ValuePtr object, bool updateLDC = true) override
  {
    sfc::SimpleCache<Key, Type>::put(key, object, updateLDC);
    invalidateFromMirrors(key);
  }

  size_t invalidate(const Key& key) override
  {
    size_t removed(sfc::SimpleCache<Key, Type>::invalidate(key));
    invalidateFromMirrors(key);
    return removed;
  }

  size_t clear() override
  {
    size_t result(sfc::SimpleCache<Key, Type>::clear());
    clearMirrors();
    return result;
  }
};
}

