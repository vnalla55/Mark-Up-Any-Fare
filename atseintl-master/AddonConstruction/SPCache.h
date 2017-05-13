#pragma once
#include "Common/KeyedFactory.h"
#include "DBAccess/DataBlobHelper.h"
#include "DBAccess/DiskCache.h"
#include "DBAccess/DistCache.h"
#include "DBAccess/DistCachePool.h"
#include "DBAccess/LDCActionQueue.h"

#include <memory>
#include <vector>


namespace sfc
{
using tse::DiskCache;
using tse::DataBlobHelper;

template <class Key, class Type, class FactoryType = KeyedFactory<Key, Type>>
class SPCache
{
protected:
  FactoryType& _factory;

public:
  typedef Key key_type;
  typedef Type value_type;
  typedef typename std::shared_ptr<Type> pointer_type;
  typedef typename std::shared_ptr<std::vector<Key>> key_vector_type;

  SPCache(FactoryType& factory,
          const std::string& type,
          const std::string& name,
          size_t version,
          bool isLoading = false)
    : _factory(factory),
      _type(type),
      _name(name),
      _version(version),
      _isLoading(isLoading),
      _ldcQueue(name),
      _cto(nullptr)
  {
  }

  virtual ~SPCache() {}

  virtual size_t size() = 0;

  virtual size_t clear() = 0;

  virtual pointer_type get(const Key& key) = 0;

  virtual pointer_type getIfResident(const Key& key) = 0;

  virtual void put(const Key& key, Type* object, bool updateLDC) = 0;

  virtual size_t invalidate(const Key& key) = 0;

  virtual std::shared_ptr<std::vector<Key>> keys() = 0;

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

  const std::string& getName() { return _name; }

  const std::string& getType() { return _type; }

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
    _ldcQueue.queueWriteReq(key, _isLoading, distCacheOp);
  }

  void queueDiskInvalidate(const Key& key, bool distCacheOp, bool ldcOp)
  {
    _ldcQueue.queueRemoveReq(key, _isLoading, ldcOp);
    if (distCacheOp && _cto && _cto->useDistCache && DISKCACHE.getDistCacheEventMaster())
    {
      tse::KeyStream stream(_name, _version);
      stream << key;
      DistCacheScopedLockQueueMemberPtr dist;
      dist->remove(stream, 0);
    }
  }

  void queueDiskClear() { _ldcQueue.queueClearReq(_isLoading); }

  void setCacheTypeOptions(tse::DiskCache::CacheTypeOptions* cto) { _cto = cto; }

  tse::DiskCache::CacheTypeOptions* getCacheTypeOptions() { return _cto; }

  virtual size_t tableVersion() const { return _version; }

  Type* getDistCache(const Key& key, bool& distCacheOp)
  {
    Type* ptr(nullptr);
    if (_cto && _cto->useDistCache)
    {
      tse::MemcachedReadPool dist;
      if (dist)
      {
        tse::KeyStream stream(_name, _version);
        stream << key;
        char* text(nullptr);
        size_t length(0);
        bool found(dist->get(stream, stream.size(), &text, &length));
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
  size_t _version;
  bool _isLoading;

  tse::LDCActionQueue<Key> _ldcQueue;

  tse::DiskCache::CacheTypeOptions* _cto;

  SPCache();
  SPCache(const SPCache& rhs);
  SPCache& operator=(const SPCache& rhs);
};

} // namespace sfc

