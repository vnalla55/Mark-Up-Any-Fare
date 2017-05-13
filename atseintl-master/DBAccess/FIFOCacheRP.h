#pragma once

#include "Allocator/TrxMalloc.h"
#include "Common/Hasher.h"
#include "Common/KeyedFactory.h"
#include "Common/Thread/TSELockGuards.h"
#include "Common/Thread/TSEReadWriteLock.h"
#include "DBAccess/Cache.h"
#include "DBAccess/DST.h"

#include <deque>
#include <stdexcept>

#include <tr1/unordered_map>
#include <unistd.h>

namespace sfc
{
template <typename Key, typename Type>
class FIFOCache : public Cache<Key, Type>
{
private:
  struct hash_func
  {
    size_t operator()(const Key& key) const
    {
      tse::Hasher hasher(tse::Global::hasherMethod());
      hasher << key;
      return hasher.hash();
    }
  };
  typedef typename Cache<Key, Type>::pointer_type PointerType;
  typedef std::tr1::unordered_map<Key, PointerType, hash_func> Map;
  typedef typename Map::iterator MapIterator;
  Map _map;
  std::deque<Key> _deque;
  TSEReadWriteLock _mutex;
  PointerType _uninitializedCacheEntry;
  size_t _capacity;

private:
  // a helper struct which sets a value in the cache when it is
  // destroyed. It is designed so that we can guarantee that
  // the 'uninitialized' entry will go away even if an exception
  // is generated.
  struct setter
  {
    setter(FIFOCache<Key, Type>& cache, const Key& key) : _cache(cache), _key(key), _valueSet(false)
    {
    }

    ~setter()
    {
      if (!_valueSet)
      {
        TSEWriteGuard<> l(_cache._mutex);
        MapIterator it(_cache._map.find(_key));
        if (it != _cache._map.end())
        {
          if (it->second != _cache._uninitializedCacheEntry)
          {
            _cache.moveToAccumulator(it->second.get());
          }
          _cache._map.erase(it);
        }
      }
    }

    void setValue(const PointerType& value, bool distCacheOp)
    {
      std::vector<Key> oldKeys;
      {
        TSEWriteGuard<> l(_cache._mutex);
        if (_cache._capacity > 0)
        {
          while (_cache._capacity <= _cache._map.size() && !_cache._deque.empty())
          {
            Key oldKey = _cache._deque.front();
            MapIterator it(_cache._map.find(oldKey));
            if (it != _cache._map.end())
            {
              if (it->second != _cache._uninitializedCacheEntry)
              {
                _cache.moveToAccumulator(it->second.get());
                oldKeys.push_back(oldKey);
              }
              _cache._map.erase(it);
            }
            _cache._deque.pop_front();
          }
        }
        _cache._deque.push_back(_key);
        std::pair<MapIterator, bool> pair(_cache._map.insert(std::make_pair(_key, value)));
        if (!pair.second)
        {
          if (pair.first->second != _cache._uninitializedCacheEntry)
          {
            _cache.moveToAccumulator(pair.first->second.get());
          }
          pair.first->second = value;
        }
        _valueSet = true;
      }
      bool ldcOp(true);
      for (typename std::vector<Key>::const_iterator i(oldKeys.begin()); i != oldKeys.end(); ++i)
      {
        _cache.queueDiskInvalidate(*i, false, ldcOp);
      }
      _cache.queueDiskPut(_key, distCacheOp);
    }

  private:
    FIFOCache<Key, Type>& _cache;
    const Key& _key;
    bool _valueSet;
  };

public:
  FIFOCache(KeyedFactory<Key, Type>& factory,
            const std::string& name,
            size_t capacity,
            size_t version)
    : Cache<Key, Type>(factory, "FIFOCache", name, version),
      _uninitializedCacheEntry(new Type),
      _capacity(capacity)
  {
  }

  virtual ~FIFOCache() {}

  size_t size() override
  {
    TSEReadGuard<> l(_mutex);
    return _map.size();
  }

  virtual void reserve(size_t capacity)
  {
    TSEWriteGuard<> l(_mutex);
    _capacity = capacity;
  }

  PointerType getIfResident(const Key& key) override
  {
    TSEReadGuard<> l(_mutex);
    MapIterator j = _map.find(key);
    if (j != _map.end() && j->second != _uninitializedCacheEntry)
    {
      return j->second;
    }
    else
    {
      return PointerType();
    }
  }

  PointerType get(const Key& key) override
  {
    MallocContextDisabler mallocController(false);

    bool dummyExists;

    while (true)
    {
      {
        // normal case, read and return if existing
        TSEReadGuard<> l(_mutex);
        MapIterator j = _map.find(key);
        if (j == _map.end())
          dummyExists = false;
        else
        {
          if (j->second != _uninitializedCacheEntry)
            return j->second;
          dummyExists = true;
        }
      }
      if (dummyExists)
      {
        // it's still being created -- wait
        usleep(10);
        continue;
      }
      {
        // doesn't exist and no one was creating it
        TSEWriteGuard<> l(_mutex);
        MapIterator j = _map.find(key);
        if (j != _map.end())
          // somebody got to it before us
          continue;
        else
        {
          // create a dummy
          mallocController.activate();
          _map[key] = _uninitializedCacheEntry;
        }
      }

      // the item is not present already.
      // unlock the container, while we perform the expensive
      // operation of constructing the object
      setter valueSetter(*this, key);
      bool distCacheOp(false);
      Type* ptr = this->getDistCache(key, distCacheOp);
      if (nullptr == ptr)
      {
        ptr = Cache<Key, Type>::_factory.create(key);
      }
      PointerType ret(ptr);
      valueSetter.setValue(ret, distCacheOp);
      return ret;
    }
  }

  void put(const Key& key, Type* object, bool updateLDC = true) override
  {
    put(key, PointerType(object), updateLDC);
  }

  virtual void put(const Key& key, PointerType object, bool updateLDC = true)
  {
    bool erased(false);
    Key oldKey;
    {
      TSEWriteGuard<> l(_mutex);
      if (_capacity > 0 && _capacity <= _deque.size())
      {
        oldKey = _deque.front();
        MapIterator itEv(_map.find(oldKey));
        if (itEv != _map.end())
        {
          if (itEv->second != _uninitializedCacheEntry)
          {
            this->moveToAccumulator(itEv->second.get());
            erased = true;
          }
          _map.erase(itEv);
        }
        _deque.pop_front();
      }
      _deque.push_back(key);
      std::pair<MapIterator, bool> pair(_map.insert(std::make_pair(key, object)));
      if (!pair.second)
      {
        if (pair.first->second != _uninitializedCacheEntry)
        {
          this->moveToAccumulator(pair.first->second.get());
        }
        pair.first->second = object;
      }
    }
    if (updateLDC)
    {
      bool distCacheOp(true);
      if (erased)
      {
        bool ldcOp(true);
        this->queueDiskInvalidate(oldKey, false, ldcOp);
      }
      this->queueDiskPut(key, distCacheOp);
    }
  }

  size_t invalidate(const Key& key) override
  {
    size_t result(0);
    bool distCacheOp(true), ldcOp(false);
    {
      TSEWriteGuard<> l(_mutex);
      MapIterator it(_map.find(key));
      if (it != _map.end())
      {
        if (it->second != _uninitializedCacheEntry)
        {
          this->moveToAccumulator(it->second.get());
          ldcOp = true;
        }
        _map.erase(it);
        result += 1;
      }
    }
    this->queueDiskInvalidate(key, distCacheOp, ldcOp);
    return result;
  }

  std::shared_ptr<std::vector<Key>> keys() override
  {
    const std::shared_ptr<std::vector<Key>> allKeys(new std::vector<Key>);
    {
      TSEReadGuard<> l(_mutex);
      allKeys->reserve(_map.size());
      for (auto& elem : _map)
      {
        allKeys->push_back(elem.first);
      }
    }
    return allKeys;
  }

  size_t clear() override
  {
    size_t result(0);
    {
      TSEWriteGuard<> l(_mutex);
      result = _map.size();
      for (typename Map::const_iterator it(_map.begin()), itEnd(_map.end()); it != itEnd; ++it)
      {
        if (it->second != _uninitializedCacheEntry)
        {
          this->moveToAccumulator(it->second.get());
        }
      }
      _map.clear();
      _deque.clear();
    }
    this->queueDiskClear();
    return result;
  }

  void emptyTrash() override
  {
    TSEWriteGuard<> l(_mutex);
    if (!this->_accumulator.empty())
    {
      this->_cacheDeleter.moveToTrashBin(this->_accumulator);
    }
  }
}; // class FIFOCache

} // namespace sfc

