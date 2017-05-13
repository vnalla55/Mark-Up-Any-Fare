//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.

//-------------------------------------------------------------------

#pragma once

#include <boost/unordered_map.hpp>
#include <boost/thread.hpp>
#include "Cache.h"

namespace sfc
{
  template<typename Key, typename Type>
  class SimpleCache : public Cache<Key, Type>
  {
  private:
    struct hash_func
    {
      size_t operator() (const Key& key) const
      {
        tse::Hasher hasher;
        hasher.setMethod(tse::Global::hasherMethod());
        hasher << key;
        return hasher.hash();
      }
    };
    typedef typename Cache<Key, Type>::pointer_type _pointer_type;
    _pointer_type  _uninitializedCacheEntry;

    typedef boost::unordered_map<Key, _pointer_type, hash_func> Map;

    Map _map;
    boost::shared_mutex _mutex;
    boost::condition_variable_any _condition;

    _pointer_type get (const Key &key,
                       bool create)
    {
      {
        boost::shared_lock<boost::shared_mutex> lock(_mutex);
        typename Map::const_iterator mit(_map.find(key));
        if (mit != _map.end()
            && mit->second != _uninitializedCacheEntry)
        {
          return mit->second;
        }
      }
      MallocContextDisabler disableCustomAllocator;
      try
      {
        {
          boost::unique_lock<boost::shared_mutex> lock(_mutex);
          typename Map::iterator mit(_map.find(key));
          bool inMap(mit != _map.end());
          if (inMap)
          {
            if (mit->second != _uninitializedCacheEntry)
            {
              return mit->second;
            }
            else// another thread put _uninitializedCacheEntry
            {
              do
              {
                _condition.wait(lock);
                inMap = (mit = _map.find(key)) != _map.end();
                if (inMap && mit->second != _uninitializedCacheEntry)
                {
                  return mit->second;
                }
              }
              while (inMap && _uninitializedCacheEntry == mit->second);
            }
          }
          else if (create)// not in the _map
          {
            std::pair<typename Map::iterator, bool> pr(_map.emplace(key, _uninitializedCacheEntry));
          }
        }
        if (create)
        {
          return createEntry(key);
        }
      }
      catch (...)
      {
        invalidate(key);
        _condition.notify_all();
        throw;
      }
      return _pointer_type();
    }

    _pointer_type createEntry (const Key &key)
    {
      _pointer_type created;
      _pointer_type ret;
      try
      {
        created = _pointer_type(this->_factory.create(key));
        ret = tryInsertMap(key, created);
        _condition.notify_all();
        if (created)
        {
          this->moveToAccumulator(created.get());
        }
        this->queueDiskPut(key, false);
      }
      catch (...)
      {
        if (created)
        {
          this->moveToAccumulator(created.get());
        }
        throw;
      }
      return ret;
    }

    bool remove (const typename Map::iterator &mit)
    {
      bool removed(false);
      if (mit != _map.end())
      {
        if (mit->second != _uninitializedCacheEntry)
        {
          this->moveToAccumulator(mit->second.get());
        }
        _map.erase(mit);
        removed = true;
      }
      return removed;
    }

    _pointer_type tryInsertMap (const Key &key,
                                _pointer_type &object)
    {
      _pointer_type ret;
      if (object)
      {
        boost::unique_lock<boost::shared_mutex> lock(_mutex);
        std::pair<typename Map::iterator, bool> pr(_map.emplace(key, _pointer_type()));
        if (pr.second)
        {
          ret = pr.first->second = object;
          object = _pointer_type();
        }
        else
        {
          ret = object;
          if (pr.first->second != _uninitializedCacheEntry)
          {
            this->moveToAccumulator(pr.first->second.get());
            pr.first->second = _pointer_type();
          }
          pr.first->second = object;
          object = _pointer_type();
        }
      }
      return ret;
    }
  public:
    SimpleCache (KeyedFactory<Key, Type> &factory,
                 const std::string &name,
                 size_t,
                 size_t version)
      : Cache<Key, Type>( factory, "SimpleCache", name, version )
      , _uninitializedCacheEntry(new Type)
    {
    }

    virtual ~SimpleCache()
    {
    }

    virtual size_t size ()
    {
      boost::shared_lock<boost::shared_mutex> lock(_mutex);
      return _map.size();
    }

    virtual void reserve(size_t)
    {
    }

    virtual _pointer_type getIfResident (const Key &key)
    {
      return get(key, false);
    }

    virtual _pointer_type get (const Key &key)
    {
      return get(key, true);
    }

    virtual void put (const Key& key,
                      Type* object,
                      bool updateLDC = true )
    {
      put(key, _pointer_type(object), updateLDC);
    }

    virtual void put (const Key &key,
                      _pointer_type object,
                      bool updateLDC = true )
    {
      bool removed(false);
      Key removedKey;
      tryInsertMap(key, object);
      _condition.notify_all();
      if (updateLDC)
      {
        this->queueDiskPut(key, true);
      }
      if (removed)
      {
        this->queueDiskInvalidate(removedKey, true, false);
      }
      if (object)
      {
        this->moveToAccumulator(object.get());
      }
    }

    virtual void invalidate (const Key &key)
    {
      bool removed(false);
      {
        boost::unique_lock<boost::shared_mutex> lock(_mutex);
        typename Map::iterator mit(_map.find(key));
        removed = remove(mit);
      }
      if (removed)
      {
        _condition.notify_all();
      }
      bool distCacheOp(true);
      this->queueDiskInvalidate(key, distCacheOp, removed);
    }

    virtual std::shared_ptr<std::vector<Key>> keys()
    {
      const std::shared_ptr<std::vector<Key>> allKeys(new std::vector<Key>);
      {
        boost::shared_lock<boost::shared_mutex> lock(_mutex);
        allKeys->reserve(_map.size());
        for (typename Map::const_iterator it(_map.begin()), itend(_map.end()); it != itend; ++it)
        {
          allKeys->push_back(it->first);
        }
      }
      return allKeys;
    }

    virtual void clear ()
    {
      {
        boost::unique_lock<boost::shared_mutex> lock(_mutex);
        while (!_map.empty())
        {
          typename Map::iterator mit(_map.begin());
          if (mit->second != _uninitializedCacheEntry)
          {
            this->moveToAccumulator(mit->second.get());
          }
          _map.erase(mit);
        }
      }
      _condition.notify_all();
      this->queueDiskClear() ;
    }

    virtual void emptyTrash ()
    {
      boost::unique_lock<boost::shared_mutex> lock(_mutex);
      if (!this->_accumulator.empty())
      {
        this->_cacheDeleter.moveToTrashBin(this->_accumulator);
      }
    }
  };// class SimpleCache
}// namespace sfc
