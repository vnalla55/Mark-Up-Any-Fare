//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once

#include "DBAccess/CacheControl.h"
#include "DBAccess/CacheManager.h"
#include "DBAccess/CacheRegistry.h"
#include "DBAccess/DiskCache.h"
#include "DBAccess/LDCOperationCounts.h"
#include "DBAccess/ObjectKey.h"

#include <string>

namespace tse
{
struct CacheStats;

// T - DataAccessObject<type>
template <typename T>
class DAOHelper : public CacheControl
{
public:
  DAOHelper(std::string id) : _id(id)
  {
    CacheRegistry& registry = CacheRegistry::instance();
    registry.addEntry(id, this);
  }

  void init(bool load = true) override
  {
    boost::lock_guard<boost::mutex> g(_mutex);
    T*& t = T::_instance;
    if (t == nullptr)
    {
      CacheManager& cm = CacheManager::instance();

      int cacheSize = cm.cacheSize(_id);
      const std::string& cacheType = cm.cacheType(_id);
      size_t fullConsolidationLimit = cm.cacheFullConsolidationLimit(_id);
      size_t partialConsolidationLimit = cm.cachePartialConsolidationLimit(_id);

      t = new T(cacheSize, cacheType);
      t->setName(_id);
      t->setFullConsolidationLimit(fullConsolidationLimit);
      t->setPartialConsolidationLimit(partialConsolidationLimit);
      t->setTotalCapacity(cm.getTotalCapacity(_id));
      t->setThreshold(cm.getThreshold(_id));

      if (load)
      {
        t->setLoading(true);
        DiskCache::Timer loadTimer;

        LOG4CXX_DEBUG(t->getLogger(), "Starting LOAD for cache [" << _id << "].");
        loadTimer.reset();
        t->load();
        DISKCACHE.doneWithDB(_id);
        loadTimer.checkpoint();
        size_t cacheSize(t->cache().size());
        if (cacheSize > 0)
        {
          std::string ess(cacheSize > 1 ? "s" : "");
          LOG4CXX_INFO(t->getLogger(),
                       "Loaded [" << cacheSize << "] object" << ess << " for cache [" << _id << "]."
                                  << "  SOURCE=" << t->loadSourceAsString() << "."
                                  << "  ELAPSED=" << loadTimer.elapsed() << "ms."
                                  << "  CPU=" << loadTimer.cpu() << "ms.");
        }

        // Perform a full consolidation so things start out clean
        t->consolidate(true);

        t->setLoading(false);
      }

      t->setLoadOnUpdate(cm.cacheLoadOnUpdate(_id));
      t->setCacheBy(cm.cacheBy(_id));
    }
  }

  uint64_t accessCount() override
  {
    T* t = T::_instance;
    return (t == nullptr) ? 0 : t->accessCount();
  }

  uint64_t readCount() override
  {
    T* t = T::_instance;
    return (t == nullptr) ? 0 : t->readCount();
  }

  uint64_t cacheMax() override
  {
    T* t = T::_instance;
    return (t == nullptr) ? 0 : t->cacheMax();
  }

  uint64_t cacheSize() override
  {
    T* t = T::_instance;
    return (t == nullptr) ? 0 : t->cacheSize();
  }

  CacheStats* cacheStats() override
  {
    T* t = T::_instance;
    return (t == nullptr) ? nullptr : t->cacheStats();
  }

  void compressedCacheStats(sfc::CompressedCacheStats& stats) override
  {
    T* t = T::_instance;
    if (t != nullptr)
    {
      t->getCompressionStats(stats);
    }
  }

  virtual void disposeCache(double fraction) override
  {
    T* t = T::_instance;
    if (t != nullptr)
    {
      t->disposeCache(fraction);
    }
  }

  sfc::CompressedDataPtr getCompressed(RBuffer& is,
                                       RemoteCache::RCStatus& status,
                                       bool& fromQuery) override
  {
    sfc::CompressedDataPtr result;
    T* t(T::_instance);
    if (t != nullptr)
    {
      result = t->getCompressed(is, status, fromQuery);
    }
    return result;
  }

  void updateLDC(const ObjectKey& objectKey, CacheUpdateAction requestType) override
  {
    T* t(T::_instance);
    if (t != nullptr)
    {
      t->updateLDC(objectKey, requestType);
    }
  }

  size_t getCacheMemory(size_t& used, size_t& indirect, size_t& item_size) override
  {
    T* t = T::_instance;
    if (t == nullptr)
    {
      used = 0;
      indirect = 0;
      item_size = 0;
      return 0;
    }
    return t->getCacheMemory(used, indirect, item_size);
  }

  uint64_t loadCallCount() const override
  {
    T* t = T::_instance;
    return (t == nullptr) ? 0 : t->loadCallCount();
  }

  uint64_t getCallCount() const override
  {
    T* t = T::_instance;
    return (t == nullptr) ? 0 : t->getCallCount();
  }

  uint64_t getAllCallCount() const override
  {
    T* t = T::_instance;
    return (t == nullptr) ? 0 : t->getAllCallCount();
  }

  uint64_t getFDCallCount() const override
  {
    T* t = T::_instance;
    return (t == nullptr) ? 0 : t->getFDCallCount();
  }

  size_t fullConsolidationLimit()
  {
    T* t = T::_instance;
    return (t == 0) ? 0 : t->fullConsolidationLimit();
  }

  size_t partialConsolidationLimit()
  {
    T* t = T::_instance;
    return (t == 0) ? 0 : t->partialConsolidationLimit();
  }

  const std::string& getID() const override { return _id; }

  bool compareMemCacheToLDC(std::vector<std::string>& results, uint32_t& numCompared) override
  {
    T* t = T::_instance;
    if (t != nullptr)
    {
      return t->ldcHelper().compareMemCacheToLDC(results, numCompared);
    }
    else
    {
      return false;
    }
  }

  bool compareMemCacheToDB(std::vector<std::string>& results, uint32_t& numCompared) override
  {
    T* t = T::_instance;
    if (t != nullptr)
    {
      return t->ldcHelper().compareMemCacheToDB(results, numCompared);
    }
    else
    {
      return false;
    }
  }

  std::string getCacheType() const override
  {
    T* t = T::_instance;
    if (t != nullptr)
    {
      return t->cache().getType();
    }
    else
    {
      return "";
    }
  }

  void getAllFlatKeys(std::set<std::string>& list, bool inclValues = false) override
  {
    T* t = T::_instance;
    if (t != nullptr)
    {
      t->ldcHelper().getAllFlatKeys(list, inclValues);
    }
  }

  bool processNextAction(LDCOperationCounts& counts) override
  {
    bool retval = false;

    T* t = T::_instance;
    if (LIKELY(t != nullptr))
    {
      retval = t->ldcHelper().processNextAction(counts);
    }

    return retval;
  }

  void store(const ObjectKey& objectKey) override
  {
    T* t = T::_instance;
    if (t == nullptr)
    {
      init(false);
    }
    t = T::_instance;
    if (t != nullptr)
    {
      t->store(objectKey);
    }
  }

  size_t invalidate(const ObjectKey& objectKey) override
  {
    T* t = T::_instance;
    if (LIKELY(t != nullptr))
    {
      return t->invalidate(objectKey);
    }
    return 0;
  }

  void invalidate(const std::string& flatKey) override
  {
    T* t = T::_instance;
    if (t != nullptr)
    {
      t->ldcHelper().invalidate(flatKey);
    }
  }

  bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey) override
  {
    T* t = T::_instance;
    if (t == nullptr)
    {
      init(false);
    }
    t = T::_instance;
    if (t != nullptr)
    {
      return t->insertDummyObject(flatKey, objectKey);
    }
    else
    {
      flatKey.clear();
      objectKey.tableName() = "";
      objectKey.keyFields().clear();
      return false;
    }
  }

  bool objectExistsInMem(const std::string& flatKey) override
  {
    T* t = T::_instance;
    if (t == nullptr)
    {
      init(false);
    }
    t = T::_instance;
    if (t != nullptr)
    {
      return t->ldcHelper().objectExistsInMem(flatKey);
    }
    else
    {
      return false;
    }
  }

  bool objectExistsInLDC(const std::string& flatKey, time_t& timestamp) override
  {
    T* t = T::_instance;
    if (t == nullptr)
    {
      init(false);
    }
    t = T::_instance;
    if (t != nullptr)
    {
      return t->ldcHelper().objectExistsInLDC(flatKey, timestamp);
    }
    else
    {
      return false;
    }
  }

  bool objectExistsInDB(const std::string& flatKey) override
  {
    T* t = T::_instance;
    if (t == nullptr)
    {
      init(false);
    }
    t = T::_instance;
    if (t != nullptr)
    {
      return t->ldcHelper().objectExistsInDB(flatKey);
    }
    else
    {
      return false;
    }
  }

  void getMemObjectAsXML(const std::string& flatKey, std::string& result) override
  {
    T* t = T::_instance;
    if (t == nullptr)
    {
      init(false);
    }
    t = T::_instance;
    if (t != nullptr)
    {
      t->ldcHelper().getMemObjectAsXML(flatKey, result);
    }
  }

  void getLDCObjectAsXML(const std::string& flatKey, std::string& result) override
  {
    T* t = T::_instance;
    if (t == nullptr)
    {
      init(false);
    }
    t = T::_instance;
    if (t != nullptr)
    {
      t->ldcHelper().getLDCObjectAsXML(flatKey, result);
    }
  }

  void getDBObjectAsXML(const std::string& flatKey, std::string& result) override
  {
    T* t = T::_instance;
    if (t == nullptr)
    {
      init(false);
    }
    t = T::_instance;
    if (t != nullptr)
    {
      t->ldcHelper().getDBObjectAsXML(flatKey, result);
    }
  }

  void getMemObjectAsText(const std::string& flatKey, std::string& result) override
  {
    T* t = T::_instance;
    if (t == nullptr)
    {
      init(false);
    }
    t = T::_instance;
    if (t != nullptr)
    {
      t->ldcHelper().getMemObjectAsText(flatKey, result);
    }
  }

  void getLDCObjectAsText(const std::string& flatKey, std::string& result) override
  {
    T* t = T::_instance;
    if (t == nullptr)
    {
      init(false);
    }
    t = T::_instance;
    if (t != nullptr)
    {
      t->ldcHelper().getLDCObjectAsText(flatKey, result);
    }
  }

  void getDBObjectAsText(const std::string& flatKey, std::string& result) override
  {
    T* t = T::_instance;
    if (t == nullptr)
    {
      init(false);
    }
    t = T::_instance;
    if (t != nullptr)
    {
      t->ldcHelper().getDBObjectAsText(flatKey, result);
    }
  }

  void getMemObjectAsFlat(const std::string& flatKey, std::string& result) override
  {
    T* t = T::_instance;
    if (t == nullptr)
    {
      init(false);
    }
    t = T::_instance;
    if (t != nullptr)
    {
      t->ldcHelper().getMemObjectAsFlat(flatKey, result);
    }
  }

  void getLDCObjectAsFlat(const std::string& flatKey, std::string& result) override
  {
    T* t = T::_instance;
    if (t == nullptr)
    {
      init(false);
    }
    t = T::_instance;
    if (t != nullptr)
    {
      t->ldcHelper().getLDCObjectAsFlat(flatKey, result);
    }
  }

  void getDBObjectAsFlat(const std::string& flatKey, std::string& result) override
  {
    T* t = T::_instance;
    if (t == nullptr)
    {
      init(false);
    }
    t = T::_instance;
    if (t != nullptr)
    {
      t->ldcHelper().getDBObjectAsFlat(flatKey, result);
    }
  }

  size_t clear() override
  {
    T* t = T::_instance;
    if (t != nullptr)
    {
      return t->clear();
    }
    return 0;
  }

  void emptyTrash() override
  {
    T* t = T::_instance;
    if (LIKELY(t != nullptr))
    {
      t->emptyTrash();
    }
  }

  virtual size_t consolidate(bool fullConsolidation)
  {
    T* t = T::_instance;
    return (t == nullptr) ? 0 : t->consolidate(fullConsolidation);
  }

  uint64_t consolidationSize(bool fullConsolidation)
  {
    T* t = T::_instance;
    return (t == 0) ? 0 : t->consolidationSize(fullConsolidation);
  }

  uint32_t tableVersion() const override
  {
    T* t = T::_instance;
    if (LIKELY(t != nullptr))
    {
      return t->tableVersion();
    }
    else
    {
      return 0;
    }
  }

  size_t actionQueueSize() const override
  {
    T* t = T::_instance;
    if (LIKELY(t != nullptr))
    {
      return t->actionQueueSize();
    }
    else
    {
      return 0;
    }
  }

  void actionQueueClear() override
  {
    T* t = T::_instance;
    if (t != nullptr)
    {
      t->actionQueueClear();
    }
  }

  bool ldcEnabled() const override
  {
    T* t = T::_instance;
    if (t != nullptr)
    {
      return t->ldcEnabled();
    }
    else
    {
      return false;
    }
  }

private:
  static boost::mutex _mutex;

  std::string _id;
};

template <typename T>
boost::mutex DAOHelper<T>::_mutex;

template <typename T>
void
deleteVectorOfPointers(std::vector<T*>& v)
{
  T* item;
  while (!v.empty())
  {
    item = v.back();
    v.pop_back();
    delete item;
  }
}

} // namespace tse

