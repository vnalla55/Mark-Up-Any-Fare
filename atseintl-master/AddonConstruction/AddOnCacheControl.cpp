//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "AddonConstruction/AddOnCacheControl.h"

#include "AddonConstruction/ACLRUCache.h"
#include "AddonConstruction/ConstructedCacheManager.h"
#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "Common/Vendor.h"
#include "DBAccess/CacheRegistry.h"

namespace tse
{
AddOnCacheControl* instance = nullptr;

AddOnCacheControl::AddOnCacheControl()
  : _id("ADDONCONSTRUCTION"), _ldcHelper(ConstructedCacheManager::instance())
{
  CacheRegistry& registry(CacheRegistry::instance());
  registry.addEntry(_id, this);
  init();
  LOG4CXX_INFO(logger(), "AddOnCacheControl registered"); // lint !e666
}

void
AddOnCacheControl::classInit()
{
  if (instance == nullptr)
  {
    instance = new AddOnCacheControl;
  }
}

void
AddOnCacheControl::init(bool preLoad)
{
  LOG4CXX_WARN(logger(), "AddOnCacheControl::init called");
  _ldcHelper.init(_id);
  ConstructedCacheManager& cacheMan(ConstructedCacheManager::instance());
  cacheMan.cache().initForLDC(_id, DISKCACHE.getCacheTypeOptions(_id));
}

size_t
AddOnCacheControl::invalidate(const ObjectKey& key)
{
  size_t result(cacheSize());
  ConstructedCacheManager& cacheMan(ConstructedCacheManager::instance());
  if (LIKELY(!cacheMan.useCache()))
    return 0;

  VendorCode vendor;
  CarrierCode cxr;
  LocCode market1;
  LocCode market2;

  LOG4CXX_DEBUG(logger(), "AddOnCacheControl::invalidate called");

  if (!key.getValue("CARRIER", cxr))
    return 0;
  if (key.getValue("MARKET1", market1) && key.getValue("MARKET2", market2))
  {
    if (key.getValue("VENDOR", vendor))
    {
      cacheMan.flushSpecifiedFares(market1, market2, vendor, cxr);
    }
    else
    {
      cacheMan.flushSpecifiedFares(market1, market2, Vendor::ATPCO, cxr);
      cacheMan.flushSpecifiedFares(market1, market2, Vendor::SITA, cxr);
    }
  }
  else if (key.getValue("INTERIORMARKET", market1) && key.getValue("GATEWAYMARKET", market2))
  {
    if (!key.getValue("VENDOR", vendor))
    {
      cacheMan.flushAddonFares(market1, market2, vendor, cxr);
    }
    else
    {
      cacheMan.flushAddonFares(market1, market2, Vendor::ATPCO, cxr);
      cacheMan.flushAddonFares(market1, market2, Vendor::SITA, cxr);
    }
  }
  else
  {
    if (!key.getValue("VENDOR", vendor))
      return 0;
    cacheMan.flushVendorCxrFares(vendor, cxr);
  }
  return result - cacheSize();
}

void
AddOnCacheControl::emptyTrash()
{
}

size_t
AddOnCacheControl::clear()
{
  size_t result(cacheSize());
  LOG4CXX_INFO(logger(), "AddOnCacheControl::clear called");
  ConstructedCacheManager& cacheMan(ConstructedCacheManager::instance());
  cacheMan.flushAll();
  return result;
}

uint64_t
AddOnCacheControl::accessCount()
{
  ConstructedCacheManager& cacheMan(ConstructedCacheManager::instance());
  return cacheMan.aclruCache().accessCount();
}

uint64_t
AddOnCacheControl::readCount()
{
  ConstructedCacheManager& cacheMan(ConstructedCacheManager::instance());
  return cacheMan.aclruCache().readCount();
}

uint64_t
AddOnCacheControl::cacheMax()
{
  ConstructedCacheManager& cacheMan(ConstructedCacheManager::instance());
  return cacheMan.aclruCache().capacity();
}

uint64_t
AddOnCacheControl::cacheSize()
{
  ConstructedCacheManager& cacheMan(ConstructedCacheManager::instance());
  return cacheMan.aclruCache().size();
}

log4cxx::LoggerPtr&
AddOnCacheControl::logger()
{
  static log4cxx::LoggerPtr logger(
      log4cxx::Logger::getLogger("atseintl.AddonConstruction.AddOnCacheControl"));
  return logger;
}

std::string
AddOnCacheControl::getCacheType() const
{
  ConstructedCacheManager& cacheMan(ConstructedCacheManager::instance());
  return cacheMan.aclruCache().getType();
}

const std::string&
AddOnCacheControl::getID() const
{
  return _id;
}

bool
AddOnCacheControl::compareMemCacheToLDC(std::vector<std::string>& results, uint32_t& numCompared)
{
  return _ldcHelper.compareMemCacheToLDC(results, numCompared);
}

void
AddOnCacheControl::getAllFlatKeys(std::set<std::string>& list, bool inclValues)
{
  _ldcHelper.getAllFlatKeys(list, inclValues);
}

void
AddOnCacheControl::invalidate(const std::string& flatKey)
{
  LOG4CXX_INFO(logger(), "AddOnCacheControl::invalidate(flatKey) called");
  CacheKey key;
  key.fromString(flatKey);
  ConstructedCacheManager& cacheMan(ConstructedCacheManager::instance());
  if (!cacheMan.useCache())
    return;

  cacheMan.aclruCache().invalidate(key);
}

bool
AddOnCacheControl::insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
{
  flatKey.clear();
  objectKey.tableName() = "";
  objectKey.keyFields().clear();
  return false;
}

bool
AddOnCacheControl::objectExistsInMem(const std::string& flatKey)
{
  return _ldcHelper.objectExistsInMem(flatKey);
}

bool
AddOnCacheControl::objectExistsInLDC(const std::string& flatKey, time_t& timestamp)
{
  return _ldcHelper.objectExistsInLDC(flatKey, timestamp);
}

CacheStats*
AddOnCacheControl::cacheStats()
{
  ConstructedCacheManager& cacheMan(ConstructedCacheManager::instance());
  return cacheMan.aclruCache().cacheStats();
}

size_t
AddOnCacheControl::getCacheMemory(size_t& used, size_t& indirect, size_t& item_size)
{
  size_t memory = 0;
  used = 0;
  indirect = 0;
  item_size = sizeof(ConstructedCacheDataWrapper);
  ConstructedCacheManager& cacheMan(ConstructedCacheManager::instance());
  std::shared_ptr<std::vector<CacheKey>> cacheKeys = cacheMan.aclruCache().keys();

  std::vector<CacheKey>::const_iterator keyPtr = cacheKeys->begin();
  std::vector<CacheKey>::const_iterator end = cacheKeys->end();
  while (keyPtr != end)
  {
    const CacheKey& key = (*keyPtr);
    keyPtr++;

    std::shared_ptr<ConstructedCacheDataWrapper> itemPtr = cacheMan.aclruCache().getIfResident(key);
    if (itemPtr)
    {
      ConstructedCacheDataWrapper* item = itemPtr.get();
      if (item != nullptr)
      {
        memory += item->ccFares().capacity() * sizeof(CacheConstructedFareInfoVec::value_type);
        used += item->ccFares().size() * sizeof(CacheConstructedFareInfoVec::value_type);
        indirect += item->ccFares().size() *
                    sizeof(boost::remove_pointer<CacheConstructedFareInfoVec::value_type>);
      }
    }
  }

  return memory;
}

uint32_t
AddOnCacheControl::tableVersion() const
{
  ConstructedCacheManager& cacheMan(ConstructedCacheManager::instance());
  return cacheMan.aclruCache().tableVersion();
}

size_t
AddOnCacheControl::actionQueueSize() const
{
  ConstructedCacheManager& cacheMan(ConstructedCacheManager::instance());
  return cacheMan.aclruCache().actionQueueSize();
}

bool
AddOnCacheControl::processNextAction(LDCOperationCounts& counts)
{
  bool retval = false;
  retval = _ldcHelper.processNextAction(counts);
  return retval;
}

bool
AddOnCacheControl::ldcEnabled() const
{
  ConstructedCacheManager& cacheMan(ConstructedCacheManager::instance());
  return cacheMan.aclruCache().ldcEnabled();
}

void
AddOnCacheControl::getMemObjectAsFlat(const std::string& flatKey, std::string& result)
{
  _ldcHelper.getMemObjectAsFlat(flatKey, result);
}

void
AddOnCacheControl::getLDCObjectAsFlat(const std::string& flatKey, std::string& result)
{
  _ldcHelper.getLDCObjectAsFlat(flatKey, result);
}
}
