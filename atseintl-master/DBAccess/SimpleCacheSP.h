#pragma once

#include "Allocator/TrxMalloc.h"
#include "Common/Hasher.h"
#include "Common/KeyedFactory.h"
#include "Common/Thread/TSEReadWriteLock.h"
#include "Common/Thread/TSELockGuards.h"
#include "DBAccess/Cache.h"
#include "DBAccess/DST.h"

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
      tse::Hasher hasher;
      hasher.setMethod(tse::Global::hasherMethod());
      hasher << key;
      return hasher.hash();
    }
  };

  typedef typename Cache<Key, Type>::pointer_type _pointer_type;
  typedef std::tr1::unordered_map<Key, _pointer_type, hash_func> Map;
  typedef typename Map::iterator MapIterator;
  Map _map;
  TSEReadWriteLock _mutex;
  _pointer_type _uninitializedCacheEntry;
  size_t _capacity;

protected:
  struct deleter
  {
  public:
    deleter(KeyedFactory<Key, Type>& factory, const Key& key) : _factory(factory), _key(key) {}

    void operator()(void* p) const { _factory.destroy(_key, reinterpret_cast<Type*>(p)); }

  private:
    KeyedFactory<Key, Type>& _factory;
    Key _key;
  };

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
      if (!_valueSet)
      {
        TSEWriteGuard<> l(_cache._mutex);
        _cache._map.erase(_key);
      }
    }

    void setValue(const _pointer_type& value, bool distCacheOp)
    {
      {
        TSEWriteGuard<> l(_cache._mutex);
        _cache._map[_key] = value;
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
    : Cache<Key, Type>(factory, "SimpleCache", name, version), _capacity(capacity)
  {
    _pointer_type tmpPtr(new Type);
    _uninitializedCacheEntry = tmpPtr;
  }

  virtual ~SimpleCache() {}

  virtual size_t size()
  {
    TSEReadGuard<> l(_mutex);
    return _map.size();
  }

  virtual void reserve(size_t capacity)
  {
    TSEWriteGuard<> l(_mutex);
    _capacity = capacity;
  }

  virtual _pointer_type getIfResident(const Key& key)
  {
    TSEReadGuard<> l(_mutex);
    MapIterator j = _map.find(key);
    if (j != _map.end() && j->second != _uninitializedCacheEntry)
    {
      return j->second;
    }
    else
    {
      return _pointer_type();
    }
  }

  virtual _pointer_type get(const Key& key)
  {
    MallocContextDisabler mallocController(false);

    // see if the item is in the cache already, locking the container
    // while we perform the lookup
    {
      TSEReadGuard<> l(_mutex);
      MapIterator j = _map.find(key);
      if (j != _map.end())
      {
        bool usePreExisting = true;

        while (j != _map.end() && j->second == _uninitializedCacheEntry && usePreExisting)
        {
          l.release();
          usleep(10);
          l.acquire();
          j = _map.find(key);
          if (j == _map.end())
          {
            mallocController.activate();
            usePreExisting = false;
          }
        }
        // the item is present, return it
        if (usePreExisting)
          return j->second;
      }
      else
      {
        mallocController.activate();
      }
    }

    {
      TSEWriteGuard<> l(_mutex);
      _map[key] = _uninitializedCacheEntry;
    }

    mallocController.activate();

    // the item is not present already.
    // unlock the container, while we perform the expensive
    // operation of constructing the object
    setter valueSetter(*this, key);
    bool distCacheOp(false);
    Type* ptr(this->getDistCache(key, distCacheOp));
    if (0 == ptr)
    {
      ptr = Cache<Key, Type>::_factory.create(key);
    }
    const _pointer_type ret = std::shared_ptr<Type>(ptr, deleter(this->_factory, key));
    valueSetter.setValue(ret, distCacheOp);
    return ret;
  }

  virtual void put(const Key& key, Type* object, bool updateLDC = true)
  {
    put(key, _pointer_type(object, deleter(Cache<Key, Type>::_factory, key)), updateLDC);
  }

  virtual void put(const Key& key, _pointer_type object, bool updateLDC = true)
  {
    {
      TSEWriteGuard<> l(_mutex);
      _map[key] = object;
    }
    if (updateLDC)
    {
      this->queueDiskPut(key, true);
    }
  }

  virtual void invalidate(const Key& key)
  {
    bool distCacheOp(true), ldcOp(false);
    {
      TSEWriteGuard<> l(_mutex);
      if (_map.erase(key))
      {
        ldcOp = true;
      }
    }
    this->queueDiskInvalidate(key, distCacheOp, ldcOp);
  }

  virtual std::shared_ptr<std::vector<Key>> keys()
  {
    const std::shared_ptr<std::vector<Key>> allKeys(new std::vector<Key>);

    {
      TSEReadGuard<> l(_mutex);
      allKeys->reserve(_map.size());
      for (MapIterator j = _map.begin(); j != _map.end(); j++)
      {
        allKeys->push_back(j->first);
      }
    }

    return allKeys;
  }

  virtual void clear()
  {
    {
      TSEWriteGuard<> l(_mutex);
      _map.clear();
    }

    this->queueDiskClear();
  }

  virtual void emptyTrash() {}
}; // class SimpleCache

} // namespace sfc
