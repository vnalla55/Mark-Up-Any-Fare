//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once

#include "Common/Config/ConfigMan.h"
#include "Common/Global.h"
#include "DataModel/CacheUpdateAction.h"
#include "DBAccess/CompressedData.h"
#include "DBAccess/LDCOperationCounts.h"
#include "DBAccess/ObjectKey.h"
#include "DBAccess/RemoteCache/RemoteCacheHeader.h"

namespace sfc
{
struct CompressedCacheStats;
}

namespace tse
{
struct CacheStats;
class RBuffer;

static const std::string UNDEFINED_ID = "UNDEFINED";

/**
 * virtual class that defines the control interface for data access objects.
 * Note: this class should really be pure virtual but gcc doesn't generate
 * a vtable for it when only templates implement it.
 */
class CacheControl
{
public:
  virtual std::string getCacheType() const { return UNDEFINED_ID; }

  virtual const std::string& getID() const { return UNDEFINED_ID; }

  virtual bool compareMemCacheToLDC(std::vector<std::string>& results, uint32_t& numCompared)
  {
    return false;
  }

  virtual bool compareMemCacheToDB(std::vector<std::string>& results, uint32_t& numCompared)
  {
    return false;
  }

  virtual void getAllFlatKeys(std::set<std::string>& list, bool inclValues = false) {}

  virtual void invalidate(const std::string& flatKey) {}

  virtual bool insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
  {
    flatKey.clear();
    objectKey.tableName() = "";
    objectKey.keyFields().clear();
    return false;
  }

  virtual bool objectExistsInMem(const std::string& flatKey) { return false; }

  virtual bool objectExistsInLDC(const std::string& flatKey, time_t& timestamp) { return false; }

  virtual bool objectExistsInDB(const std::string& flatKey) { return false; }

  virtual void getDBObjectAsXML(const std::string& flatKey, std::string& result) {}

  virtual void getMemObjectAsXML(const std::string& flatKey, std::string& result) {}

  virtual void getLDCObjectAsXML(const std::string& flatKey, std::string& result) {}

  virtual void getDBObjectAsText(const std::string& flatKey, std::string& result) {}

  virtual void getMemObjectAsText(const std::string& flatKey, std::string& result) {}

  virtual void getLDCObjectAsText(const std::string& flatKey, std::string& result) {}

  virtual void getDBObjectAsFlat(const std::string& flatKey, std::string& result) {}

  virtual void getMemObjectAsFlat(const std::string& flatKey, std::string& result) {}

  virtual void getLDCObjectAsFlat(const std::string& flatKey, std::string& result) {}

  virtual void init(bool preLoad = true) {}

  virtual void store(const ObjectKey& key) = 0;

  virtual bool nodupInvalidate(const ObjectKey& key)
  {
    if (((time(nullptr) - lastInvalidate) < dupInterval()) && (lastKey == key))
    {
      return false;
    }
    invalidate(key);
    lastInvalidate = time(nullptr);
    lastKey = key;
    return true;
  }

  virtual size_t invalidate(const ObjectKey& key) = 0;

  virtual size_t clear() = 0;

  virtual void emptyTrash() = 0;

  virtual uint64_t accessCount() { return 0; }

  virtual uint64_t readCount() { return 0; }

  virtual uint64_t cacheMax() { return 0; }

  virtual uint64_t cacheSize() { return 0; }

  virtual CacheStats* cacheStats() { return nullptr; }

  virtual void compressedCacheStats(sfc::CompressedCacheStats&) {}

  virtual void disposeCache(double fraction) {}

  virtual sfc::CompressedDataPtr getCompressed(RBuffer& is,
                                               RemoteCache::RCStatus& status,
                                               bool&)
  {
    status = RemoteCache::RC_NOT_IMPLEMENTED;
    return sfc::CompressedDataPtr();
  }

  virtual void updateLDC(const ObjectKey&, CacheUpdateAction) {}

  virtual size_t getCacheMemory(size_t& used, size_t& indirect, size_t& item_size) { return 0; }

  virtual uint32_t tableVersion() const { return 1; }

  virtual size_t actionQueueSize() const { return 0; }

  virtual void actionQueueClear() {}

  virtual bool processNextAction(LDCOperationCounts& counts) { return false; }

  virtual bool ldcEnabled() const { return false; }

  // Functions used to report code coverage within DAOs
  virtual uint64_t loadCallCount() const { return 0; }
  virtual uint64_t getCallCount() const { return 0; }
  virtual uint64_t getAllCallCount() const { return 0; }
  virtual uint64_t getFDCallCount() const { return 0; }

protected:
  // Used by the nodupInvalidate()
  time_t lastInvalidate;
  ObjectKey lastKey;
  int32_t _dupInterval;
  virtual int32_t dupInterval(void)
  {
    if (_dupInterval == -1)
    {
      tse::ConfigMan& config = Global::config();
      std::string configValue;
      static log4cxx::LoggerPtr logger(
          log4cxx::Logger::getLogger("atseintl.CacheUpdate.CacheUpdateService"));

      // Initialize the duplicate time interval to safely ignore duplicate cache notify updates
      // used by nodupInvalidate()
      if (!config.getValue("DUP_INTERVAL", configValue, "CACHE_ADP"))
      {
        _dupInterval = 10;
        if (logger)
          LOG4CXX_WARN(logger,
                       "Couldn't find config entry for 'CACHE_ADP:DUP_INTERVAL', using default of "
                           << _dupInterval);
      }
      else
      {
        _dupInterval = atoi(configValue.c_str());
        if (logger)
          LOG4CXX_DEBUG(logger, "Found 'CACHE_ADP:DUP_INTERVAL', using value of " << _dupInterval);
      }
    }
    return (_dupInterval);
  }

  CacheControl() : lastInvalidate(0), _dupInterval(-1) {}
  virtual ~CacheControl() {}
};

} // namepsace tse
