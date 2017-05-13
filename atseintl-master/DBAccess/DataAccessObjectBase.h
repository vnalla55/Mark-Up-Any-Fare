//-------------------------------------------------------------------------------
// Copyright 2012, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#pragma once

#include "Common/CacheStats.h"
#include "Common/Config/ConfigMan.h"
#include "Common/Global.h"
#include "Common/KeyedFactory.h"
#include "DataModel/CacheUpdateAction.h"
#include "DBAccess/Cache.h"
#include "DBAccess/CacheControl.h"
#include "DBAccess/CacheFactory.h"
#include "DBAccess/CacheManager.h"
#include "DBAccess/CreateResult.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/DiskCache.h"
#include "DBAccess/LDCHelper.h"
#include "DBAccess/RemoteCache/ASIOClient/RCClient.h"
#include "DBAccess/RemoteCache/RCServerAttributes.h"
#include "DBAccess/RemoteCache/ReadConfig.h"

#include <boost/type_traits.hpp>

#include <algorithm>
#include <limits>
#include <map>
#include <vector>

#include <tr1/unordered_map>

namespace
{

typedef char (&no_tag)[1];
typedef char (&yes_tag)[2];

template <typename M>
struct has_capacity
{
  template <typename S, size_t (S::*)() const>
  struct dummyCapacity
  {
  };

  template <typename S>
  static yes_tag checkCapacity(dummyCapacity<S, &S::capacity>*);

  template <typename S>
  static no_tag checkCapacity(...);

  enum
  {
    value = (sizeof(checkCapacity<M>(nullptr)) == sizeof(yes_tag))
  };
};

template <typename M>
struct has_size
{
  template <typename S, size_t (S::*)() const>
  struct dummySize
  {
  };

  template <typename S>
  static yes_tag checkSize(dummySize<S, &S::size>*);

  template <typename S>
  static no_tag checkSize(...);

  enum
  {
    value = (sizeof(checkSize<M>(nullptr)) == sizeof(yes_tag))
  };
};

} // namespace anon

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{

Logger getCacheUpdateLogger();

template <typename T>
class IsEffectiveDH
{
public:
  IsEffectiveDH(const DateTime& startDate,
                const DateTime& endDate,
                const DateTime& ticketDate)
    : _startDate(startDate.date()),
      _endDate(endDate.date()),
      _ticketDate(ticketDate),
      _ticketDateNoTime(ticketDate.date())
  {
    init(ticketDate);
  }

  IsEffectiveDH(const DateTime& date,
                const DateTime& ticketDate)
    : _startDate(date.date()),
      _endDate(date.date()),
      _ticketDate(ticketDate),
      _ticketDateNoTime(ticketDate.date())
  {
    init(ticketDate);
  }

  bool operator()(const T* rec) const
  {
    if (rec->effDate() > _endDate)
      return false;
    if (_startDate > rec->discDate().date())
      return false;

    if (_ticketDate.historicalIncludesTime())
    {
      if (_ticketDate > rec->expireDate())
        return false;
      if (rec->createDate().date() > _ticketDateNoTime)
        return false;
    }
    else
    {
      if (_ticketDateNoTime > rec->expireDate().date())
        return false;
      if (rec->createDate().date() > _ticketDateNoTime)
        return false;
    }
    return true;
  }

private:
  const boost::gregorian::date _startDate;
  const DateTime _endDate;
  const DateTime& _ticketDate;
  const boost::gregorian::date _ticketDateNoTime;

  void init(const DateTime& ticketDate)
  {
    if (ticketDate.isEmptyDate())
      throw tse::ErrorResponseException(ErrorResponseException::SYSTEM_ERROR);
  }
};

template <typename T>
class IsCurrentDH
{
public:
  IsCurrentDH(const DateTime& ticketDate)
    : _ticketDate(ticketDate), _ticketDateNoTime(ticketDate.date())
  {
    init(ticketDate);
  }
  bool operator()(const T* rec) const
  {
    if (_ticketDate.historicalIncludesTime())
    {
      if (_ticketDate > rec->expireDate())
        return false;
      if (rec->createDate().date() > _ticketDateNoTime)
        return false;
    }
    else
    {
      if (_ticketDateNoTime > rec->expireDate().date())
        return false;
      if (rec->createDate().date() > _ticketDateNoTime)
        return false;
    }
    return true;
  }

private:
  const DateTime& _ticketDate;
  const boost::gregorian::date _ticketDateNoTime;
  void init(const DateTime& ticketDate)
  {
    if (ticketDate.isEmptyDate())
      throw tse::ErrorResponseException(ErrorResponseException::SYSTEM_ERROR);
  }
};

template <typename T>
class IsNotCurrentDH
{
public:
  IsNotCurrentDH(const DateTime& ticketDate)
    : _ticketDate(ticketDate), _ticketDateNoTime(ticketDate.date())
  {
    init(ticketDate);
  }
  bool operator()(const T* rec) const
  {
    if (_ticketDate.historicalIncludesTime())
    {
      if (_ticketDate > rec->expireDate())
        return true;
      if (rec->createDate().date() > _ticketDateNoTime)
        return true;
    }
    else
    {
      if (_ticketDateNoTime > rec->expireDate().date())
        return true;
      if (rec->createDate().date() > _ticketDateNoTime)
        return true;
    }
    return false;
  }

private:
  const DateTime& _ticketDate;
  const boost::gregorian::date _ticketDateNoTime;
  void init(const DateTime& ticketDate)
  {
    if (ticketDate.isEmptyDate())
      throw tse::ErrorResponseException(ErrorResponseException::SYSTEM_ERROR);
  }
};

template <typename T> class IsNotEffectiveT
{
public:

  IsNotEffectiveT(const DateTime& dateTime)
    : _dateTime(dateTime)
  {
  }

  bool operator()(const T* rec) const
  {
    if (rec->effDate() > _dateTime)
      return true;
    if (_dateTime > rec->discDate())
      return true;
    if (_dateTime > rec->expireDate())
      return true;
    return false;
  }

private:
  const DateTime _dateTime;
};

template <typename Key, typename T, bool lru = true>
class DataAccessObjectBase
{
  typedef typename sfc::Cache<Key, T>::pointer_type _pointer_type;
  typedef LDCHelper<DataAccessObjectBase, Key, T, _pointer_type> LDCHelper_type;

  friend class LDCHelper<DataAccessObjectBase, Key, T, _pointer_type>;
  friend class LDCHelper<DataAccessObjectBase, Key, T, _pointer_type>::DeserializeTask;

public:
  virtual const std::string& name() const { return _cache.name(); }

  LDCHelper_type& ldcHelper() { return _ldcHelper; }

  sfc::CompressedDataPtr getCompressed(RBuffer& is,
                                       RemoteCache::RCStatus& status,
                                       bool& fromQuery)
  {
    Key key;
    is.read(key);
    sfc::CompressedDataPtr compressed(_cache.getCompressed(key, status, fromQuery));
    if (RemoteCache::RC_NOT_COMPRESSED_CACHE == status)
    {
      LOG4CXX_ERROR(getLogger(), __FUNCTION__ << " #### cannot get here if CompressedCache");
    }
    if (compressed)
    {
      LOG4CXX_DEBUG(getLogger(),
                    __FUNCTION__ << " #### name:" << name() << ",key:" << key
                                 << " status=" << status
                                 << ",compressed->_deflated.size()=" << compressed->_deflated.size()
                                 << ",compressed->_inflatedSz=" << compressed->_inflatedSz
                                 << ",displaying not empty items");
    }
    return compressed;
  }

  void updateLDC(const ObjectKey& objectKey, CacheUpdateAction requestType)
  {
    LOG4CXX_DEBUG(getLogger(), "DataAccessObjectBase::" << __FUNCTION__ << " ####");
    // remove from LDC immediately
    switch (requestType)
    {
    case CACHE_UPDATE_INVALIDATE:
    {
      Key key;
      if (translateKey(objectKey, key))
      {
        _cache.getCacheImpl()->queueDiskInvalidate(key, true, true);
      }
    }
    break;
    case CACHE_UPDATE_CLEAR:
      _cache.getCacheImpl()->queueDiskClear();
      break;
    default:
      break;
    }
  }

  virtual sfc::CompressedData* compress(const T* data) const
  {
    std::cerr << __FUNCTION__ << " for " << name() << " not implemented!" << std::endl;
    assert(false);
    return nullptr;
  }

  virtual T* uncompress(const sfc::CompressedData&) const
  {
    std::cerr << __FUNCTION__ << " for " << name() << " not implemented!" << std::endl;
    assert(false);
    return nullptr;
  }

  uint32_t tableVersion() const { return static_cast<uint32_t>(_cache.tableVersion()); }

protected:
  friend class DAOFactory;

  class DAOFactory : public sfc::KeyedFactory<Key, T>
  {
  public:
    DAOFactory(DataAccessObjectBase& dao) : _dao(dao) {}
    virtual ~DAOFactory() {}

    sfc::CompressedData* compress(const T* data) const override { return _dao.compress(data); }

    T* uncompress(const sfc::CompressedData& compressed) const override
    {
      return _dao.uncompress(compressed);
    }

  private:
    DataAccessObjectBase& _dao;

    T* create(Key key) override
    {
      ++_dao._accessCount;
      return _dao.create(key);
    }

    CreateResult<T> create(const Key& key, int) override { return _dao.create(key, 0); }

    void destroy(Key key, T* t) override { _dao.destroy(key, t); }
  }; // DAOFactory

  class DAOCache
  {
  public:
    typedef typename sfc::Cache<Key, T>::pointer_type pointer_type;
    typedef T value_type;

    DAOCache(DataAccessObjectBase& dao,
             sfc::KeyedFactory<Key, T>& factory,
             size_t capacity,
             const std::string& cacheType,
             size_t version)
      : _dao(dao), _cache(CacheFactory<Key, T>::create(cacheType, factory, capacity, version))
    {
    }

    ~DAOCache() { delete _cache; }

    void setName(const std::string& name)
    {
      _cache->setName(name);
      _cache->initForLDC(name, DISKCACHE.getCacheTypeOptions(name));
    }

    const std::string& name() const { return _cache->getName(); }

    const std::string& getType() { return _cache->getType(); }

    size_t tableVersion() const { return _cache->tableVersion(); }

    size_t actionQueueSize() const { return _cache->actionQueueSize(); }

    void actionQueueClear() { _cache->actionQueueClear(); }

    bool actionQueueNext(LDCOperation<Key>& target) { return _cache->actionQueueNext(target); }

    size_t size() { return _cache->size(); }

    size_t clear() { return _cache->clear(); }

    void emptyTrash() { _cache->emptyTrash(); }

    sfc::Cache<Key, T>* getCacheImpl() { return _cache; }

    std::shared_ptr<std::vector<Key>> keys() { return _cache->keys(); }

    size_t consolidationSize() { return _cache->consolidationSize(); }

    size_t consolidate(size_t maxRecordsToConsolidate)
    {
      return _cache->consolidate(maxRecordsToConsolidate);
    }

    pointer_type get(const Key& key)
    {
      ++_dao._readCount;
      return _cache->get(key);
    }

    pointer_type getIfResident(const Key& key)
    {
      ++_dao._readCount;
      return _cache->getIfResident(key);
    }

    sfc::CompressedDataPtr getCompressed(const Key& key,
                                         RemoteCache::RCStatus& status,
                                         bool& fromQuery)
    {
      ++_dao._readCount;
      return _cache->getCompressed(key, status, fromQuery);
    }

    void put(const Key& key, T* object, bool updateLDC = true)
    {
      _cache->put(key, object, updateLDC);
    }

    size_t invalidate(const Key& key) { return _cache->invalidate(key); }

    bool ldcEnabled() const { return _cache->ldcEnabled(); }

    bool isLoading() const { return _cache->isLoading(); }

    void setLoading(bool val = true) { _cache->setLoading(val); }

    size_t getTotalCapacity() const { return _cache->getTotalCapacity(); }

    void setTotalCapacity(size_t totalCapacity) { _cache->setTotalCapacity(totalCapacity); }

    void setThreshold(size_t threshold) { _cache->setThreshold(threshold); }

    void getCompressionStats(sfc::CompressedCacheStats& stats)
    {
      _cache->getCompressionStats(stats);
    }

    void disposeCache(double fraction)
    {
      _cache->disposeCache(fraction);
    }

  private:
    DAOCache(const DAOCache&);
    DAOCache& operator=(const DAOCache&);

    DataAccessObjectBase& _dao;
    sfc::Cache<Key, T>* _cache;
  }; // DAOCache

  ConfigMan& _config;

  uint64_t _cacheMax;
  uint64_t _accessCount;
  uint64_t _readCount;

  bool _loadOnUpdate;

  size_t _partialConsolidationLimit;
  size_t _fullConsolidationLimit;

  CacheStats _cacheStats;

  uint64_t _codeCoverageLoadCallCount;
  uint64_t _codeCoverageGetCallCount;
  uint64_t _codeCoverageGetAllCallCount;
  uint64_t _codeCoverageGetFDCallCount;

  explicit DataAccessObjectBase(int cacheSize = 0,
                                const std::string& cacheType = "",
                                size_t version = 1)
    : _config(CacheManager::instance().config()),
      _cacheMax(static_cast<uint64_t>(cacheSize)),
      _accessCount(0),
      _readCount(0),
      _loadOnUpdate(false),
      _partialConsolidationLimit(std::numeric_limits<size_t>::max()),
      _fullConsolidationLimit(std::numeric_limits<size_t>::max()),
      _cacheStats(),
      _codeCoverageLoadCallCount(0),
      _codeCoverageGetCallCount(0),
      _codeCoverageGetAllCallCount(0),
      _codeCoverageGetFDCallCount(0),
      _factory(*this),
      _cache(*this, _factory, cacheSize, cacheType, version),
      _ldcHelper(*this)
  {
  }

  virtual ~DataAccessObjectBase() {}

  DAOCache& cache() { return _cache; }

  void setName(const std::string& name)
  {
    _ldcHelper.init(name);
    _cache.setName(name);
  }

  virtual void load() {}

  virtual T* create(Key key) { return new T; }

  virtual CreateResult<T> create(const Key& key, bool isHistorical)
  {
    CreateResult<T> remote;
    RemoteCache::RCClient::get(*this, key, remote, isHistorical);
    if (remote)// client enabled
    {
      if (RemoteCache::ReadConfig::isDebug())
      {
        CreateResult<T> local;
        local._ptr = create(key);
        if (local._ptr && remote._ptr)
        {
          std::string msg;
          bool equal(equalPtrContainer(*local._ptr, *remote._ptr, msg));
          if (!equal)
          {
            LOG4CXX_ERROR(RemoteCache::RCClient::getLogger(), name() << ':' << key << ':'
                          << "local!=remote:" << msg);
          }
        }
      }
      return remote;
    }
    // got here for server and local creates
    ++_accessCount;
    CreateResult<T> local;
    local._ptr = create(key);
    return local;
  }

  virtual void destroy(Key key, T* t) = 0;

  virtual size_t clear() { return _cache.clear(); }

  void emptyTrash() { _cache.emptyTrash(); }

  virtual void store(const ObjectKey& objectKey)
  {
    Key key;
    if (translateKey(objectKey, key))
    {
      _cache.get(key);
    }
  }

  virtual size_t invalidate(const ObjectKey& objectKey)
  {
    Key key;
    if (LIKELY(translateKey(objectKey, key)))
    {
      return invalidate(key);
    }
    return 0;
  }

  virtual bool translateKey(const ObjectKey& objectKey, Key& key) const { return false; }

  virtual void translateKey(const Key& key, ObjectKey& objectKey) const {}

  virtual size_t actionQueueSize() const { return _cache.actionQueueSize(); }

  virtual void actionQueueClear() { _cache.actionQueueClear(); }

  uint64_t accessCount() { return _accessCount; }

  uint64_t readCount() { return _readCount; }

  uint64_t cacheMax() { return _cacheMax; }

  uint64_t cacheSize() { return _cache.size(); }

  size_t fullConsolidationLimit() { return _fullConsolidationLimit; }

  size_t partialConsolidationLimit() { return _partialConsolidationLimit; }

  CacheStats* cacheStats() { return &_cacheStats; }

  void getCompressionStats(sfc::CompressedCacheStats& stats) { _cache.getCompressionStats(stats); }

  void disposeCache(double fraction) { _cache.disposeCache(fraction); }

  void incrementLoadCallCount() { ++_codeCoverageLoadCallCount; }

  virtual uint64_t loadCallCount() const { return _codeCoverageLoadCallCount; }

  virtual uint64_t getCallCount() const { return _codeCoverageGetCallCount; }

  virtual uint64_t getAllCallCount() const { return _codeCoverageGetAllCallCount; }

  virtual uint64_t getFDCallCount() const { return _codeCoverageGetFDCallCount; }

  template <class S>
  typename boost::enable_if<has_capacity<S>, size_t>::type get_capacity(S& obj)
  {
    return obj.capacity();
  }

  template <class S>
  typename boost::disable_if<has_capacity<S>, size_t>::type get_capacity(S& obj)
  {
    return 0;
  }

  template <class S>
  typename boost::enable_if<has_size<S>, size_t>::type get_size(S& obj)
  {
    return obj.size();
  }

  template <class S>
  typename boost::disable_if<has_size<S>, size_t>::type get_size(S& obj)
  {
    return sizeof(obj);
  }

  template <class S>
  typename boost::enable_if<has_capacity<S>, size_t>::type get_value_size(S& obj)
  {
    return sizeof(typename S::value_type);
  }

  template <class S>
  typename boost::disable_if<has_capacity<S>, size_t>::type get_value_size(S& obj)
  {
    return 1;
  }

  template <class S>
  typename boost::enable_if<has_capacity<S>, size_t>::type get_indirect_size(S& obj)
  {
    if (boost::is_pointer<typename S::value_type>::value)
      return sizeof(typename boost::remove_pointer<typename S::value_type>::type);
    else
      return 0;
  }

  template <class S>
  typename boost::disable_if<has_capacity<S>, size_t>::type get_indirect_size(S& obj)
  {
    return 0;
  }

  size_t getCacheMemory(size_t& used, size_t& indirect, size_t& item_size)
  {
    size_t memory = 0;
    used = 0;
    indirect = 0;
    item_size = sizeof(T);
    std::shared_ptr<std::vector<Key>> cacheKeys = cache().keys();
    typename std::vector<Key>::const_iterator keyPtr = cacheKeys->begin();
    typename std::vector<Key>::const_iterator end = cacheKeys->end();
    while (keyPtr != end)
    {
      const Key& key = (*keyPtr);
      keyPtr++;
      typename DAOCache::pointer_type itemPtr = cache().getIfResident(key);
      if (itemPtr)
      {
        T* item = itemPtr.get();
        if (item != nullptr)
        {
          if (boost::is_class<T>::value)
          {
            memory += get_capacity(*item) * get_value_size(*item);
            used += get_size(*item) * get_value_size(*item);
            indirect += get_size(*item) * get_indirect_size(*item);
          }
          else
          {
            used += sizeof(*item);
            if (boost::is_pointer<T>::value)
              indirect += sizeof(typename boost::remove_pointer<T>::type);
          }
        }
      }
    }
    return memory;
  }

  DiskCache::LoadSource loadSource() const { return _ldcHelper.loadSource(); }

  void setLoadSource(DiskCache::LoadSource val) { _ldcHelper.setLoadSource(val); }

  std::string loadSourceAsString() const { return _ldcHelper.loadSourceAsString(); }

  size_t consolidate(bool fullConsolidation)
  {
    if (fullConsolidation)
    {
      return _cache.consolidate(_fullConsolidationLimit);
    }
    else
    {
      return _cache.consolidate(_partialConsolidationLimit);
    }
  }

  uint64_t consolidationSize(bool fullConsolidation)
  {
    size_t effectiveSize = _cache.consolidationSize();

    if (fullConsolidation)
    {
      if (effectiveSize > _fullConsolidationLimit)
      {
        effectiveSize = _fullConsolidationLimit;
      }
    }
    else
    {
      if (effectiveSize > _partialConsolidationLimit)
      {
        effectiveSize = _partialConsolidationLimit;
      }
    }
    return effectiveSize;
  }

  void setLoadOnUpdate(bool newValue) { _loadOnUpdate = newValue; }

  virtual bool ldcEnabled() const { return _cache.ldcEnabled(); }

  virtual void setLoading(bool loading = true) { _cache.setLoading(loading); }

  virtual bool isLoading() const { return _cache.isLoading(); }

  void setFullConsolidationLimit(size_t newValue) { _fullConsolidationLimit = newValue; }

  void setPartialConsolidationLimit(size_t newValue) { _partialConsolidationLimit = newValue; }

  void setCacheBy(DAOUtils::CacheBy newValue) {}

  void setTotalCapacity(size_t value) { _cache.setTotalCapacity(value); }

  void setThreshold(size_t value) { _cache.setThreshold(value); }

  virtual log4cxx::LoggerPtr& getLogger() = 0;

  virtual bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
  {
    flatKey.clear();
    objectKey.tableName() = "";
    objectKey.keyFields().clear();
    return false;
  }

  size_t invalidate(const Key& key)
  {
    bool isMaster(RemoteCache::RCServerAttributes::isMaster(name()));
    if (_loadOnUpdate && !isMaster)
    {
      static const bool loadOnStart(CacheManager::instance().cacheLoadOnStart(name()));
      if (loadOnStart || _cache.getIfResident(key) != 0)
      {
        _cache.put(key, create(key, false)._ptr);
        LOG4CXX_INFO(getCacheUpdateLogger(), "Load on update:" << name() << '-' << key);
        return 1;
      }
    }
    else
    {
      return _cache.invalidate(key);
    }
    return 0;
  }

  virtual const std::string& cacheClass()
  {
    static std::string s("Default");
    return s;
  }

  template <typename Container>
  void destroyContainer(Container* container)
  {
    if (container == nullptr)
    {
      return;
    }

    for (auto& elem : *container)
    {
      delete elem;
    }

    delete container;
  }

  template <typename I, typename DAO>
  class DummyObjectInserter
  {
  private:
    bool _success;

  public:
    bool success() const { return _success; }

    void putInfoIntoObject(const Key& key, I* info, std::vector<I*>* object)
    {
      object->push_back(info);
    }

    void putInfoIntoObject(const Key& key, I* info, std::vector<const I*>* object)
    {
      object->push_back(info);
    }

    void putInfoIntoObject(const Key& key, I* info, std::map<Key, I*>* object)
    {
      object->insert(std::map<Key, I*>::value_type(key, info));
    }

    template <typename A, typename C, typename D>
    void putInfoIntoObject(const Key& key,
                           I* info,
                           std::tr1::unordered_multimap<const A, I*, C, D>* object)
    {
      object->insert(
          typename std::tr1::unordered_multimap<const A, I*, C, D>::value_type(A(), info));
    }

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

    template <typename A, typename C, typename D>
    void putInfoIntoObject(const Key& key,
                           I* info,
                           std::unordered_map<A, std::vector<I*>, C, D>* object)
    {
      (*object)[A()].push_back(info);
    }

#endif

    template <typename A>
    void putInfoIntoObject(const Key& key, I* info, std::multimap<A, I*>* object)
    {
      object->insert(typename std::multimap<A, I*>::value_type(A(), info));
    }

    void putInfoIntoObject(const Key& key, I* info, I* object)
    {
      // not really a collection, so no insertion necessary
    }

    DummyObjectInserter(std::string& flatKey, ObjectKey& objectKey) : _success(false)
    {
      I* info(new I);
      T* object(new T);
      I::dummyData(*info);
      Key key(DAO::instance().createKey(info));
      putInfoIntoObject(key, info, object);
      DAO::instance().cache().put(key, object, true);

      objectKey.tableName() = DAO::instance().name();
      objectKey.keyFields().clear();
      DAO::instance().translateKey(key, objectKey);

      KeyStream stream(0);
      stream << key;
      flatKey = stream;

      _success = true;
    }
  };

  template <typename DAO>
  void putByKey(DAO& dao, T& recs, bool updateLDC = true)
  {
    typedef typename T::value_type value_type;

    Key prevKey;
    T* ptr = nullptr;

    for (const auto& rec : recs)
    {
      value_type info = const_cast<value_type>(rec);

      // The 'putByKey' function is only called whenever records have been
      // preloaded from the Database.  For HISTORICAL, only those caches that
      // are NOT qualified with a start and end date are preloaded in this
      // manner.  Therefore, the associated DAO's 'createKey' method has no
      // need to ever be called with start and end dates.

      Key key(dao.createKey(info));

      if (!(key == prevKey))
      {
        if (ptr != nullptr)
        {
          cache().put(prevKey, ptr, updateLDC);
        }
        ptr = new T;
        prevKey = key;
      }

      ptr->push_back(info);
    }

    if (ptr != nullptr)
    {
      cache().put(prevKey, ptr, updateLDC);
    }
  }

  template <typename I>
  void printCacheObject(const std::vector<I*>& source)
  {
    for (typename std::vector<I*>::const_iterator cit = source.begin(), citEnd = source.end();
         cit != citEnd;
         ++cit)
    {
      const I* info = (*cit);
      LOG4CXX_INFO(getLogger(), "      => Object: " << *info);
    }
  }

  void printCache()
  {
    LOG4CXX_INFO(getLogger(), "CACHE CONTENTS for [" << name() << "]:");
    std::shared_ptr<std::vector<Key>> keys = cache().keys();
    for (typename std::vector<Key>::const_iterator kit = keys->begin(), kitEnd = keys->end();
         kit != kitEnd;
         ++kit)
    {
      const Key& key = (*kit);
      typename DAOCache::pointer_type ptr = cache().getIfResident(key);
      if (ptr.get() != NULL)
      {
        LOG4CXX_INFO(getLogger(), "   => Key: [" << key.toString() << "]");
        printCacheObject(*ptr);
      }
    }
    LOG4CXX_INFO(getLogger(), "END of CACHE CONTENTS for [" << name() << "].");
  }

  template <typename I>
  void deepCopyCacheObject(const std::vector<I*>& source, std::vector<I*>& target)
  {
    for (const auto elem : source)
    {
      target.push_back(elem->newDuplicate());
    }
  }

  void deepCopyCache(T& target)
  {
    // convenience method to make a deep copy of the cache
    std::shared_ptr<std::vector<Key>> keys = cache().keys();
    for (typename std::vector<Key>::const_iterator kit = keys->begin(), kitEnd = keys->end();
         kit != kitEnd;
         ++kit)
    {
      const Key& key = (*kit);
      typename sfc::Cache<Key, T>::pointer_type ptr = cache().getIfResident(key);
      if (ptr.get() != nullptr)
      {
        deepCopyCacheObject(*ptr, target);
      }
    }
  }

  DAOFactory _factory;
  DAOCache _cache;
  LDCHelper_type _ldcHelper;
};

} // namespace tse;

