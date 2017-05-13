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
      tse::Hasher hasher;
      hasher.setMethod(tse::Global::hasherMethod());
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
    setter(FIFOCache<Key, Type>& cache, const Key& key) : _cache(cache), _key(key), _valueSet(false)
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
            if (_cache._map.erase(oldKey) != 0)
            {
              oldKeys.push_back(oldKey);
            }
            _cache._deque.pop_front();
          }
        }
        _cache._deque.push_back(_key);
        _cache._map[_key] = value;
      }
      bool ldcOp(true);
      for (typename std::vector<Key>::const_iterator i = oldKeys.begin(); i != oldKeys.end(); ++i)
      {
        _cache.queueDiskInvalidate(*i, false, ldcOp);
      }
      _cache.queueDiskPut(_key, distCacheOp);
      _valueSet = true;
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
    : Cache<Key, Type>(factory, "FIFOCache", name, version), _capacity(capacity)
  {
    std::cerr << __FILE__ << ":" << __FUNCTION__ << ": " << capacity << std::endl;
    PointerType tmpPtr(new Type);
    _uninitializedCacheEntry = tmpPtr;
  }

  virtual ~FIFOCache() {}

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

  virtual PointerType getIfResident(const Key& key)
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

  virtual PointerType get(const Key& key)
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
      Type* ptr(this->getDistCache(key, distCacheOp));
      if (0 == ptr)
      {
        ptr = Cache<Key, Type>::_factory.create(key);
      }
      const PointerType ret = std::shared_ptr<Type>(ptr, deleter(this->_factory, key));
      valueSetter.setValue(ret, distCacheOp);
      return ret;
    }
  }

  virtual void put(const Key& key, Type* object, bool updateLDC = true)
  {
    put(key, PointerType(object, deleter(Cache<Key, Type>::_factory, key)), updateLDC);
  }

  virtual void put(const Key& key, PointerType object, bool updateLDC = true)
  {
    bool erased(false);
    Key oldKey;
    {
      TSEWriteGuard<> l(_mutex);
      if (_capacity > 0 && _capacity <= _map.size())
      {
        oldKey = _deque.front();
        if (_map.erase(oldKey))
        {
          erased = true;
        }
        _deque.pop_front();
      }
      _deque.push_back(key);
      _map[key] = object;
    }
    if (updateLDC)
    {
      bool distCacheOp(true);
      if (erased)
      {
        bool ldcOp(true);
        queueDiskInvalidate(oldKey, false, ldcOp);
      }
      queueDiskPut(key, distCacheOp);
    }
  }

  virtual void invalidate(const Key& key)
  {
    {
      TSEWriteGuard<> l(_mutex);
      _map.erase(key);
    }
    bool distCacheOp(true), ldcOp(true);
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
      _deque.clear();
    }
    this->queueDiskClear();
  }

  virtual void emptyTrash() {}
}; // class FIFOCache

} // namespace sfc

