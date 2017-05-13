#pragma once

#include "Allocator/TrxMalloc.h"
#include "Common/Hasher.h"
#include "Common/KeyedFactory.h"
#include "Common/Thread/TSELockGuards.h"
#include "Common/Thread/TSEReadWriteLock.h"
#include "DBAccess/Cache.h"

#include <list>
#include <set>
#include <stdexcept>

#include <tr1/unordered_map>

namespace sfc
{
template <typename Key, typename Type>
class DualMapCache : public Cache<Key, Type>
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
  typedef std::set<Key> Set;
  typedef typename Set::iterator SetIterator;
  typedef std::tr1::unordered_map<Key, PointerType, hash_func> Map;
  typedef typename Map::iterator MapIterator;

  Map _readMap;
  Map _readWriteMap;
  TSEReadWriteLock _mutex;
  PointerType _initializingCacheEntry;
  PointerType _flushedCacheEntry;
  size_t _capacity; // not used
  bool _mustClear;
  Set _deleteList;

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
  // the 'initializing' entry will go away even if an exception
  // is generated.
  struct mapValueSetter
  {
    mapValueSetter(DualMapCache<Key, Type>& cache, bool setReadWriteMap, const Key& key)
      : _cache(cache), _key(key), _setReadWriteMap(setReadWriteMap), _valueSet(false)
    {
    }

    ~mapValueSetter()
    {
      if (!_valueSet)
      {
        TSEWriteGuard<> l(_cache._mutex);
        if (_setReadWriteMap)
        {
          _cache._readWriteMap.erase(_key);
        }
        else
        {
          _cache._readMap[_key] = _cache._flushedCacheEntry;
        }
      }
    }

    void setValue(const PointerType& value)
    {
      TSEWriteGuard<> l(_cache._mutex);
      if (_setReadWriteMap)
      {
        _cache._readWriteMap[_key] = value;
      }
      else
      {
        _cache._readMap[_key] = value;
      }
      _cache.queueDiskPut(_key, true);
      _valueSet = true;
    }

  private:
    DualMapCache<Key, Type>& _cache;
    const Key& _key;
    bool _setReadWriteMap;
    bool _valueSet;
  };

public:
  DualMapCache(KeyedFactory<Key, Type>& factory,
               const std::string& name,
               size_t capacity,
               size_t version)
    : Cache<Key, Type>(factory, "SimpleDualMapCache", name, version),
      _capacity(capacity),
      _mustClear(false)
  {
    PointerType initializingPtr(new Type);
    PointerType flushedPtr(new Type);

    _initializingCacheEntry = initializingPtr;
    _flushedCacheEntry = flushedPtr;
  }

  virtual ~DualMapCache() {}

  size_t size() override
  {
    // There should be no lock necessary for fetching this number.
    // The copy is an atomic operation.
    // TSEReadGuard<> l(_mutex);

    return _readMap.size() + _readWriteMap.size();
  }

  size_t consolidationSize() override
  {
    // There should be no lock necessary for fetching this number.
    // The copy is an atomic operation.
    // TSEReadGuard<> l(_mutex);

    return _readWriteMap.size();
  }

  virtual void reserve(size_t capacity)
  {
    // There should be no lock necessary for fetching this number.
    // The copy is an atomic operation.
    // TSEWriteGuard<> l(_mutex);

    _capacity = capacity;
  }

  PointerType getIfResident(const Key& key) override
  {
    // Check the "quick access" map first
    MapIterator mapIter = _readMap.find(key);
    if (mapIter != _readMap.end())
    {
      if ((mapIter->second != _initializingCacheEntry) && (mapIter->second != _flushedCacheEntry))
      {
        return mapIter->second;
      }
      else
      {
        return PointerType();
      }
    }
    else
    {
      // IF it wasn't found there, check the "updatable" map
      TSEReadGuard<> l(_mutex);
      mapIter = _readWriteMap.find(key);
      if (mapIter != _readWriteMap.end() && mapIter->second != _initializingCacheEntry)
      {
        return mapIter->second;
      }
      else
      {
        return PointerType();
      }
    }
  }
  void emptyTrash() override {}

  PointerType get(const Key& key) override
  {
    // Check the "quick access" map first
    MapIterator j = _readMap.find(key);

    // IF there is an entry in the "quick access" map
    if (j != _readMap.end())
    {
      // IF it is a valid entry  -> return it

      if ((j->second != _initializingCacheEntry) && (j->second != _flushedCacheEntry))
      {
        return j->second;
      }
      else
      {

        TSEReadGuard<> l(_mutex);

        // An entry exists in the "quick access" map for this object.
        // This means that either it was present and is no longer or
        // it is in the process of being loaded by another thread.

        // Set the flag to indicate if it is being loaded by another thread

        bool usePreExisting = (j->second != _flushedCacheEntry);

        // LOOP until either the item has been loaded by another thread or
        //      it has been flushed from the cache.  (The "end()" test is really
        //      not needed here since entries are not ever removed from this map.
        //      They are merely flagged as "flushed".)
        while (j != _readMap.end() && j->second == _initializingCacheEntry && usePreExisting)
        {
          // Delay to give the other thread time to load the object

          l.release();
          usleep(10);
          l.acquire();

          // FETCH the intry for the object from the map
          j = _readMap.find(key);

          // IF it is now flagged as having been flushed
          if (j == _readMap.end())
          {
          }
          else if (j->second == _flushedCacheEntry)
          {
            // Ensure solo updatge access to the map
            l.release();
            TSEWriteGuard<> lw(_mutex);

            // IF the object is still flagged as flushed (no other thread is
            //    creating it)

            j = _readMap.find(key);
            if (j->second == _flushedCacheEntry)
            {
              // ASSUME responsibility for creating the object
              usePreExisting = false;

              _readMap[key] = _initializingCacheEntry;
            }
          }
        }

        // the item is present, return it
        if (usePreExisting)
        {
          return j->second;
        }
        else
        {
          // the item is not present already.
          // Perform the expensive operation of constructing the object

          MallocContextDisabler mallocController;

          mapValueSetter valueSetter(*this, false, key);
          const PointerType ret(Cache<Key, Type>::_factory.create(key));
          valueSetter.setValue(ret);
          return ret;
        }
      }
    }
    else
    {
      // IF it wasn't found there, check the "updatable" map

      MallocContextDisabler mallocController(false);

      // see if the item is in the cache already, locking the container
      // while we perform the lookup
      {
        TSEReadGuard<> l(_mutex);
        j = _readWriteMap.find(key);
        if (j != _readWriteMap.end())
        {
          bool usePreExisting = true;

          while (j != _readWriteMap.end() && j->second == _initializingCacheEntry && usePreExisting)
          {
            l.release();
            usleep(10);
            l.acquire();

            j = _readWriteMap.find(key);
            if (j == _readWriteMap.end())
            {
              mallocController.activate();
              usePreExisting = false;

              TSEWriteGuard<> l(_mutex);
              _readWriteMap[key] = _initializingCacheEntry;
            }
          }
          // the item is present, return it
          if (usePreExisting)
            return j->second;
        }
        else
        {
          mallocController.activate();

          l.release();
          TSEWriteGuard<> lw(_mutex);
          _readWriteMap[key] = _initializingCacheEntry;
        }
      }

      mallocController.activate();

      // the item is not present already.
      // unlock the container, while we perform the expensive
      // operation of constructing the object
      mapValueSetter valueSetter(*this, true, key);
      const PointerType ret(Cache<Key, Type>::_factory.create(key));
      valueSetter.setValue(ret);
      return ret;
    }
  }

  void put(const Key& key, Type* object, bool updateLDC = true) override
  {
    put(key, PointerType(object), updateLDC);
  }

  virtual void put(const Key& key, PointerType object, bool updateLDC = true)
  {
    // IF an object for this key is in the "readOnly map"
    MapIterator j = _readMap.find(key);
    if (j != _readMap.end())
    {
      // Replace it
      _readMap[key] = object;
    }
    else // ELSE (Not present in the "ReadOnly map")
    {
      // Add the object to the "updateable map", or replace the entry for the key if present
      // (There should be a check here for the "uninitialized entry to fire trigger)

      j = _readWriteMap.find(key);
      if (j != _readWriteMap.end())
      {
        _readWriteMap[key] = object;
      }
      else
      {
        // Add the object to the "updateable map", or replace the entry for the key if present
        TSEWriteGuard<> l(_mutex);
        _readWriteMap[key] = object;
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
    bool ldcOp(true), distCacheOp(true);
    this->queueDiskInvalidate(key, distCacheOp, ldcOp);

    // IF the key is present in the "read only" map.
    MapIterator readMapIter = _readMap.find(key);
    if (readMapIter != _readMap.end())
    {
      // Flag it as invalid
      _readMap[key] = _flushedCacheEntry;

      TSEWriteGuard<> l(_mutex);
      result += 1;
      _deleteList.insert(key);
    }
    else
    {
      // Remove the key from the "updatable map" if present
      TSEWriteGuard<> l(_mutex);
      _readWriteMap.erase(key);
    }
    return result;
  }

  std::shared_ptr<std::vector<Key>> keys() override
  {
    const std::shared_ptr<std::vector<Key>> allKeys(new std::vector<Key>);

    allKeys->reserve(_readMap.size() + _readWriteMap.size());

    // Get the keys from the "read only" map
    for (auto& elem : _readMap)
    {
      // Don't include the key if the object was removed from the cache
      if (elem.second != _initializingCacheEntry && elem.second != _flushedCacheEntry)
        allKeys->push_back(elem.first);
    }

    // Add any keys that are in the "updatable map"
    TSEReadGuard<> l(_mutex);
    for (auto& elem : _readWriteMap)
    {
      // Don't include the key if the object is in the process of being created
      if (elem.second != _initializingCacheEntry)
        allKeys->push_back(elem.first);
    }
    return allKeys;
  }

  size_t clear() override
  {
    size_t result(0);
    // Flag the "readOnly map" entries as invalid
    // (Can't actually clear it because there is no locking for this map)
    for (auto& elem : _readMap)
    {
      // Don't include the key if the object is in the process of being created
      // if (j->second != _initializingCacheEntry && j->second != _flushedCacheEntry)
      {
        _readMap[elem.first] = _flushedCacheEntry;

        TSEWriteGuard<> l(_mutex);
        _deleteList.insert(elem.first);
      }
    }

    // Delete all entries from the updatable map
    TSEWriteGuard<> l(_mutex);
    result = _readWriteMap.size();
    _readWriteMap.clear();

    this->queueDiskClear();
    return result;
  }

  size_t consolidate(size_t maxRecordsToConsolidate) override
  {
    size_t numMoved = 0;

    // Ensure there is no contention during the updates
    TSEWriteGuard<> l(_mutex);

    // FOR EACH item in the "delete list"
    for (const auto& elem : _deleteList)
    {
      // IF it has not been replaced in the map
      MapIterator j = _readMap.find(elem);
      if (j != _readMap.end() && (j->second == _flushedCacheEntry))
      {
        //  Remove it from the "read only" map
        _readMap.erase(elem);
      }
    }

    // Clear the "delete list"
    _deleteList.clear();

    // FOR EACH item in the "updatable map"
    for (MapIterator j = _readWriteMap.begin();
         (j != _readWriteMap.end()) && (numMoved < maxRecordsToConsolidate);
         j++, numMoved++)
    {
      //     Insert it into the "read only" map
      _readMap[j->first] = j->second;
    }

    // Remove all entries from the "updatabale map"
    _readWriteMap.clear();

    return numMoved;
  }
};

} // namespace sfc

