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
#include "Cache.h"
#include "CompressedDataUtils.h"
#include "PointerDeleter.h"

#include <condition_variable>
#include <mutex>

//./appconsole.pl atsedbld05b 5432 CCSTATS FARE

namespace sfc
{
  template<typename Key, typename Type>
  class CompressedCache : public Cache<Key, Type>
  {
  protected:
    class RemovedKey : boost::noncopyable
    {
    public:
      explicit RemovedKey(Cache<Key, Type>& cache)
        : _initialized(false)
        , _cache(cache)
      {
      }
      ~RemovedKey()
      {
        if (_initialized)
        {
          _cache.queueDiskInvalidate(_key, true, false);
        }
      }
      RemovedKey& operator = (const Key& key)
      {
        _key = key;
        _initialized = true;
        return *this;
      }
    private:
      Key _key;
      bool _initialized;
      Cache<Key, Type>& _cache;
    };

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
    size_t _capacity;

    typedef std::list<Key> Keys;

    struct CacheEntry
    {
      CacheEntry(Keys& uckeys,
                 Keys& ckeys)
        : _uncompressedKeyIterator(uckeys.end())
        , _compressedKeyIterator(ckeys.end())
        , _compressed(0)
      {
      }
      typename Keys::iterator _uncompressedKeyIterator;
      typename Keys::iterator _compressedKeyIterator;
      _pointer_type _inflated;
      CompressedData* _compressed;
    };

    typedef boost::unordered_map<Key, CacheEntry, hash_func> Map;

    size_t _keysSize;
    Map _map;
    Keys _uncompressedKeys;
    Keys _compressedKeys;
    std::mutex _mutex;
    std::condition_variable _condition;
    PointerDeleter<CompressedData> _compressedDeleter;

    void deleteCompressed(CompressedData* compressed)
    {
      _compressedDeleter.del(compressed);
    }

    void insertUninitialized(const Key& key,
                             RemovedKey& removedKey)
    {
      CacheEntry entry(_uncompressedKeys, _compressedKeys);
      entry._inflated = _uninitializedCacheEntry;
      std::pair<typename Map::iterator, bool> pr(_map.emplace(key, entry));
      if (pr.second)
      {
        insertUncompressed(pr.first);
        controlMapSize(removedKey);
      }
    }

    void insertUncompressed(const typename Map::iterator& uit)
    {
      typename Keys::iterator kit(_uncompressedKeys.begin());
      typename Map::iterator mitend(_map.end());
      while (kit != _uncompressedKeys.end() && _keysSize >= _capacity)
      {
        typename Map::iterator mit(_map.find(*kit));
        if (mit != mitend
            && mit->second._inflated
            && mit->second._inflated != _uninitializedCacheEntry)
        {
          if (0 == mit->second._compressed)
          {
            mit->second._compressed = compressEntry(mit->second._inflated);
          }
          if (mit->second._compressed != 0)
          {
            this->moveToAccumulator(mit->second._inflated.get());
            mit->second._inflated = _pointer_type();
          }
          if (this->_totalCapacity != 0)
          {
            _compressedKeys.push_back(*kit);
            mit->second._compressedKeyIterator = --_compressedKeys.end();
          }
          kit = _uncompressedKeys.erase(kit);
          --_keysSize;
          mit->second._uncompressedKeyIterator = _uncompressedKeys.end();
        }
        else
        {
          ++kit;
        }
      }
      if (uit->second._compressedKeyIterator != _compressedKeys.end())
      {
        _compressedKeys.erase(uit->second._compressedKeyIterator);
        uit->second._compressedKeyIterator = _compressedKeys.end();
      }
      _uncompressedKeys.push_back(uit->first);
      ++_keysSize;
      uit->second._uncompressedKeyIterator = --_uncompressedKeys.end();
    }

    void controlMapSize(RemovedKey& removedKey)
    {
      if (!_compressedKeys.empty() && _map.size() > this->_totalCapacity)
      {
        typename Keys::iterator kit(_compressedKeys.begin());
        typename Map::iterator mit(_map.find(*kit));
        if (mit != _map.end())
        {
          if (mit->second._inflated
              && mit->second._inflated != _uninitializedCacheEntry)
          {
            this->moveToAccumulator(mit->second._inflated.get());
          }
          removedKey = *kit;
          _compressedKeys.erase(kit);
          deleteCompressed(mit->second._compressed);
          _map.erase(mit);
        }
      }
    }

    _pointer_type get(const Key& key,
                      bool create)
    {
      MallocContextDisabler disableCustomAllocator;
      try
      {
        CompressedData compressed;
        {
          RemovedKey removedKey(*this);
          std::unique_lock<std::mutex> lock(_mutex);
          typename Map::iterator mit(_map.find(key));
          bool inMap(mit != _map.end());
          if (inMap)
          {
            if (mit->second._inflated)
            {
              if (mit->second._inflated != _uninitializedCacheEntry)
              {
                if (mit->second._compressed)
                {
                if (mit->second._uncompressedKeyIterator != _uncompressedKeys.end())
                {
                  _uncompressedKeys.splice(_uncompressedKeys.end(), _uncompressedKeys, mit->second._uncompressedKeyIterator);
                  mit->second._uncompressedKeyIterator = --_uncompressedKeys.end();
                }
                else
                {
                  insertUncompressed(mit);                  
                }
                }
                return mit->second._inflated;
              }
              else// another thread put _uninitializedCacheEntry
              {
                do
                {
                  _condition.wait(lock);
                  inMap = (mit = _map.find(key)) != _map.end();
                  if (inMap && mit->second._inflated && mit->second._inflated != _uninitializedCacheEntry)
                  {
                    return mit->second._inflated;
                  }
                }
                while (inMap && _uninitializedCacheEntry == mit->second._inflated);
              }
            }
            if (inMap && !mit->second._inflated && mit->second._compressed != 0 && mit->second._compressed->empty())
            {
              do
              {
                _condition.wait(lock);
                inMap = (mit = _map.find(key)) != _map.end();
                if (inMap && mit->second._inflated && mit->second._inflated != _uninitializedCacheEntry)
                {
                  return mit->second._inflated;
                }
              }
              while (inMap && mit->second._compressed != 0 && mit->second._compressed->empty());
            }
            if (inMap && !mit->second._inflated && mit->second._compressed != 0 && !mit->second._compressed->empty())
            {
              mit->second._compressed->swap(compressed);
            }
          }
          if (create && !inMap)
          {
            insertUninitialized(key, removedKey);
          }
        }
        if (!compressed.empty())
        {
          _pointer_type restored(this->_factory.uncompress(compressed));
          _pointer_type ret;
          if (restored)
          {
            std::unique_lock<std::mutex> lock(_mutex);
            typename Map::iterator mit(_map.find(key));
            if (mit != _map.end())
            {
              if (mit->second._inflated)
              {
                if (_uninitializedCacheEntry != mit->second._inflated)
                {
                  ret = mit->second._inflated;
                }
              }
              else if (mit->second._compressed != 0 && mit->second._compressed->empty())
              {
                mit->second._compressed->swap(compressed);
                ret = restored;
                if (_uncompressedKeys.end() == mit->second._uncompressedKeyIterator)
                {
                  mit->second._inflated = restored;
                  restored = _pointer_type();
                  insertUncompressed(mit);
                }
              }
            }
            if (restored)
            {
              this->moveToAccumulator(restored.get());
            }
          }
          _condition.notify_all();
          if (ret)
          {
            return ret;
          }
        }
        if (create)
        {
          return _pointer_type(createEntry(key)._ptr);
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

    tse::CreateResult<Type> createEntry(const Key& key)
    {
      _pointer_type created;
      tse::CreateResult<Type> result;
      try
      {
        result = this->_factory.create(key, 0);
        created = _pointer_type(result._ptr);
        CompressedData* compressed(0);
        if (result._compressed)
        {
          compressed = new CompressedData;
          result._compressed->swap(*compressed);
        }
        RemovedKey removedKey(*this);
        result = tryInsertMap(key, created, removedKey, compressed);
        _condition.notify_all();
        this->queueDiskPut(key, false);
        return result;
      }
      catch (...)
      {
        if (created)
        {
          std::unique_lock<std::mutex> lock(_mutex);
          this->moveToAccumulator(created.get());
        }
        throw;
      }
      return result;
    }

    CompressedData* compressEntry(_pointer_type ptr) const
    {
      return this->_factory.compress(ptr.get());
    }

    bool remove(const typename Map::iterator& mit)
    {
      if (mit->second._compressedKeyIterator != _compressedKeys.end())
      {
        _compressedKeys.erase(mit->second._compressedKeyIterator);
      }
      if (mit->second._uncompressedKeyIterator != _uncompressedKeys.end())
      {
        _uncompressedKeys.erase(mit->second._uncompressedKeyIterator);
        --_keysSize;
      }
      if (mit->second._inflated
          && mit->second._inflated != _uninitializedCacheEntry)
      {
        this->moveToAccumulator(mit->second._inflated.get());
      }
      deleteCompressed(mit->second._compressed);
      _map.erase(mit);
      return true;
    }

    tse::CreateResult<Type> tryInsertMap(const Key& key,
                                         _pointer_type& object,
                                         RemovedKey& removedKey,
                                         CompressedData* compressedIn = 0)
    {
      tse::CreateResult<Type> result;
      CompressedData* compressed(compressedIn);
      if (object)
      {
        if (0 == compressed)
        {
          compressed = compressEntry(object);
        }
        std::unique_lock<std::mutex> lock(_mutex);
        CacheEntry entry(_uncompressedKeys, _compressedKeys);
        std::pair<typename Map::iterator, bool> pr(_map.emplace(key, entry));
        result._ptr = object.get();
        if (0 == compressed)
        {
          result._status = tse::RemoteCache::UNCOMPRESSED_VALUE;
        }
        else
        {
          result._status = tse::RemoteCache::COMPRESSED_VALUE;
          // create shared_ptr under lock
          result._compressed = CompressedDataPtr(compressed,
                                                 DeleterFunc<CompressedData>(compressed, _compressedDeleter));
        }
        if (pr.second)
        {
          pr.first->second._inflated = object;
          object = _pointer_type();
          pr.first->second._compressed = compressed;
          insertUncompressed(pr.first);
          controlMapSize(removedKey);
        }
        else
        {
          if (pr.first->second._inflated && pr.first->second._inflated != _uninitializedCacheEntry)
          {
            this->moveToAccumulator(pr.first->second._inflated.get());
            pr.first->second._inflated = _pointer_type();
          }
          if (pr.first->second._uncompressedKeyIterator != _uncompressedKeys.end() || 0 == compressed)
          {
            pr.first->second._inflated = object;
            object = _pointer_type();
          }
          deleteCompressed(pr.first->second._compressed);
          pr.first->second._compressed = compressed;
        }
        if (object)
        {
          this->moveToAccumulator(object.get());
        }
      }
      return result;
    }
  public:
    CompressedCache(KeyedFactory<Key, Type>& factory,
                    const std::string& name,
                    size_t capacity,
                    size_t version)
      : Cache<Key, Type>( factory, "CompressedCache", name, version )
      , _uninitializedCacheEntry(new Type)
      , _capacity(capacity)
      , _keysSize(0)
    {
      this->_cacheDeleter.setCachePtr(this);
    }

    virtual ~CompressedCache()
    {
    }

    virtual size_t size()
    {
      std::unique_lock<std::mutex> lock(_mutex);
      return _map.size();
    }
    // put into interface
    virtual size_t uncompressedSize()
    {
      std::unique_lock<std::mutex> lock(_mutex);
      return _keysSize;
    }

    virtual void reserve(size_t capacity)
    {
      std::unique_lock<std::mutex> lock(_mutex);
      _capacity = capacity;
    }

    virtual _pointer_type getIfResident(const Key& key)
    {
      return get(key, false);
    }

    virtual _pointer_type get(const Key& key)
    {
      return get(key, true);
    }

    virtual void put(const Key& key,
                     Type* object,
                     bool updateLDC = true )
    {
      put(key, _pointer_type(object), updateLDC);
    }

    virtual void put(const Key& key,
                     _pointer_type object,
                     bool updateLDC = true )
    {
      RemovedKey removedKey(*this);
      tryInsertMap(key, object, removedKey);
      _condition.notify_all();
      if (updateLDC)
      {
        this->queueDiskPut(key, true);
      }
    }

    virtual void invalidate(const Key& key)
    {
      bool removed(false);
      {
        std::unique_lock<std::mutex> lock(_mutex);
        typename Map::iterator mit(_map.find(key));
        while (mit != _map.end()
               && (mit->second._inflated == _uninitializedCacheEntry
                   || (mit->second._compressed && mit->second._compressed->empty())))
        {
          _condition.wait(lock);
          mit = _map.find(key);
        }
        if (mit != _map.end())
        {
          removed = remove(mit);
        }
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
        std::unique_lock<std::mutex> lock(_mutex);
        allKeys->reserve(_map.size());
        for (typename Map::const_iterator it(_map.begin()), itend(_map.end()); it != itend; ++it)
        {
          allKeys->push_back(it->first);
        }
      }
      return allKeys;
    }

    virtual void clear()
    {
      {
        std::unique_lock<std::mutex> lock(_mutex);
        while (!_map.empty())
        {
          typename Map::iterator mit(_map.begin());
          if (mit->second._inflated
              && mit->second._inflated != _uninitializedCacheEntry)
          {
            this->moveToAccumulator(mit->second._inflated.get());
          }
          deleteCompressed(mit->second._compressed);
          _map.erase(mit);
        }
        _uncompressedKeys.clear();
        _compressedKeys.clear();
        _keysSize = 0;
      }
      _condition.notify_all();
      this->queueDiskClear() ;
    }

    virtual CompressedDataPtr getCompressed(const Key& key,
                                            tse::RemoteCache::StatusType& status)
    {
      CompressedDataPtr result;
      MallocContextDisabler disableCustomAllocator;
      try
      {
        {
          RemovedKey removedKey(*this);
          std::unique_lock<std::mutex> lock(_mutex);
          typename Map::const_iterator mit(_map.find(key));
          bool inMap(mit != _map.end());
          while (inMap && _uninitializedCacheEntry == mit->second._inflated)
          {
            _condition.wait(lock);
            inMap = (mit = _map.find(key)) != _map.end();
          }
          if (inMap)
          {
            CompressedData* compressed(mit->second._compressed);
            if (0 == compressed)
            {
              status = tse::RemoteCache::UNCOMPRESSED_VALUE;
            }
            else
            {
              status = tse::RemoteCache::COMPRESSED_VALUE;
              // create shared_ptr under lock
              result = CompressedDataPtr(compressed,
                                         DeleterFunc<CompressedData>(compressed, _compressedDeleter));
            }
            return result;
          }
          else// not in map
          {
            insertUninitialized(key, removedKey);
          }
        }
        tse::CreateResult<Type> createResult(createEntry(key));
        status = createResult._status;
        return createResult._compressed;
      }
      catch (...)
      {
        invalidate(key);
        _condition.notify_all();
        throw;
      }
      return result;
    }

    virtual void emptyTrash()
    {
      std::unique_lock<std::mutex> lock(_mutex);
      if (!this->_accumulator.empty())
      {
        this->_cacheDeleter.moveToTrashBin(this->_accumulator);
        this->_accumulator.reserve(this->_accumulatorSize);
      }
    }

    virtual void getCompressionStats(CompressedCacheStats& stats)
    {
      std::unique_lock<std::mutex> lock(_mutex);
      stats._totalSize = _map.size();
      stats._totalCapacity = this->_totalCapacity;
      stats._uncompressedSize = _uncompressedKeys.size();
      stats._uncompressedCapacity = _capacity;
      if (_keysSize != stats._uncompressedSize)
      {
        stats._errors.append("_keysSize != _uncompressedKeys.size()\n");
      }
      stats._compressedSize = 0 == this->_totalCapacity ? stats._totalSize - stats._uncompressedSize : _compressedKeys.size();
      stats._threshold = this->_threshold;
      size_t compressedBytes(0);
      size_t uncompressedBytes(0);
      size_t numberCompressed(0);
      for (typename Map::const_iterator mit(_map.begin()), mitend(_map.end()); mit != mitend; ++mit)
      {
        if (mit->second._inflated
            && mit->second._compressed != 0
            && _uncompressedKeys.end() == mit->second._uncompressedKeyIterator)
        {
          stats._errors.append("inflated is not empty in compressed\n");
        }
        if (_uncompressedKeys.end() != mit->second._uncompressedKeyIterator
            && _compressedKeys.end() != mit->second._compressedKeyIterator)
        {
          stats._errors.append("both iterators are valid\n");
        }
        if (this->_totalCapacity > 0
            && _uncompressedKeys.end() == mit->second._uncompressedKeyIterator
            && _compressedKeys.end() == mit->second._compressedKeyIterator)
        {
          stats._errors.append("both iterators are invalid\n");
        }
        if (mit->second._compressed != 0)
        {
          uncompressedBytes += tse::getUncompressedSize(mit->second._compressed);
          compressedBytes += mit->second._compressed->_deflated.size();
          ++numberCompressed;
        }
        if (0 == mit->second._compressed
            && mit->second._inflated
            && mit->second._inflated != _uninitializedCacheEntry)
        {
          ++stats._numberEmpty;
        }
        if (mit->second._compressed != 0)
        {
          if (mit->second._inflated
              && mit->second._inflated != _uninitializedCacheEntry)
          {
            stats._memoryEstimate += tse::getUncompressedSize(mit->second._compressed);
          }
          stats._memoryEstimate += mit->second._compressed->_deflated.size();
        }
      }
      if (numberCompressed != 0)
      {
        stats._averageCompressedBytes = compressedBytes / numberCompressed;
      }
      if (compressedBytes != 0)
      {
        stats._averageRatio = static_cast<double>(uncompressedBytes) / compressedBytes;
      }
    }
  };// class CompressedCache

}// namespace sfc
