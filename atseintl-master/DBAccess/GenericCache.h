#pragma once

#include "DBAccess/Cache.h"

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/unordered_map.hpp>

namespace sfc
{
template <typename Key, typename Type>
class GenericCache : public Cache<Key, Type>
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
  typedef typename Cache<Key, Type>::pointer_type _pointer_type;
  _pointer_type _uninitializedCacheEntry;

  typedef boost::unordered_map<Key, _pointer_type, hash_func> Map;

  Map _map;
  boost::shared_mutex _mutex;
  boost::condition_variable_any _condition;

  _pointer_type get(const Key& key,
                    bool create)
  {
    const MallocContextDisabler disableCustomAllocator;
    try
    {
      {
        boost::shared_lock<boost::shared_mutex> lock(_mutex);
        auto mit(_map.find(key));
        bool inMap(mit != _map.end());
        if (inMap)
        {
          if (mit->second != _uninitializedCacheEntry)
          {
            return mit->second;
          }
          else
          {
            do
            {
              _condition.wait(lock);
              inMap = (mit = _map.find(key)) != _map.end();
              if (inMap && mit->second != _uninitializedCacheEntry)
              {
                return mit->second;
              }
            } while (inMap);
          }
        }
      }
      if (!create)
      {
        return _pointer_type();
      }
      {
        std::pair<typename Map::const_iterator, bool> pair;
        boost::upgrade_lock<boost::shared_mutex> lock(_mutex);
        {
          boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);
          pair = _map.emplace(key, _uninitializedCacheEntry);
        }
        if (!pair.second)
        {
          auto mit(pair.first);
          while (mit != _map.end())
          {
            if (mit->second != _uninitializedCacheEntry)
            {
              return mit->second;
            }
            _condition.wait(lock);
            mit = _map.find(key);
          }
        }
      }
      return createEntry(key);
    }
    catch (...)
    {
      invalidate(key);
      throw;
    }
    return _pointer_type();
  }

  _pointer_type createEntry(const Key& key)
  {
    _pointer_type created;
    _pointer_type result;
    try
    {
      created = _pointer_type(this->_factory.create(key, 0)._ptr);
      result = tryInsertMap(key, created);
      this->queueDiskPut(key, false);
    }
    catch (...)
    {
      if (created)
      {
        boost::unique_lock<boost::shared_mutex> lock(_mutex);
        this->moveToAccumulator(created.get());
      }
      throw;
    }
    return result;
  }

  _pointer_type tryInsertMap(const Key& key,
                             _pointer_type& object)
  {
    boost::unique_lock<boost::shared_mutex> lock(_mutex);
    auto pr(_map.emplace(key, object));
    if (!pr.second)
    {
      if (_uninitializedCacheEntry != pr.first->second)
      {
        this->moveToAccumulator(pr.first->second.get());
      }
      pr.first->second = object;
    }
    _condition.notify_all();
    return object;
  }

public:
  GenericCache(KeyedFactory<Key, Type>& factory,
               const std::string& name,
               size_t,
               size_t version)
    : Cache<Key, Type>(factory, "GenericCache", name, version)
    , _uninitializedCacheEntry(new Type)
  {
    this->_cacheDeleter.setCachePtr(this);
  }

  virtual size_t size() override
  {
    boost::shared_lock<boost::shared_mutex> lock(_mutex);
    return _map.size();
  }

  virtual _pointer_type getIfResident(const Key& key) override
  {
    return get(key, false);
  }

  virtual _pointer_type get(const Key& key) override
  {
    return get(key, true);
  }

  virtual void put(const Key& key,
                   Type* ptr,
                   bool updateLDC = true) override
  {
    if (ptr)
    {
      _pointer_type object(ptr);
      boost::unique_lock<boost::shared_mutex> lock(_mutex);
      auto pr(_map.emplace(key, _pointer_type()));
      if (!pr.second && pr.first->second != _uninitializedCacheEntry)
      {
        this->moveToAccumulator(pr.first->second.get());
      }
      pr.first->second = object;
      _condition.notify_all();
    }
    if (updateLDC)
    {
      this->queueDiskPut(key, true);
    }
  }

  virtual size_t invalidate(const Key& key) override
  {
    bool removed(false);
    {
      boost::upgrade_lock<boost::shared_mutex> lock(_mutex);
      auto mit(_map.find(key));
      if (mit != _map.end())
      {
        boost::upgrade_to_unique_lock<boost::shared_mutex> uniqueLock(lock);
        if (mit->second != _uninitializedCacheEntry)
        {
          this->moveToAccumulator(mit->second.get());
        }
        _map.erase(mit);
        removed = true;
        _condition.notify_all();
      }
    }
    bool distCacheOp(true);
    this->queueDiskInvalidate(key, distCacheOp, removed);
    return removed ? 1 : 0;
  }

  virtual std::shared_ptr<std::vector<Key>> keys() override
  {
    std::shared_ptr<std::vector<Key>> allKeys(new std::vector<Key>);
    {
      boost::shared_lock<boost::shared_mutex> lock(_mutex);
      allKeys->reserve(_map.size());
      for (const auto& pair : _map)
      {
        allKeys->push_back(pair.first);
      }
    }
    return allKeys;
  }

  virtual size_t clear() override
  {
    size_t result(0);
    {
      boost::unique_lock<boost::shared_mutex> lock(_mutex);
      result = _map.size();
      while (!_map.empty())
      {
        auto mit(_map.begin());
        if (mit->second != _uninitializedCacheEntry)
        {
          this->moveToAccumulator(mit->second.get());
        }
        _map.erase(mit);
      }
      _condition.notify_all();
    }
    this->queueDiskClear();
    return result;
  }

  virtual void emptyTrash() override
  {
    boost::unique_lock<boost::shared_mutex> lock(_mutex);
    if (!this->_accumulator.empty())
    {
      this->_cacheDeleter.moveToTrashBin(this->_accumulator);
      this->_accumulator.reserve(this->_accumulatorSize);
    }
  }

};// class GenericCache

}// sfc
