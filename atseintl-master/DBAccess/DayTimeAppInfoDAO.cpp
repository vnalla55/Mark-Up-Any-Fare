//----------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//----------------------------------------------------------------------------
#include "DBAccess/DayTimeAppInfoDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DayTimeAppInfo.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetDayTimeApplications.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
DayTimeAppInfoDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.DayTimeAppInfoDAO"));

DayTimeAppInfoDAO&
DayTimeAppInfoDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const DayTimeAppInfo*
getDayTimeAppInfoData(const VendorCode& vendor,
                      int itemNo,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    DayTimeAppInfoHistoricalDAO& dao = DayTimeAppInfoHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    DayTimeAppInfoDAO& dao = DayTimeAppInfoDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const DayTimeAppInfo*
DayTimeAppInfoDAO::get(DeleteList& del,
                       const VendorCode& vendor,
                       int itemNo,
                       const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  DayTimeAppInfoKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (UNLIKELY(ptr->empty()))
    return nullptr;

  del.copy(ptr);

  IsCurrentG<DayTimeAppInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (LIKELY(isCurrent(*iter) && NotInhibit<DayTimeAppInfo>(*iter)))
      return *iter;
  }
  return nullptr;
}

std::vector<DayTimeAppInfo*>*
DayTimeAppInfoDAO::create(DayTimeAppInfoKey key)
{
  std::vector<DayTimeAppInfo*>* ret = new std::vector<DayTimeAppInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetDayTimeApplications dta(dbAdapter->getAdapter());
    dta.findDayTimeAppInfo(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in DayTimeAppInfoDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
DayTimeAppInfoDAO::destroy(DayTimeAppInfoKey key, std::vector<DayTimeAppInfo*>* recs)
{
  destroyContainer(recs);
}

DayTimeAppInfoKey
DayTimeAppInfoDAO::createKey(DayTimeAppInfo* info)
{
  return DayTimeAppInfoKey(info->vendor(), info->itemNo());
}

void
DayTimeAppInfoDAO::load()
{
  StartupLoaderNoDB<DayTimeAppInfo, DayTimeAppInfoDAO>();
}

sfc::CompressedData*
DayTimeAppInfoDAO::compress(const std::vector<DayTimeAppInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<DayTimeAppInfo*>*
DayTimeAppInfoDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<DayTimeAppInfo>(compressed);
}

std::string
DayTimeAppInfoDAO::_name("DayTimeAppInfo");
std::string
DayTimeAppInfoDAO::_cacheClass("Rules");
DAOHelper<DayTimeAppInfoDAO>
DayTimeAppInfoDAO::_helper(_name);
DayTimeAppInfoDAO* DayTimeAppInfoDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: DayTimeAppInfoHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
DayTimeAppInfoHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.DayTimeAppInfoHistoricalDAO"));
DayTimeAppInfoHistoricalDAO&
DayTimeAppInfoHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const DayTimeAppInfo*
DayTimeAppInfoHistoricalDAO::get(DeleteList& del,
                                 const VendorCode& vendor,
                                 int itemNo,
                                 const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  DayTimeAppInfoHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<DayTimeAppInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<DayTimeAppInfo>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<DayTimeAppInfo*>*
DayTimeAppInfoHistoricalDAO::create(DayTimeAppInfoHistoricalKey key)
{
  std::vector<DayTimeAppInfo*>* ret = new std::vector<DayTimeAppInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetDayTimeApplicationsHistorical dta(dbAdapter->getAdapter());
    dta.findDayTimeAppInfo(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in DayTimeAppInfoHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
DayTimeAppInfoHistoricalDAO::destroy(DayTimeAppInfoHistoricalKey key,
                                     std::vector<DayTimeAppInfo*>* recs)
{
  destroyContainer(recs);
}

sfc::CompressedData*
DayTimeAppInfoHistoricalDAO::compress(const std::vector<DayTimeAppInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<DayTimeAppInfo*>*
DayTimeAppInfoHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<DayTimeAppInfo>(compressed);
}

std::string
DayTimeAppInfoHistoricalDAO::_name("DayTimeAppInfoHistorical");
std::string
DayTimeAppInfoHistoricalDAO::_cacheClass("Rules");
DAOHelper<DayTimeAppInfoHistoricalDAO>
DayTimeAppInfoHistoricalDAO::_helper(_name);
DayTimeAppInfoHistoricalDAO* DayTimeAppInfoHistoricalDAO::_instance = nullptr;

} // namespace tse
