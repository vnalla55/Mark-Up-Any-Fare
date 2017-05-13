//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once

#include "AddonConstruction/ConstructionDefs.h"
#include "AddonConstruction/SPCache.h"
#include "Allocator/TrxMalloc.h"
#include "Common/Global.h"
#include "Common/Hasher.h"
#include "Common/KeyedFactory.h"
#include "Common/CacheStats.h"

#include <boost/detail/atomic_count.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>

#include <list>
#include <memory>
#include <stdexcept>

#include <tr1/unordered_map>

namespace sfc
{
template <class Key, class Type, class FactoryType = KeyedFactory<Key, Type>>
class SPLRUCache : public sfc::SPCache<Key, Type, FactoryType>
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

  typedef std::pair<Key, std::shared_ptr<Type>> Pair;
  typedef std::list<Pair> List;
  typedef typename List::iterator ListIterator;
  typedef typename List::reverse_iterator ListReverseIterator;
  typedef std::tr1::unordered_map<Key, ListIterator, hash_func> Map;
  typedef typename Map::iterator MapIterator;
  Map _map;
  boost::mutex _mutex;
  boost::condition _condition;
  List _list;
  size_t _capacity;
  static const size_t CAPACITY_MAX = (size_t)(-1);
  tse::CacheStats _cacheStats;
  boost::detail::atomic_count _accessCount;
  boost::detail::atomic_count _readCount;

  struct deleter
  {
  public:
    deleter(FactoryType& factory, const Key& key) : _factory(factory), _key(key) {}

    void operator()(void* p) const { _factory.destroy(_key, reinterpret_cast<Type*>(p)); }

  private:
    FactoryType& _factory;
    Key _key;
  };

  template <typename Parm>
  std::shared_ptr<Type> getInternal(const Key& key, Parm parm)
  {
    MallocContextDisabler mallocController(false);

    std::shared_ptr<Type> oldObj;

    // see if the item is in the cache already, locking the container
    // while we perform the lookup
    {
      boost::mutex::scoped_lock lock(_mutex);

      MapIterator j = _map.find(key);
      if (j != _map.end())
      {
        bool useExistingObject = true;
        // If the object is in the process of being put in the cache,
        // wait for it to complete
        while (j->second == _list.end())
        {
          _condition.wait(lock);
          j = _map.find(key);

          // IF the item was deleted from the map
          if (j == _map.end())
          {
            // The item is not present in the cache.  Add an entry to the cache
            // indicating it is in the process of being constructed so as to
            // eliminate duplicate object creations
            mallocController.activate();
            _map.insert(std::pair<Key, ListIterator>(key, _list.end()));

            // Create the object in this thread
            useExistingObject = false;
            break;
          }
        }

        // the item is present, so move it to the front of the list
        // and return it
        if (useExistingObject)
        {
          // IF the object is valid,
          //    move it to the front of the list and return it

          oldObj = (*(j->second)).second;
          if (validate(key, oldObj.get()))
          {
            _list.splice(_list.begin(), _list, j->second);
            return _list.front().second;
          }
          // ELSE (the object is not valid)
          //     Set the flag to recreate it and flag it as "under construction"
          else
          {
            this->_factory.logUnderConstruction(key, oldObj.get());
            mallocController.activate();
            erase(key);
            _map.insert(std::pair<Key, ListIterator>(key, _list.end()));
          }
        }
      }
      else
      {
        // The item is not present in the cache.  Add an entry to the cache indicating
        // it is in the process of being constructed so as to eliminate duplicate
        // object creations
        mallocController.activate();
        _map.insert(std::pair<Key, ListIterator>(key, _list.end()));
      }
    }

    try
    {
      // If the object already exists
      if (oldObj.get())
      {
        // Recreate it
        mallocController.activate();
        std::shared_ptr<Type> ret(reCreate(key, parm, oldObj.get()), deleter(this->_factory, key));
        std::shared_ptr<Type> oldItem;
        {
          boost::mutex::scoped_lock lock(_mutex);
          _map.erase(key);
          putWithoutLock(key, ret, oldItem, true);

          // Notify any threads waiting that the object is initialized
          _condition.notify_all();
        }

        return ret;
      }
      else
      {
        // the item is not already present.
        // unlock the container, while we perform the expensive
        // operation of constructing the object
        mallocController.activate();
        std::shared_ptr<Type> oldItem;
        bool distCacheOp(false);
        Type* ptr(this->getDistCache(key, distCacheOp));
        if (nullptr == ptr)
        {
          ptr = create(key, parm);
        }

        std::shared_ptr<Type> ret(ptr, deleter(this->_factory, key));
        // we have the item, now insert it into the cache
        {
          boost::mutex::scoped_lock lock(_mutex);
          _map.erase(key);
          putWithoutLock(key, ret, oldItem, distCacheOp);

          // Notify any threads waiting that the object is initialized
          _condition.notify_all();
        }

        return ret;
      }
    }
    catch (...)
    {
      // Remove the placeholder from the cache and notify any other thread waiting for it
      boost::mutex::scoped_lock lock(_mutex);
      if (_map.erase(key) > 0)
      {
        this->queueDiskInvalidate(key, false, true);
      }
      // Notify any threads waiting that the object is initialized
      _condition.notify_all();

      throw;
    }

    const std::shared_ptr<Type> ret(create(key, parm), deleter(this->_factory, key));
    return ret;
  }

  template <typename Parm>
  Type* create(const Key& key, Parm p)
  {
    ++_accessCount;
    return this->_factory.create(key, p);
  }

  // a dummy type used to signal to 'getInternal' that there is
  // no parameter at all
  struct null_t
  {
  };

  Type* create(const Key& key, null_t p)
  {
    ++_accessCount;
    return this->_factory.create(key);
  }

  template <typename Parm>
  Type* reCreate(const Key& key, Parm p, Type* object)
  {
    ++_accessCount;
    return this->_factory.reCreate(key, p, object);
  }

  Type* reCreate(const Key& key, null_t p, Type* object)
  {
    ++_accessCount;
    return this->_factory.reCreate(key, object);
  }

  bool validate(const Key& key, Type* object) { return this->_factory.validate(key, object); }

public:
  SPLRUCache(FactoryType& factory,
             const std::string& name,
             size_t version,
             size_t capacity = tse::Global::getUnlimitedCacheSize())
    : SPCache<Key, Type, FactoryType>(factory, "LRUCache", name, version),
      _capacity((capacity == tse::Global::getUnlimitedCacheSize()) ? CAPACITY_MAX : capacity),
      _accessCount(0),
      _readCount(0)
  {
  }

  virtual ~SPLRUCache() {}

  size_t size() override
  {
    boost::mutex::scoped_lock lock(_mutex);
    return _map.size();
  }

  virtual void reserve(size_t capacity)
  {
    boost::mutex::scoped_lock lock(_mutex);

    _capacity = (capacity == tse::Global::getUnlimitedCacheSize()) ? CAPACITY_MAX : capacity;
  }

  virtual size_t capacity() { return _capacity; }

  virtual uint64_t accessCount() { return _accessCount; }

  virtual uint64_t readCount() { return _readCount; }

  std::shared_ptr<Type> get(const Key& key) override
  {
    ++_readCount;
    return getInternal(key, null_t());
  }

  template <typename Parm>
  std::shared_ptr<Type> get(const Key& key, Parm parm)
  {
    ++_readCount;
    return getInternal(key, parm);
  }

  std::shared_ptr<Type> getIfResident(const Key& key) override
  {
    ++_readCount;
    // see if the item is in the cache already, locking the container
    // while we perform the lookup
    {
      boost::mutex::scoped_lock lock(_mutex);

      MapIterator j = _map.find(key);
      if (j != _map.end())
      {
        bool useExistingObject = true;
        // If the object is in the process of being put in the cache,
        // wait for it to complete
        while (j->second == _list.end())
        {
          _condition.wait(lock);
          j = _map.find(key);

          // IF the item was deleted from the map
          if (j == _map.end())
          {
            // Create the object in this thread
            useExistingObject = false;
            break;
          }
        }

        // the item is present, so
        if (useExistingObject)
        {
          //    move it to the front of the list and return it
          _list.splice(_list.begin(), _list, j->second);
          return _list.front().second;
        }
      }
    }

    return std::shared_ptr<Type>();
  }

  void put(const Key& key, Type* object, bool updateLDC = true) override
  {
    put(key, std::shared_ptr<Type>(object, deleter(this->_factory, key)), updateLDC);
  }

  virtual void put(const Key& key, std::shared_ptr<Type> object, bool updateLDC = true)
  {
    std::shared_ptr<Type> oldItem;
    boost::mutex::scoped_lock lock(_mutex);

    // insert the item at the front of the list, and attempt to
    // insert an item into the map pointing at it
    _list.push_front(Pair(key, object));
    const std::pair<MapIterator, bool> res =
        _map.insert(std::pair<Key, ListIterator>(key, _list.begin()));
    if (res.second && updateLDC)
    {
      this->queueDiskPut(key, true);
    }

    // if an item with the same key is already in the cache, then
    // erase the item, and reassign the iterator in the map to the newly
    // inserted item at the front of the list
    if (!res.second)
    {
      if (res.first->second != _list.end())
      {
        // make sure the item isn't actually destroyed
        // until after we unlock the mutex
        object = res.first->second->second;
        _list.erase(res.first->second);
      }
      res.first->second = _list.begin();
    }

    // make sure there's enough room for the item
    make_room(oldItem, updateLDC);
  }

  size_t invalidate(const Key& key) override
  {
    size_t result(0);
    std::shared_ptr<Type> keepAliveRef;
    boost::mutex::scoped_lock lock(_mutex);

    // Wait for any construction of the object that is in progress
    //      to end before clearing the cache object
    MapIterator j = _map.find(key);
    while (j != _map.end() && j->second == _list.end())
    {
      _condition.wait(lock);
      j = _map.find(key);
    }

    if (j != _map.end() && j->second != _list.end())
    {
      keepAliveRef = j->second->second;
      result += 1;
    }

    erase(key);
    this->_factory.logInvalidation(key, keepAliveRef.get());
    return result;
  }

  std::shared_ptr<std::vector<Key>> keys() override
  {
    boost::mutex::scoped_lock lock(_mutex);
    const std::shared_ptr<std::vector<Key>> allKeys(new std::vector<Key>);
    allKeys->reserve(_map.size());
    for (const auto& elem : _map)
    {
      allKeys->push_back(elem.first);
    }
    return allKeys;
  }

  size_t clear() override
  {
    boost::mutex::scoped_lock lock(_mutex);
    size_t result(_map.size());
    _map.clear();
    _list.clear();
    this->queueDiskClear();
    return result;
  }

  void flush()
  {
    List list;
    Map map;
    tse::FlushMap flushMap;
    {
      boost::mutex::scoped_lock lock(_mutex);
      boost::lock_guard<boost::mutex> g(this->_factory.factoryMutex());
      _map.swap(map);
      _list.swap(list);
      this->_factory.flushMap().swap(flushMap);
      this->queueDiskClear();
    }
    tse::FlushMapIter iter(flushMap.begin()), iterEnd(flushMap.end());
    for (; iter != iterEnd; ++iter)
    {
      delete iter->second;
    }
  }

  tse::CacheStats* cacheStats() { return &_cacheStats; }

private:
  // function to make room in the cache when a new item is added.
  // the function will delete one item if the cache is oversize.
  // it should be called every time a new item is added to ensure
  // the cache stays at the right size.
  //
  // the 'oldItem' parameter can be used to guarantee the item isn't
  // destroyed until after the mutex has been released
  void make_room(std::shared_ptr<Type>& oldItem, bool updateLDC = true)
  {
    if (_map.size() > _capacity && !_list.empty())
    {
      ListReverseIterator i = _list.rbegin();
      oldItem = i->second;
      // erase(i->first);
      const Key& key = i->first;
      _map.erase(key);
      if (updateLDC)
      {
        bool distCacheOp(false), ldcOp(true);
        if (!validate(key, oldItem.get()))
          distCacheOp = true; // if the old item was partially valid
        this->queueDiskInvalidate(key, distCacheOp, ldcOp);
      }
      _list.pop_back();
    }
  }

  void erase(const Key& key)
  {
    const MapIterator j = _map.find(key);
    if (j != _map.end())
    {
      // If the object has more than a placeholder in the map
      if (j->second != _list.end())
      {
        // Remove the object from the list
        _list.erase(j->second);
      }

      // Remove the object from the map (placeholder or full object)
      _map.erase(j);
      this->queueDiskInvalidate(key, true, true);
    }
  }

  virtual void putWithoutLock(const Key& key,
                              std::shared_ptr<Type> object,
                              std::shared_ptr<Type>& oldItem,
                              bool distCacheOp)
  {
    // insert the item at the front of the list, and attempt to
    // insert an item into the map pointing at it
    _list.push_front(Pair(key, object));
    const std::pair<MapIterator, bool> res =
        _map.insert(std::pair<Key, ListIterator>(key, _list.begin()));
    if (res.second)
    {
      this->queueDiskPut(key, distCacheOp);
    }

    // if an item with the same key is already in the cache, then
    // erase the item, and reassign the iterator in the map to the newly
    // inserted item at the front of the list
    if (!res.second)
    {
      if (res.first->second != _list.end())
      {
        oldItem = res.first->second->second;
        _list.erase(res.first->second);
      }
      res.first->second = _list.begin();
    }

    // make sure there's enough room for the item
    if (oldItem.get() == nullptr)
    {
      make_room(oldItem);
    }
  }
};

} // namespace sfc

