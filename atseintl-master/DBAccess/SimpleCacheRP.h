#pragma once

#include "Allocator/TrxMalloc.h"
#include "Common/Hasher.h"
#include "Common/KeyedFactory.h"
#include "Common/Thread/TSELockGuards.h"
#include "Common/Thread/TSEReadWriteLock.h"
#include "DBAccess/Cache.h"
#include "DBAccess/DST.h"

#include <memory>
#include <list>
#include <stdexcept>

#include <tr1/unordered_map>
#include <unistd.h>

namespace sfc
{
template <typename Key, typename Type>
class SimpleCache : public Cache<Key, Type>
{
private:
  struct hash_func
  {
    size_t operator()(const Key& key) const
    {
      size_t hash(0);
      tse::hashCombine(hash, key);
      return hash;
    }
  };

  typedef typename Cache<Key, Type>::pointer_type _pointer_type;
  typedef std::tr1::unordered_map<Key, _pointer_type, hash_func> Map;
  typedef typename Map::iterator MapIterator;
  Map _map;
  TSEReadWriteLock _mutex;
  _pointer_type _uninitializedCacheEntry;
  size_t _capacity;

private:
  // a helper struct which sets a value in the cache when it is
  // destroyed. It is designed so that we can guarantee that
  // the 'uninitialized' entry will go away even if an exception
  // is generated.
  struct setter
  {
    setter(SimpleCache<Key, Type>& cache, const Key& key)
      : _cache(cache), _key(key), _valueSet(false)
    {
    }

    ~setter()
    {
      if (UNLIKELY(!_valueSet))
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

    void setValue(const _pointer_type& value, bool distCacheOp)
    {
      {
        TSEWriteGuard<> l(_cache._mutex);
        std::pair<MapIterator, bool> pair(_cache._map.insert(std::make_pair(_key, value)));
        if (LIKELY(!pair.second))
        {
          if (UNLIKELY(pair.first->second != _cache._uninitializedCacheEntry))
          {
            _cache.moveToAccumulator(pair.first->second.get());
          }
          pair.first->second = value;
        }
      }
      _cache.queueDiskPut(_key, distCacheOp);
      _valueSet = true;
    }

  private:
    SimpleCache<Key, Type>& _cache;
    const Key& _key;
    bool _valueSet;
  };

public:
  SimpleCache(KeyedFactory<Key, Type>& factory,
              const std::string& name,
              size_t capacity,
              size_t version)
    : Cache<Key, Type>(factory, "SimpleCache", name, version),
      _uninitializedCacheEntry(new Type),
      _capacity(capacity)
  {
  }

  virtual ~SimpleCache() {}

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

  _pointer_type getIfResident(const Key& key) override
  {
    TSEReadGuard<> l(_mutex);
    typename Map::const_iterator j(_map.find(key));
    if (j != _map.end() && j->second != _uninitializedCacheEntry)
    {
      return j->second;
    }
    else
    {
      return _pointer_type();
    }
  }

  _pointer_type get(const Key& key) override
  {
    MallocContextDisabler mallocController(false);

    // see if the item is in the cache already, locking the container
    // while we perform the lookup
    {
      TSEReadGuard<> l(_mutex);
      typename Map::const_iterator j = _map.find(key);
      if (j != _map.end())
      {
        bool usePreExisting = true;

        while (j != _map.end() && j->second == _uninitializedCacheEntry && usePreExisting)
        {
          l.release();
          usleep(10);
          l.acquire();
          j = _map.find(key);
          if (UNLIKELY(j == _map.end()))
          {
            mallocController.activate();
            usePreExisting = false;
          }
        }
        // the item is present, return it
        if (LIKELY(usePreExisting))
          return j->second;
      }
      else
      {
        mallocController.activate();
      }
    }

    {
      // TSEWriteGuard<> l(_mutex);
      _mutex.acquireWrite();
      if (_map.count(key) != 0)
      {
        _mutex.releaseWrite();
        return get(key);
      }
      std::pair<MapIterator, bool> pair(_map.insert(std::make_pair(key, _uninitializedCacheEntry)));
      if (UNLIKELY(!pair.second))
      {
        if (pair.first->second != _uninitializedCacheEntry)
        {
          this->moveToAccumulator(pair.first->second.get());
        }
        pair.first->second = _uninitializedCacheEntry;
      }
      _mutex.releaseWrite();
    }
    mallocController.activate();

    // the item is not present already.
    // unlock the container, while we perform the expensive
    // operation of constructing the object
    setter valueSetter(*this, key);
    bool distCacheOp(false);
    Type* ptr = this->getDistCache(key, distCacheOp);
    if (LIKELY(nullptr == ptr))
    {
      ptr = Cache<Key, Type>::_factory.create(key, 0)._ptr;
    }
    _pointer_type ret(ptr);
    valueSetter.setValue(ret, distCacheOp);
    return ret;
  }

  void put(const Key& key, Type* object, bool updateLDC = true) override
  {
    put(key, _pointer_type(object), updateLDC);
  }

  virtual void put(const Key& key, _pointer_type object, bool updateLDC = true)
  {
    {
      TSEWriteGuard<> l(_mutex);
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
      this->queueDiskPut(key, true);
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
          result += 1;
        }
        _map.erase(it);
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
}; // class SimpleCache

} // namespace sfc

