//-------------------------------------------------------------------------------
// Copyright 2004, 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/ZoneDAO.h"

#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetZone.h"
#include "DBAccess/ZoneInfo.h"

#include <algorithm>
#include <functional>

namespace tse
{

log4cxx::LoggerPtr
ZoneDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.ZoneDAO"));

ZoneDAO&
ZoneDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const ZoneInfo*
getZoneFareFocusGroupData(const VendorCode& vendor,
                          const Zone& zone,
                          Indicator zoneType,
                          const DateTime& date,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical,
                          bool fareFocusGroup)
{
  if (isHistorical)
  {
    ZoneHistoricalDAO& dao = ZoneHistoricalDAO::instance();
    const ZoneInfo* curr = dao.get(deleteList, vendor, zone, zoneType, date, ticketDate, fareFocusGroup);
    // ***
    // remove the next two lines when the general tables have been copied
    // to the historical database and replace "const ZoneInfo* curr = " with return
    if (curr)
      return curr;
    return getZoneFareFocusGroupData(vendor, zone, zoneType, date, deleteList, ticketDate, false, fareFocusGroup);
    // ***
  }
  else
  {
    ZoneDAO& dao = ZoneDAO::instance();
    return dao.get(deleteList, vendor, zone, zoneType, date, ticketDate, fareFocusGroup);
  }
}

const ZoneInfo*
getZoneData(const VendorCode& vendor,
            const Zone& zone,
            Indicator zoneType,
            const DateTime& date,
            DeleteList& deleteList,
            const DateTime& ticketDate,
            bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    ZoneHistoricalDAO& dao = ZoneHistoricalDAO::instance();
    const ZoneInfo* curr = dao.get(deleteList, vendor, zone, zoneType, date, ticketDate);
    // ***
    // remove the next two lines when the general tables have been copied
    // to the historical database and replace "const ZoneInfo* curr = " with return
    if (curr)
      return curr;
    return getZoneData(vendor, zone, zoneType, date, deleteList, ticketDate, false);
    // ***
  }
  else
  {
    ZoneDAO& dao = ZoneDAO::instance();
    return dao.get(deleteList, vendor, zone, zoneType, date, ticketDate);
  }
}

const ZoneInfo*
ZoneDAO::get(DeleteList& del,
             const VendorCode& vendor,
             const Zone& zone,
             Indicator zoneType,
             const DateTime& date,
             const DateTime& ticketDate,
             bool fareFocusGroup)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  ZoneKey key(vendor, zone, zoneType);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  const ZoneInfo* ret = nullptr;
  if (UNLIKELY(fareFocusGroup))
  {
    DAOCache::value_type::iterator i =
        find_if(ptr->begin(), ptr->end(), IsCurrentG<ZoneInfo>(ticketDate));
    if (i != ptr->end())
      ret = *i;
  }
  else
  {
    DAOCache::value_type::iterator i =
        find_if(ptr->begin(), ptr->end(), IsEffectiveG<ZoneInfo>(date, ticketDate));
    if (i != ptr->end())
      ret = *i;
  }
  return ret;
}

std::vector<const ZoneInfo*>*
ZoneDAO::create(ZoneKey key)
{
  std::vector<const ZoneInfo*>* ret = new std::vector<const ZoneInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetZone zn(dbAdapter->getAdapter());

    zn.findZone(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in ZoneDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
ZoneDAO::destroy(ZoneKey key, std::vector<const ZoneInfo*>* recs)
{
  std::vector<const ZoneInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i; // lint !e605
  delete recs;
}

std::string
ZoneDAO::_name("Zone");
std::string
ZoneDAO::_cacheClass("Common");

DAOHelper<ZoneDAO>
ZoneDAO::_helper(_name);

ZoneDAO* ZoneDAO::_instance = nullptr;

ZoneKey
ZoneDAO::createKey(const ZoneInfo* info)
{
  return ZoneKey(info->vendor(), info->zone(), info->zoneType());
}

void
ZoneDAO::load()
{
  StartupLoaderNoDB<ZoneInfo, ZoneDAO>();
}

sfc::CompressedData*
ZoneDAO::compress(const std::vector<const ZoneInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<const ZoneInfo*>*
ZoneDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<const ZoneInfo>(compressed);
}

////////////////////////////////////
// Historical DAO: ZoneHistoricalDAO
////////////////////////////////////

log4cxx::LoggerPtr
ZoneHistoricalDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.ZoneHistoricalDAO"));

ZoneHistoricalDAO&
ZoneHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const ZoneInfo*
ZoneHistoricalDAO::get(DeleteList& del,
                       const VendorCode& vendor,
                       const Zone& zone,
                       Indicator zoneType,
                       const DateTime& date,
                       const DateTime& ticketDate,
                       bool fareFocusGroup)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  ZoneHistoricalKey key(vendor, zone, zoneType);
  DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  const ZoneInfo* ret = nullptr;

  if (fareFocusGroup)
  {
    DAOCache::value_type::iterator i =
        find_if(ptr->begin(), ptr->end(), IsCurrentDH<ZoneInfo>(ticketDate));
    if (i != ptr->end())
      ret = *i;
  }
  else
  {
    DAOCache::value_type::iterator i =
        find_if(ptr->begin(), ptr->end(), IsEffectiveHist<ZoneInfo>(date, ticketDate));
    if (i != ptr->end())
      ret = *i;
  }
  return ret;
}

std::vector<const ZoneInfo*>*
ZoneHistoricalDAO::create(ZoneHistoricalKey key)
{
  std::vector<const ZoneInfo*>* ret = new std::vector<const ZoneInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetZoneHistorical zn(dbAdapter->getAdapter());
    zn.findZone(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in ZoneHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

void
ZoneHistoricalDAO::destroy(ZoneHistoricalKey key, std::vector<const ZoneInfo*>* recs)
{
  std::vector<const ZoneInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i; // lint !e605
  delete recs;
}

ZoneHistoricalKey
ZoneHistoricalDAO::createKey(const ZoneInfo* info,
                             const DateTime& startDate,
                             const DateTime& endDate)
{
  return ZoneHistoricalKey(info->vendor(), info->zone(), info->zoneType(), startDate, endDate);
}

void
ZoneHistoricalDAO::load()
{
  StartupLoaderNoDB<ZoneInfo, ZoneHistoricalDAO>();
}

sfc::CompressedData*
ZoneHistoricalDAO::compress(const std::vector<const ZoneInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<const ZoneInfo*>*
ZoneHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<const ZoneInfo>(compressed);
}

std::string
ZoneHistoricalDAO::_name("ZoneHistorical");
std::string
ZoneHistoricalDAO::_cacheClass("Common");

DAOHelper<ZoneHistoricalDAO>
ZoneHistoricalDAO::_helper(_name);

ZoneHistoricalDAO* ZoneHistoricalDAO::_instance = nullptr;

} // namespace tse
