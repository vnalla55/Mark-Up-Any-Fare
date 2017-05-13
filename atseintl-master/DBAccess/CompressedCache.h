#pragma once

#include "DBAccess/Cache.h"
#include "DBAccess/CompressedData.h"
#include "DBAccess/PointerDeleter.h"

#include <condition_variable>
#include <memory>
#include <mutex>
#include <unordered_map>

//./appconsole.pl atsedbld05b 5432 CCSTATS FARE

namespace sfc
{
template <typename Key, typename Type>
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
    RemovedKey& operator=(const Key& key)
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
    size_t operator()(const Key& key) const
    {
      size_t hash(0);
      tse::hashCombine(hash, key);
      return hash;
    }
  };
  typedef typename Cache<Key, Type>::pointer_type _pointer_type;
  _pointer_type _uninitializedCacheEntry;
  size_t _capacity;

  typedef std::list<Key> Keys;

  struct CacheEntry
  {
    CacheEntry(Keys& uckeys,
               Keys& ckeys)
      : _uncompressedKeyIterator(uckeys.end())
      , _compressedKeyIterator(ckeys.end())
      , _compressed(nullptr)
    {
    }
    typename Keys::iterator _uncompressedKeyIterator;
    typename Keys::iterator _compressedKeyIterator;
    _pointer_type _inflated;
    CompressedData* _compressed;
  };

  typedef std::unordered_map<Key, CacheEntry, hash_func> Map;

  size_t _keysSize;
  size_t _notEmptySize;
  Map _map;
  Keys _uncompressedKeys;
  Keys _compressedKeys;
  std::mutex _mutex;
  std::condition_variable _condition;
  PointerDeleter<CompressedData> _compressedDeleter;

  void deleteCompressed(CompressedData* compressed)
  {
    if (compressed)
    {
      _compressedDeleter.del(compressed);
    }
  }

  void insertUninitialized(const Key& key)
  {
    CacheEntry entry(_uncompressedKeys, _compressedKeys);
    entry._inflated = _uninitializedCacheEntry;
    _map.emplace(key, entry);
  }

  void insertUncompressed(const typename Map::iterator& uit)
  {
    if (_map.end() == uit || nullptr == uit->second._compressed)
    {
      return;
    }
    for (typename Keys::iterator kit(_uncompressedKeys.begin());
         kit != _uncompressedKeys.end() && _keysSize >= _capacity;
         )
    {
      typename Map::iterator mit(_map.find(*kit));
      if (mit != _map.end()
          && mit->second._compressed
          && mit->second._inflated
          && mit->second._inflated != _uninitializedCacheEntry)
      {
        this->moveToAccumulator(mit->second._inflated.get());
        mit->second._inflated = _pointer_type();
        if (this->_totalCapacity > 0)
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
    if (!_compressedKeys.empty()
        && this->_totalCapacity > 0
        && _notEmptySize > this->_totalCapacity)
    {
      for (typename Keys::iterator kit(_compressedKeys.begin());
           kit != _compressedKeys.end();
           ++kit)
      {
        typename Map::iterator mit(_map.find(*kit));
        if (mit != _map.end() && mit->second._compressed)
        {
          if (mit->second._inflated && mit->second._inflated != _uninitializedCacheEntry)
          {
            this->moveToAccumulator(mit->second._inflated.get());
          }
          removedKey = *kit;
          _compressedKeys.erase(kit);
          if (mit->second._compressed)
          {
            --_notEmptySize;
            deleteCompressed(mit->second._compressed);
          }
          _map.erase(mit);
          break;
        }
      }
    }
  }

  _pointer_type get(const Key& key,
                    bool create)
  {
    const MallocContextDisabler disableCustomAllocator;
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
              if (mit->second._uncompressedKeyIterator != _uncompressedKeys.end())
              {
                _uncompressedKeys.splice(_uncompressedKeys.end(),
                                         _uncompressedKeys,
                                         mit->second._uncompressedKeyIterator);
                mit->second._uncompressedKeyIterator = --_uncompressedKeys.end();
              }
              return mit->second._inflated;
            }
            else// another thread put _uninitializedCacheEntry
            {
              do
              {
                _condition.wait(lock);
                inMap = (mit = _map.find(key)) != _map.end();
                if (inMap
                    && mit->second._inflated
                    && mit->second._inflated != _uninitializedCacheEntry)
                {
                  return mit->second._inflated;
                }
              } while (inMap && _uninitializedCacheEntry == mit->second._inflated);
            }
          }
          if (inMap
              && !mit->second._inflated
              && mit->second._compressed
              && mit->second._compressed->empty())
          {
            do
            {
              _condition.wait(lock);
              inMap = (mit = _map.find(key)) != _map.end();
              if (inMap && mit->second._inflated
                  && mit->second._inflated != _uninitializedCacheEntry)
              {
                return mit->second._inflated;
              }
            } while (inMap && mit->second._compressed && mit->second._compressed->empty());
          }
          if (inMap
              && !mit->second._inflated
              && mit->second._compressed
              && !mit->second._compressed->empty())
          {
            mit->second._compressed->swap(compressed);
          }
        }
        if (create && !inMap)
        {
          insertUninitialized(key);
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
            else if (mit->second._compressed != nullptr && mit->second._compressed->empty())
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
      CompressedData* compressed(nullptr);
      if (result._compressed)
      {
        compressed = new CompressedData;
        result._compressed->swap(*compressed);
      }
      RemovedKey removedKey(*this);
      result = tryInsertMap(key, created, removedKey, compressed);
      _condition.notify_all();
      this->queueDiskPut(key, false);
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

  CompressedData* compressEntry(_pointer_type ptr)
  {
    return this->_factory.compress(ptr.get());
  }

  bool remove(const typename Map::iterator& mit)
  {
    bool removed(false);
    if (mit != _map.end())
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
      if (mit->second._inflated && mit->second._inflated != _uninitializedCacheEntry)
      {
        this->moveToAccumulator(mit->second._inflated.get());
      }
      if (mit->second._compressed)
      {
        --_notEmptySize;
        deleteCompressed(mit->second._compressed);
      }
      _map.erase(mit);
      removed = true;
    }
    return removed;
  }

  tse::CreateResult<Type> tryInsertMap(const Key& key,
                                       _pointer_type& object,
                                       RemovedKey& removedKey,
                                       CompressedData* compressedIn)
  {
    tse::CreateResult<Type> result;
    CompressedData* compressed(compressedIn);
    if (object)
    {
      if (nullptr == compressed)
      {
        compressed = compressEntry(object);
      }
      std::unique_lock<std::mutex> lock(_mutex);
      CacheEntry entry(_uncompressedKeys, _compressedKeys);
      std::pair<typename Map::iterator, bool> pr(_map.emplace(key, entry));
      result._ptr = object.get();
      if (nullptr == compressed)
      {
        result._status = tse::RemoteCache::RC_UNCOMPRESSED_VALUE;
      }
      else
      {
        result._status = tse::RemoteCache::RC_COMPRESSED_VALUE;
        // create shared_ptr under lock
        result._compressed = CompressedDataPtr(
            compressed, DeleterFunc<CompressedData>(compressed, _compressedDeleter));
      }
      if (pr.second)
      {
        pr.first->second._inflated = object;
        object = _pointer_type();
        pr.first->second._compressed = compressed;
        if (compressed)
        {
          ++_notEmptySize;
          insertUncompressed(pr.first);
          controlMapSize(removedKey);
        }
      }
      else
      {
        bool uninitialized(_uninitializedCacheEntry == pr.first->second._inflated);
        if (pr.first->second._inflated && !uninitialized)
        {
          this->moveToAccumulator(pr.first->second._inflated.get());
        }
        pr.first->second._inflated = _pointer_type();
        if (pr.first->second._compressed)
        {
          --_notEmptySize;
          deleteCompressed(pr.first->second._compressed);
          pr.first->second._compressed = nullptr;
        }
        if (compressed)
        {
          ++_notEmptySize;
          pr.first->second._compressed = compressed;
        }
        if (uninitialized && compressed)
        {
          insertUncompressed(pr.first);
          controlMapSize(removedKey);
        }
        if (pr.first->second._uncompressedKeyIterator != _uncompressedKeys.end()
            || nullptr == compressed)
        {
          pr.first->second._inflated = object;
          object = _pointer_type();
        }
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
    : Cache<Key, Type>(factory, "CompressedCache", name, version)
    , _uninitializedCacheEntry(new Type)
    , _capacity(capacity)
    , _keysSize(0)
    , _notEmptySize(0)
  {
    this->_cacheDeleter.setCachePtr(this);
  }

  virtual ~CompressedCache() {}

  virtual size_t size() override { return _notEmptySize; }
  // put into interface
  virtual size_t uncompressedSize()
  {
    std::unique_lock<std::mutex> lock(_mutex);
    return _keysSize;
  }

  void reserve(size_t capacity)
  {
    std::unique_lock<std::mutex> lock(_mutex);
    _capacity = capacity;
  }

  virtual _pointer_type getIfResident(const Key& key) override { return get(key, false); }

  virtual _pointer_type get(const Key& key) override { return get(key, true); }

  virtual void put(const Key& key, Type* object, bool updateLDC = true) override
  {
    put(key, _pointer_type(object), updateLDC);
  }

  void put(const Key& key,
           _pointer_type object,
           bool updateLDC = true)
  {
    if (object)
    {
      RemovedKey removedKey(*this);
      CompressedData* compressed(compressEntry(object));
      std::unique_lock<std::mutex> lock(_mutex);
      CacheEntry entry(_uncompressedKeys, _compressedKeys);
      std::pair<typename Map::iterator, bool> pr(_map.emplace(key, entry));
      if (pr.second)
      {
        pr.first->second._compressed = compressed;
        if (compressed)
        {
          if (this->_totalCapacity > 0)
          {
            _compressedKeys.push_back(pr.first->first);
            pr.first->second._compressedKeyIterator = --_compressedKeys.end();
          }
          ++_notEmptySize;
          controlMapSize(removedKey);
        }
        else
        {
          pr.first->second._inflated = object;
          object = _pointer_type();
        }
      }
      else
      {
        if (pr.first->second._inflated
            && pr.first->second._inflated != _uninitializedCacheEntry)
        {
          this->moveToAccumulator(pr.first->second._inflated.get());
        }
        pr.first->second._inflated = _pointer_type();
        if (pr.first->second._compressed)
        {
          deleteCompressed(pr.first->second._compressed);
          --_notEmptySize;
          pr.first->second._compressed = nullptr;
        }
        if (compressed)
        {
          pr.first->second._compressed = compressed;
          ++_notEmptySize;
        }
        if (pr.first->second._uncompressedKeyIterator != _uncompressedKeys.end()
            || nullptr == compressed)
        {
          pr.first->second._inflated = object;
          object = _pointer_type();
        }
      }
      if (object)
      {
        this->moveToAccumulator(object.get());
      }
    }
    _condition.notify_all();
    if (updateLDC)
    {
      this->queueDiskPut(key, true);
    }
  }

  virtual size_t invalidate(const Key& key) override
  {
    bool removed(false);
    {
      std::unique_lock<std::mutex> lock(_mutex);
      typename Map::iterator mit(_map.find(key));
      removed = remove(mit);
    }
    if (removed)
    {
      _condition.notify_all();
    }
    bool distCacheOp(true);
    this->queueDiskInvalidate(key, distCacheOp, removed);
    return removed ? 1 : 0;
  }

  virtual std::shared_ptr<std::vector<Key>> keys() override
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

  virtual size_t clear() override
  {
    size_t result(0);
    {
      std::unique_lock<std::mutex> lock(_mutex);
      result = _map.size();
      while (!_map.empty())
      {
        typename Map::iterator mit(_map.begin());
        if (mit->second._inflated && mit->second._inflated != _uninitializedCacheEntry)
        {
          this->moveToAccumulator(mit->second._inflated.get());
        }
        deleteCompressed(mit->second._compressed);
        _map.erase(mit);
      }
      _uncompressedKeys.clear();
      _notEmptySize = 0;
      _compressedKeys.clear();
      _keysSize = 0;
    }
    _condition.notify_all();
    this->queueDiskClear();
    return result;
  }

  virtual CompressedDataPtr getCompressed(const Key& key,
                                          tse::RemoteCache::RCStatus& status,
                                          bool& fromQuery) override
  {
    CompressedDataPtr result;
    const MallocContextDisabler disableCustomAllocator;
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
          if (nullptr == compressed)
          {
            status = tse::RemoteCache::RC_UNCOMPRESSED_VALUE;
          }
          else
          {
            status = tse::RemoteCache::RC_COMPRESSED_VALUE;
            // create shared_ptr under lock
            result = CompressedDataPtr(compressed,
                                       DeleterFunc<CompressedData>(compressed, _compressedDeleter));
          }
          return result;
        }
        else// not in map
        {
          insertUninitialized(key);
        }
      }
      tse::CreateResult<Type> createResult(createEntry(key));
      status = createResult._status;
      fromQuery = true;
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

  virtual void emptyTrash() override
  {
    std::unique_lock<std::mutex> lock(_mutex);
    if (!this->_accumulator.empty())
    {
      this->_cacheDeleter.moveToTrashBin(this->_accumulator);
      this->_accumulator.reserve(this->_accumulatorSize);
    }
  }

  virtual void disposeCache(double fraction) override
  {
    std::unique_lock<std::mutex> lock(_mutex);
    size_t compressedSize(_compressedKeys.size());
    size_t numberToRemove(compressedSize * fraction);
    size_t count(0);
    while (count < numberToRemove)
    {
      typename Keys::iterator kit(_compressedKeys.begin());
      if (kit != _compressedKeys.end())
      {
        typename Map::iterator mit(_map.find(*kit));
        if (mit != _map.end())
        {
          remove(mit);
          ++count;
          this->queueDiskInvalidate(*kit, true, true);
        }
        else
        {
          break;
        }
      }
    }
  }

  virtual void getCompressionStats(CompressedCacheStats& stats) override
  {
    std::unique_lock<std::mutex> lock(_mutex);
    stats._totalSize = _notEmptySize;
    stats._totalCapacity = this->_totalCapacity;
    stats._uncompressedSize = _uncompressedKeys.size();
    stats._uncompressedCapacity = _capacity;
    if (_keysSize != stats._uncompressedSize)
    {
      stats._errors.append("_keysSize != _uncompressedKeys.size()\n");
    }
    stats._compressedSize = 0 == this->_totalCapacity ? stats._totalSize - stats._uncompressedSize
                                                      : _compressedKeys.size();
    stats._threshold = this->_threshold;
    size_t compressedBytes(0);
    size_t uncompressedBytes(0);
    size_t numberCompressed(0);
    for (typename Map::const_iterator mit(_map.begin()), mitend(_map.end());
         mit != mitend;
         ++mit)
    {
      if (mit->second._inflated
          && mit->second._compressed
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
          && mit->second._compressed
          && _uncompressedKeys.end() == mit->second._uncompressedKeyIterator
          && _compressedKeys.end() == mit->second._compressedKeyIterator)
      {
        stats._errors.append("both iterators are invalid for not empty\n");
      }
      if (nullptr == mit->second._compressed
          && (_uncompressedKeys.end() != mit->second._uncompressedKeyIterator
              || _compressedKeys.end() != mit->second._compressedKeyIterator))
      {
        stats._errors.append("at least one iterator is valid for empty\n");
      }
      if (mit->second._compressed)
      {
        uncompressedBytes += mit->second._compressed->_inflatedSz;
        compressedBytes += mit->second._compressed->_deflated.size();
        ++numberCompressed;
      }
      if (nullptr == mit->second._compressed
          && mit->second._inflated != _uninitializedCacheEntry)
      {
        ++stats._numberEmpty;
      }
      if (mit->second._compressed)
      {
        if (mit->second._inflated && mit->second._inflated != _uninitializedCacheEntry)
        {
          stats._memoryEstimate += mit->second._compressed->_inflatedSz;
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
      stats._averageRatio =
          static_cast<double>(uncompressedBytes) / static_cast<double>(compressedBytes);
    }
    if (stats._numberEmpty + _notEmptySize != _map.size())
    {
      std::ostringstream oss;
      oss << "empty(" << stats._numberEmpty
          << ")+notEmpty(" << _notEmptySize
          << ")!=mapSize(" << _map.size() << ")\n";
      stats._errors.append(oss.str());
    }
  }

};// class CompressedCache

}// sfc
