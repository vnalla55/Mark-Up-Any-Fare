#ifndef CACHE_H
#define CACHE_H
#include "Allocator/TrxMalloc.h"
#include "Common/KeyedFactory.h"
#include "DataModel/SerializationTrx.h"
#include "DBAccess/CacheDeleter.h"
#include "DBAccess/DataBlobHelper.h"
#include "DBAccess/DiskCache.h"
#include "DBAccess/DistCache.h"
#include "DBAccess/DistCachePool.h"
#include "DBAccess/LDCActionQueue.h"
#include "DBAccess/raw_ptr.h"
#include "DBAccess/RemoteCache/RemoteCacheHeader.h"

#include <vector>

#include <log4cxx/helpers/objectptr.h>
#include <memory>

namespace sfc
{
struct CompressedCacheStats
{
  CompressedCacheStats()
    : _totalSize(0),
      _totalCapacity(0),
      _uncompressedSize(0),
      _uncompressedCapacity(0),
      _compressedSize(0),
      _averageCompressedBytes(0),
      _numberEmpty(0),
      _memoryEstimate(0),
      _averageRatio(0),
      _threshold(0)
  {
  }
  size_t _totalSize;
  size_t _totalCapacity;
  size_t _uncompressedSize;
  size_t _uncompressedCapacity;
  size_t _compressedSize;
  size_t _averageCompressedBytes;
  size_t _numberEmpty;
  size_t _memoryEstimate;
  double _averageRatio;
  size_t _threshold;
  std::string _errors;
};

template <class Key, class Type, class FactoryType>
class Cache
{
protected:
  FactoryType& _factory;
  size_t _accumulatorSize;
  typename tse::CacheDeleter<Key, Type> _cacheDeleter;
  typedef std::vector<Type*> TrashBin;
  TrashBin _accumulator;
  size_t _totalCapacity;
  size_t _threshold;

public:
  typedef Key key_type;
  typedef Type value_type;
#ifdef _USERAWPOINTERS
  typedef RawPtr<Type> pointer_type;
#else
  typedef std::shared_ptr<Type> pointer_type;
#endif // _USERAWPOINTERS
  typedef typename std::shared_ptr<std::vector<Key>> key_vector_type;

  Cache(FactoryType& factory, const std::string& type, const std::string& name, size_t version)
    : _factory(factory),
      _accumulatorSize(1000),
      _cacheDeleter(factory),
      _totalCapacity(0),
      _threshold(0),
      _type(type),
      _name(name),
      _isLoading(false),
      _ldcQueue(name),
      _version(version),
      _cto(nullptr)
  {
  }

  virtual ~Cache() {}

  virtual size_t size() = 0;

  virtual size_t clear() = 0;

  virtual pointer_type get(const Key& key) = 0;

  virtual pointer_type getIfResident(const Key& key) = 0;

  virtual CompressedDataPtr getCompressed(const Key& key,
                                          tse::RemoteCache::RCStatus& status,
                                          bool& fromQuery)
  {
    CompressedDataPtr result;
    tse::SerializationTrx trx;
    pointer_type ptr(getIfResident(key));
    if (!ptr)
    {
      ptr = get(key);
      fromQuery = true;
    }
    result.reset(_factory.compress(ptr.get()));
    status = result ? tse::RemoteCache::RC_COMPRESSED_VALUE
                      : tse::RemoteCache::RC_UNCOMPRESSED_VALUE;
    return result;
  }

  virtual void put(const Key& key, Type* object, bool updateLDC) = 0;

  virtual size_t invalidate(const Key& key) = 0;

  virtual std::shared_ptr<std::vector<Key>> keys() = 0;

  virtual void emptyTrash() = 0;

  // Not all caches are required to suppoirt "consolidation", so
  //      provide default behavior for "consolitation" functions.

  // Function to fetch how many objects would be moved by a consolidation
  virtual size_t consolidationSize()
  {
    // No consolidation to be done for this cache, so no records to consolidate
    return 0;
  }

  // Function to perform a consolidation
  virtual size_t consolidate(size_t maxRecordsToConsolidate)
  {
    // No consolidation to be done for this cache

    return 0; // Number of objects consolidated
  }

  // Helpers

  tse::LDCActionQueue<Key>& ldcQueue() { return _ldcQueue; }

  virtual size_t actionQueueSize() const { return _ldcQueue.size(); }

  virtual void actionQueueClear() { _ldcQueue.clear(); }

  bool actionQueueNext(tse::LDCOperation<Key>& target) { return _ldcQueue.getNext(target); }

  const std::string& getName() const { return _name; }

  const std::string& getType() const { return _type; }

  void setName(const std::string& name) { _name = name; }

  void initForLDC(const std::string& name, tse::DiskCache::CacheTypeOptions* cto)
  {
    _ldcQueue.initialize(name, cto);
  }

  bool ldcEnabled() const { return _ldcQueue.ldcEnabled(); }

  bool isLoading() const { return _isLoading; }

  void setLoading(bool val = true) { _isLoading = val; }

  void queueDiskPut(const Key& key, bool distCacheOp)
  {
    if (_cto && ((_cto->useDistCache && distCacheOp) || _cto->enabled))
    {
      _ldcQueue.queueWriteReq(key, _isLoading, distCacheOp);
    }
  }

  void queueDiskInvalidate(const Key& key, bool distCacheOp, bool ldcOp)
  {
    _ldcQueue.queueRemoveReq(key, _isLoading, ldcOp);
    if (UNLIKELY(distCacheOp && _cto && _cto->useDistCache && DISKCACHE.getDistCacheEventMaster()))
    {
      tse::KeyStream stream(_name, static_cast<int>(_version));
      stream << key;
      DistCacheScopedLockQueueMemberPtr dist;
      dist->remove(stream, 0);
    }
  }

  void queueDiskClear() { _ldcQueue.queueClearReq(_isLoading); }

  void setCacheTypeOptions(tse::DiskCache::CacheTypeOptions* cto)
  {
    if (cto)
      _cto = cto;
  }

  void setAccumulatorSize(int accumulatorSize) { _accumulatorSize = accumulatorSize; }

  void moveToAccumulator(Type* ptr)
  {
    MallocContextDisabler mallocController;
    _accumulator.push_back(ptr);
    if (_accumulator.size() >= _accumulatorSize)
    {
      _cacheDeleter.moveToTrashBin(_accumulator);
      _accumulator.reserve(_accumulatorSize);
    }
  }

  virtual void getCompressionStats(CompressedCacheStats&) {}

  virtual void disposeCache(double fraction) {}

  size_t getTotalCapacity() const { return _totalCapacity; }

  void setTotalCapacity(size_t totalCapacity) { _totalCapacity = totalCapacity; }

  size_t getThreshold() const { return _threshold; }

  void setThreshold(size_t threshold) { _threshold = threshold; }

  virtual size_t tableVersion() const { return _version; }

  Type* getDistCache(const Key& key, bool& distCacheOp)
  {
    distCacheOp = false;
    Type* ptr(nullptr);
    if (UNLIKELY(_cto && _cto->useDistCache))
    {
      tse::MemcachedReadPool dist;
      if (dist)
      {
        tse::KeyStream stream(_name, static_cast<int>(_version));
        stream << key;
#ifdef _DEBUGFLATKEY
        std::ostringstream os;
        os << _name << '.' << _version << '/' << key;
        const std::string& distKey(os.str());
        if (strcmp(stream, distKey.c_str()))
        {
          std::cerr << "distKey  :" << distKey << std::endl;
          std::cerr << "KeyStream:" << stream << std::endl;
        }
#endif // _DEBUGFLATKEY
        char* text(nullptr);
        size_t length(0);
        bool found(dist->get(stream, static_cast<uint32_t>(stream.size()), &text, &length));
        dist.release();
        distCacheOp = !found;
        if (found)
        {
          ptr = tse::DataBlobHelper<Key, Type>::deserialize(text, length, stream, _name, *_cto);
          delete[] text;
        }
      }
    }
    return ptr;
  }

private:
  std::string _type;
  std::string _name;
  bool _isLoading;

  tse::LDCActionQueue<Key> _ldcQueue;

  const size_t _version;
  tse::DiskCache::CacheTypeOptions* _cto;

  Cache();
  Cache(const Cache& rhs);
  Cache& operator=(const Cache& rhs);
};

} // namespace sfc

#endif // CACHE_H
