//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/AddOnCombFareClassDAO.h"

#include "Common/Logger.h"
#include "DBAccess/AddonCombFareClassInfo.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetCombFareClass.h"

#include <boost/range/algorithm.hpp>

namespace tse
{
log4cxx::LoggerPtr
AddOnCombFareClassDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.AddOnCombFareClassDAO"));

AddOnCombFareClassDAO&
AddOnCombFareClassDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

const AddonFareClassCombMultiMap&
getAddOnCombFareClassData(const VendorCode& vendor,
                          const TariffNumber& fareTariff,
                          const CarrierCode& carrier,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    AddOnCombFareClassHistoricalDAO& dao = AddOnCombFareClassHistoricalDAO::instance();
    const std::vector<AddonCombFareClassInfo*>& vect(
        dao.get(deleteList, vendor, fareTariff, carrier, ticketDate));
    AddonFareClassCombMultiMap* map(new AddonFareClassCombMultiMap);
    deleteList.adopt(map);
    // Make a local copy
    // date (travelDate) is invalid, so just check ticketDate using ICurrentH
    IsCurrentH<AddonCombFareClassInfo> pred(ticketDate);
    std::vector<AddonCombFareClassInfo*>::const_iterator it(vect.begin()), itEnd(vect.end());
    while (it != itEnd)
    {
      AddonCombFareClassInfo* addon(*it);
      if (pred(addon))
      {
        AddonCombFareClassSpecifiedKey key(addon->fareClass(), addon->owrt());
        AddonFareClassCombMultiMap::mapped_type& pr((*map)[std::move(key)]);
        if (!QueryGetCombFareClass::complementGeoAppl(addon, pr))
          pr.push_back(addon);
      }
      ++it;
    }
    return *map;
  }
  else
  {
    AddOnCombFareClassDAO& dao = AddOnCombFareClassDAO::instance();
    return dao.get(deleteList, vendor, fareTariff, carrier);
  }
} // getAddOnCombFareClassData()

const AddonFareClassCombMultiMap&
AddOnCombFareClassDAO::get(DeleteList& del,
                           const VendorCode& vendor,
                           const TariffNumber& fareTariff,
                           const CarrierCode& carrier)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  AddOnCombFareClassKey key(vendor, fareTariff, carrier);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  return *ptr;
}

void
AddOnCombFareClassDAO::load()
{
  AddonFareClassVCCombMap maps;
  Loader<QueryGetAllCombFareClass, AddOnCombFareClassDAO, AddonFareClassVCCombMap> loader(maps);
  if (loader.successful() && (!loader.gotFromLDC()))
  {
    for (AddonFareClassVCCombMap::const_iterator iter = maps.begin(); iter != maps.end(); ++iter)
    {
      AddonFareClassCombMultiMap* multiMapPtr = iter->second;

      // Get the key from the first entry
      AddonFareClassCombMultiMapIterator multimapIter = multiMapPtr->begin();

      if (multimapIter != multiMapPtr->end())
      {
        // Insert this entry into the map
        AddOnCombFareClassDAO::instance().cache().put(createKey(iter->first), multiMapPtr);
      }
    }
  }
} // AddOnCombFareClassDAO::load()

AddonFareClassCombMultiMap*
AddOnCombFareClassDAO::create(AddOnCombFareClassKey key)
{
  AddonFareClassCombMultiMap* ret = new AddonFareClassCombMultiMap;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(_cacheClass);
  try
  {
    QueryGetCombFareClass cfc(dbAdapter->getAdapter());
    cfc.findAddonCombFareClassInfo(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in AddOnCombFareClassDAO::create");
    destroy(key, ret);
    throw;
  }

  return ret;
} // AddOnCombFareClassDAO::create()

void
AddOnCombFareClassDAO::destroy(AddOnCombFareClassKey key, AddonFareClassCombMultiMap* recs)
{
  AddonFareClassCombMultiMap::const_iterator currIter = recs->begin();
  AddonFareClassCombMultiMap::const_iterator endIter = recs->end();
  for (; currIter != endIter; ++currIter)
  {
    for (AddonCombFareClassInfo* afc: currIter->second)
      delete afc;
  }

  delete recs;
}

AddOnCombFareClassKey
AddOnCombFareClassDAO::createKey(const AddonCombFareClassInfoKey1& key)
{
  return AddOnCombFareClassKey(key.getVendor(), key.getFareTariff(), key.getCarrier());
}

sfc::CompressedData*
AddOnCombFareClassDAO::compress(const AddonFareClassCombMultiMap* map) const
{
  std::vector<AddonCombFareClassInfo*> vect;
  vect.reserve(map->size());
  for (const auto& elem : *map)
  {
    for (auto e: elem.second)
      vect.push_back(e);
  }
  return compressVector(&vect);
}

AddonFareClassCombMultiMap*
AddOnCombFareClassDAO::uncompress(const sfc::CompressedData& compressed) const
{
  const std::vector<AddonCombFareClassInfo*>* vect(
      uncompressVectorPtr<AddonCombFareClassInfo>(compressed));
  AddonFareClassCombMultiMap* map(new AddonFareClassCombMultiMap);
  if (vect)
  {
    for (const auto addon : *vect)
    {
      AddonCombFareClassSpecifiedKey key(addon->fareClass(), addon->owrt());
      AddonFareClassCombMultiMap::mapped_type& pr((*map)[std::move(key)]);
      pr.push_back(addon);
    }
  }
  delete vect;
  return map;
}

#else

const AddonFareClassCombMultiMap&
getAddOnCombFareClassData(const VendorCode& vendor,
                          const TariffNumber& fareTariff,
                          const CarrierCode& carrier,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    AddOnCombFareClassHistoricalDAO& dao = AddOnCombFareClassHistoricalDAO::instance();
    const std::vector<AddonCombFareClassInfo*>& vect(
        dao.get(deleteList, vendor, fareTariff, carrier, ticketDate));
    AddonFareClassCombMultiMap* map(new AddonFareClassCombMultiMap);
    deleteList.adopt(map);
    // Make a local copy
    // date (travelDate) is invalid, so just check ticketDate using ICurrentH
    IsCurrentH<AddonCombFareClassInfo> pred(ticketDate);
    std::vector<AddonCombFareClassInfo*>::const_iterator it(vect.begin()), itEnd(vect.end());
    while (it != itEnd)
    {
      AddonCombFareClassInfo* addon(*it);
      if (pred(addon))
      {
        AddonCombFareClassInfoKey key(
            addon->addonFareClass(), addon->geoAppl(), addon->owrt(), addon->fareClass());
        AddonFareClassCombMultiMap::value_type pr(key, addon);
        map->insert(boost::move(pr));
      }
      ++it;
    }
    return *map;
  }
  else
  {
    AddOnCombFareClassDAO& dao = AddOnCombFareClassDAO::instance();
    return dao.get(deleteList, vendor, fareTariff, carrier);
  }
} // getAddOnCombFareClassData()

const AddonFareClassCombMultiMap&
AddOnCombFareClassDAO::get(DeleteList& del,
                           const VendorCode& vendor,
                           const TariffNumber& fareTariff,
                           const CarrierCode& carrier)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  AddOnCombFareClassKey key(vendor, fareTariff, carrier);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  return *ptr;
}

void
AddOnCombFareClassDAO::load()
{
  AddonFareClassVCCombMap maps;
  Loader<QueryGetAllCombFareClass, AddOnCombFareClassDAO, AddonFareClassVCCombMap> loader(maps);
  if (loader.successful() && (!loader.gotFromLDC()))
  {
    for (AddonFareClassVCCombMap::const_iterator iter = maps.begin(); iter != maps.end(); ++iter)
    {
      AddonFareClassCombMultiMap* multiMapPtr = iter->second;

      // Get the key from the first entry
      AddonFareClassCombMultiMapIterator multimapIter = multiMapPtr->begin();

      if (multimapIter != multiMapPtr->end())
      {
        AddonCombFareClassInfo* info = multimapIter->second;
        AddOnCombFareClassKey key(createKey(info));

        // Insert this entry into the map
        AddOnCombFareClassDAO::instance().cache().put(key, multiMapPtr);
      }
    }
  }
} // AddOnCombFareClassDAO::load()

AddonFareClassCombMultiMap*
AddOnCombFareClassDAO::create(AddOnCombFareClassKey key)
{
  AddonFareClassCombMultiMap* ret = new AddonFareClassCombMultiMap;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(_cacheClass);
  try
  {
    QueryGetCombFareClass cfc(dbAdapter->getAdapter());
    cfc.findAddonCombFareClassInfo(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in AddOnCombFareClassDAO::create");
    destroy(key, ret);
    throw;
  }

  return ret;
} // AddOnCombFareClassDAO::create()

void
AddOnCombFareClassDAO::destroy(AddOnCombFareClassKey key, AddonFareClassCombMultiMap* recs)
{
  AddonFareClassCombMultiMap::const_iterator currIter = recs->begin();
  AddonFareClassCombMultiMap::const_iterator endIter = recs->end();
  for (; currIter != endIter; ++currIter)
  {
    delete currIter->second;
  }

  delete recs;
}

AddOnCombFareClassKey
AddOnCombFareClassDAO::createKey(AddonCombFareClassInfo* info)
{
  return AddOnCombFareClassKey(info->vendor(), info->fareTariff(), info->carrier());
}

sfc::CompressedData*
AddOnCombFareClassDAO::compress(const AddonFareClassCombMultiMap* map) const
{
  std::vector<AddonCombFareClassInfo*> vect;
  vect.reserve(map->size());
  for (const auto& elem : *map)
  {
    vect.push_back(elem.second);
  }
  return compressVector(&vect);
}

AddonFareClassCombMultiMap*
AddOnCombFareClassDAO::uncompress(const sfc::CompressedData& compressed) const
{
  const std::vector<AddonCombFareClassInfo*>* vect(
      uncompressVectorPtr<AddonCombFareClassInfo>(compressed));
  AddonFareClassCombMultiMap* map(new AddonFareClassCombMultiMap);
  if (vect)
  {
    for (const auto addon : *vect)
    {
      AddonCombFareClassInfoKey key(
          addon->addonFareClass(), addon->geoAppl(), addon->owrt(), addon->fareClass());
      AddonFareClassCombMultiMap::value_type pr(key, addon);
      map->insert(boost::move(pr));
    }
  }
  delete vect;
  return map;
}

#endif

std::string
AddOnCombFareClassDAO::_name("AddOnCombFareClass");
std::string
AddOnCombFareClassDAO::_cacheClass("Fares");
DAOHelper<AddOnCombFareClassDAO>
AddOnCombFareClassDAO::_helper(_name);
AddOnCombFareClassDAO* AddOnCombFareClassDAO::_instance = nullptr;

// Historical Stuff
// //////////////////////////////////////////////////////////////////////////////////////////////////////////////

const std::vector<AddonCombFareClassInfo*>&
getAddOnCombFareClassHistoricalData(const VendorCode& vendor,
                                    TariffNumber fareTariff,
                                    const CarrierCode& carrier,
                                    DeleteList& deleteList,
                                    const DateTime& ticketDate)
{
  static std::vector<AddonCombFareClassInfo*> emptyVector;
  AddOnCombFareClassHistoricalDAO& dao(AddOnCombFareClassHistoricalDAO::instance());
  const std::vector<AddonCombFareClassInfo*>& vect(
      dao.get(deleteList, vendor, fareTariff, carrier, ticketDate));
  std::vector<AddonCombFareClassInfo*> tmp;
  tmp.reserve(vect.size());
  // date (travelDate) is invalid, so just check ticketDate using ICurrentH
  IsNotCurrentH<AddonCombFareClassInfo> pred(ticketDate);
  boost::remove_copy_if(vect, std::back_inserter(tmp), pred);
  if (tmp.empty())
  {
    return emptyVector;
  }
  else if (tmp.size() == vect.size())
  {
    return vect;
  }
  else
  {
    std::vector<AddonCombFareClassInfo*>* result(
        new std::vector<AddonCombFareClassInfo*>(tmp.size()));
    deleteList.adopt(result);
    std::copy(tmp.begin(), tmp.end(), result->begin());
    // std::cerr << "tmp.size()=" << tmp.size() << " ratio=" << (double(result->size()) /
    // vect.size()) << std::endl;
    return *result;
  }
} // getAddOnCombFareClassHistoricalData

log4cxx::LoggerPtr
AddOnCombFareClassHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.AddOnCombFareClassHistoricalDAO"));

AddOnCombFareClassHistoricalDAO&
AddOnCombFareClassHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<AddonCombFareClassInfo*>&
AddOnCombFareClassHistoricalDAO::get(DeleteList& del,
                                     const VendorCode& vendor,
                                     const TariffNumber& fareTariff,
                                     const CarrierCode& carrier,
                                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  AddOnCombFareClassHistoricalKey key(vendor, fareTariff, carrier);
  DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  DAOCache::pointer_type ptr(cache().get(key));
  del.copy(ptr);

  return *ptr;
}

std::vector<AddonCombFareClassInfo*>*
AddOnCombFareClassHistoricalDAO::create(AddOnCombFareClassHistoricalKey key)
{
  std::vector<AddonCombFareClassInfo*>* ret(new std::vector<AddonCombFareClassInfo*>);

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetCombFareClassHistorical cfc(dbAdapter->getAdapter());
    cfc.findAddonCombFareClassInfo(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in AddOnCombFareClassHistoricalDAO::create");
    destroy(key, ret);
    throw;
  }
  SortACFCKeyLess keyLess;
  boost::stable_sort(*ret, keyLess);

  return ret;
}

void
AddOnCombFareClassHistoricalDAO::destroy(AddOnCombFareClassHistoricalKey key,
                                         std::vector<AddonCombFareClassInfo*>* recs)
{
  std::vector<AddonCombFareClassInfo*>::const_iterator currIter(recs->begin());
  std::vector<AddonCombFareClassInfo*>::const_iterator endIter(recs->end());
  for (; currIter != endIter; ++currIter)
  {
    delete *currIter;
  }
  delete recs;
}

sfc::CompressedData*
AddOnCombFareClassHistoricalDAO::compress(const std::vector<AddonCombFareClassInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<AddonCombFareClassInfo*>*
AddOnCombFareClassHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<AddonCombFareClassInfo>(compressed);
}

std::string
AddOnCombFareClassHistoricalDAO::_name("AddOnCombFareClassHistorical");
std::string
AddOnCombFareClassHistoricalDAO::_cacheClass("Fares");
DAOHelper<AddOnCombFareClassHistoricalDAO>
AddOnCombFareClassHistoricalDAO::_helper(_name);
AddOnCombFareClassHistoricalDAO* AddOnCombFareClassHistoricalDAO::_instance = nullptr;

} // namespace tse
