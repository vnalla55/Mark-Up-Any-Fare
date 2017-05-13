//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once

#include "Allocator/TrxMalloc.h"
#include "Common/Global.h"
#include "Common/Hasher.h"
#include "Common/KeyedFactory.h"
#include "DBAccess/Cache.h"

#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>

#include <list>
#include <stdexcept>

#include <tr1/unordered_map>

namespace sfc
{
template <typename Key, typename Type, typename FactoryType = KeyedFactory<Key, Type> >
class LRUCache : public Cache<Key, Type, FactoryType>
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

  typedef typename Cache<Key, Type, FactoryType>::pointer_type _pointer_type;
  typedef std::pair<Key, _pointer_type> Pair;
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
  _pointer_type getInternal(const Key& key, Parm parm)
  {
    MallocContextDisabler mallocController(false);

    _pointer_type oldObj;

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
            mallocController.activate();
            bool distCacheOp(false), ldcOp(true);
            erase(key, distCacheOp, ldcOp);
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
        _pointer_type ret(reCreate(key, parm, oldObj.get()), deleter(this->_factory, key));
        _pointer_type oldItem;
        {
          boost::mutex::scoped_lock lock(_mutex);
          _map.erase(key);
          bool distCacheOp(false);
          putWithoutLock(key, ret, oldItem, distCacheOp);

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
        _pointer_type oldItem;
        _pointer_type ptr;
        bool distCacheOp(false);
        Type* type(this->getDistCache(key, distCacheOp));
        if (0 == type)
        {
          type = create(key, parm);
        }
        // we have the item, now insert it into the cache
        {
          ptr = std::shared_ptr<Type>(type, deleter(this->_factory, key));
          boost::mutex::scoped_lock lock(_mutex);
          _map.erase(key);
          putWithoutLock(key, ptr, oldItem, distCacheOp);

          // Notify any threads waiting that the object is initialized
          _condition.notify_all();
        }
        return ptr;
      }
    }
    catch (...)
    {
      // Remove the placeholder from the cache and notify any other thread waiting for it
      boost::mutex::scoped_lock lock(_mutex);
      _map.erase(key);
      bool distCacheOp(false), ldcOp(true);
      queueDiskInvalidate(key, distCacheOp, ldcOp);

      // Notify any threads waiting that the object is initialized
      _condition.notify_all();

      throw;
    }

    const _pointer_type ret(create(key, parm), deleter(this->_factory, key));
    return ret;
  }

  template <typename Parm>
  Type* create(const Key& key, Parm p)
  {
    return this->_factory.create(key, p);
  }

  // a dummy type used to signal to 'getInternal' that there is
  // no parameter at all
  struct null_t
  {
  };

  Type* create(const Key& key, null_t p) { return this->_factory.create(key); }

  template <typename Parm>
  Type* reCreate(const Key& key, Parm p, Type* object)
  {
    return this->_factory.reCreate(key, p, object);
  }

  Type* reCreate(const Key& key, null_t p, Type* object)
  {
    return this->_factory.reCreate(key, object);
  }

  bool validate(const Key& key, Type* object) { return this->_factory.validate(key, object); }

public:
  LRUCache(FactoryType& factory,
           const std::string& name,
           size_t version,
           size_t capacity = tse::Global::getUnlimitedCacheSize())
    : Cache<Key, Type, FactoryType>(factory, "LRUCache", name, version),
      _capacity((capacity == tse::Global::getUnlimitedCacheSize()) ? CAPACITY_MAX : capacity)
  {
  }

  virtual ~LRUCache() {}

  virtual size_t size()
  {
    boost::mutex::scoped_lock lock(_mutex);
    return _map.size();
  }

  virtual void reserve(size_t capacity)
  {
    boost::mutex::scoped_lock lock(_mutex);

    _capacity = (capacity == tse::Global::getUnlimitedCacheSize()) ? CAPACITY_MAX : capacity;
  }

  virtual _pointer_type get(const Key& key) { return getInternal(key, null_t()); }

  template <typename Parm>
  _pointer_type get(const Key& key, Parm parm)
  {
    return getInternal(key, parm);
  }

  virtual _pointer_type getIfResident(const Key& key)
  {
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

    return _pointer_type();
  }

  virtual void put(const Key& key, Type* object, bool updateLDC = true)
  {
    put(key, _pointer_type(object, deleter(this->_factory, key)), updateLDC);
  }

  virtual void put(const Key& key, _pointer_type object, bool updateLDC = true)
  {
    _pointer_type oldItem;
    boost::mutex::scoped_lock lock(_mutex);

    // insert the item at the front of the list, and attempt to
    // insert an item into the map pointing at it
    _list.push_front(Pair(key, object));
    const std::pair<MapIterator, bool> res =
        _map.insert(std::pair<Key, ListIterator>(key, _list.begin()));
    if (res.second && updateLDC)
    {
      queueDiskPut(key, true);
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

  virtual void invalidate(const Key& key)
  {
    _pointer_type keepAliveRef;
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
    }

    bool distCacheOp(true), ldcOp(true);
    erase(key, distCacheOp, ldcOp);
  }

  virtual std::shared_ptr<std::vector<Key>> keys()
  {
    boost::mutex::scoped_lock lock(_mutex);
    const std::shared_ptr<std::vector<Key>> allKeys(new std::vector<Key>);
    allKeys->reserve(_map.size());
    for (MapIterator j = _map.begin(); j != _map.end(); j++)
    {
      allKeys->push_back((*j).first);
    }
    return allKeys;
  }

  virtual void clear()
  {
    boost::mutex::scoped_lock lock(_mutex);
    _map.clear();
    _list.clear();
    this->queueDiskClear();
  }

private:
  // function to make room in the cache when a new item is added.
  // the function will delete one item if the cache is oversize.
  // it should be called every time a new item is added to ensure
  // the cache stays at the right size.
  //
  // the 'oldItem' parameter can be used to guarantee the item isn't
  // destroyed until after the mutex has been released
  void make_room(_pointer_type& oldItem, bool updateLDC = true)
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
        queueDiskInvalidate(key, distCacheOp, ldcOp);
      }
      _list.pop_back();
    }
  }

  void erase(const Key& key, bool distCacheOp, bool ldcOp)
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
      queueDiskInvalidate(key, distCacheOp, ldcOp);
    }
  }

  virtual void
  putWithoutLock(const Key& key, _pointer_type object, _pointer_type& oldItem, bool distCacheOp)
  {
    // insert the item at the front of the list, and attempt to
    // insert an item into the map pointing at it
    _list.push_front(Pair(key, object));
    const std::pair<MapIterator, bool> res =
        _map.insert(std::pair<Key, ListIterator>(key, _list.begin()));
    if (res.second)
    {
      queueDiskPut(key, distCacheOp);
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
    if (oldItem.get() == NULL)
    {
      make_room(oldItem);
    }
  }

  virtual void emptyTrash() {}
}; // LRUCache

} // namespace sfc

