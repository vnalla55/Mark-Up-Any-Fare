//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/FareByRuleItemDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/Queries/QueryGetFBRItem.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <algorithm>
#include <functional>

namespace tse
{

log4cxx::LoggerPtr
FareByRuleItemDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FareByRuleItemDAO"));

FareByRuleItemDAO&
FareByRuleItemDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const FareByRuleItemInfo*
getFareByRuleItemData(const VendorCode& vendor,
                      const int itemNo,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    FareByRuleItemHistoricalDAO& dao = FareByRuleItemHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    FareByRuleItemDAO& dao = FareByRuleItemDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const FareByRuleItemInfo*
FareByRuleItemDAO::get(DeleteList& del,
                       const VendorCode& vendor,
                       int itemNo,
                       const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  FareByRuleItemKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentG<FareByRuleItemInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (LIKELY(isCurrent(*iter) && NotInhibit<FareByRuleItemInfo>(*iter)))
      return *iter;
  }
  return nullptr;
}

std::vector<FareByRuleItemInfo*>*
FareByRuleItemDAO::create(FareByRuleItemKey key)
{
  std::vector<FareByRuleItemInfo*>* ret = new std::vector<FareByRuleItemInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFBRItem fbr(dbAdapter->getAdapter());
    fbr.findFareByRuleItemInfo(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareByRuleItemDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FareByRuleItemDAO::destroy(FareByRuleItemKey key, std::vector<FareByRuleItemInfo*>* recs)
{
  std::vector<FareByRuleItemInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

FareByRuleItemKey
FareByRuleItemDAO::createKey(FareByRuleItemInfo* info)
{
  return FareByRuleItemKey(info->vendor(), info->itemNo());
}

void
FareByRuleItemDAO::load()
{
  StartupLoaderNoDB<FareByRuleItemInfo, FareByRuleItemDAO>();
}

sfc::CompressedData*
FareByRuleItemDAO::compress(const std::vector<FareByRuleItemInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<FareByRuleItemInfo*>*
FareByRuleItemDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<FareByRuleItemInfo>(compressed);
}

std::string
FareByRuleItemDAO::_name("FareByRuleItem");
std::string
FareByRuleItemDAO::_cacheClass("Fares");
DAOHelper<FareByRuleItemDAO>
FareByRuleItemDAO::_helper(_name);
FareByRuleItemDAO* FareByRuleItemDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: FareByRuleItemHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
FareByRuleItemHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareByRuleItemHistoricalDAO"));
FareByRuleItemHistoricalDAO&
FareByRuleItemHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const FareByRuleItemInfo*
FareByRuleItemHistoricalDAO::get(DeleteList& del,
                                 const VendorCode& vendor,
                                 int itemNo,
                                 const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  FareByRuleItemHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<FareByRuleItemInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<FareByRuleItemInfo>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<FareByRuleItemInfo*>*
FareByRuleItemHistoricalDAO::create(FareByRuleItemHistoricalKey key)
{
  std::vector<FareByRuleItemInfo*>* ret = new std::vector<FareByRuleItemInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);

  try
  {
    QueryGetFBRItemHistorical fbr(dbAdapter->getAdapter());
    fbr.findFareByRuleItemInfo(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareByRuleItemHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

FareByRuleItemHistoricalKey
FareByRuleItemHistoricalDAO::createKey(FareByRuleItemInfo* info,
                                       const DateTime& startDate,
                                       const DateTime& endDate)
{
  return FareByRuleItemHistoricalKey(info->vendor(), info->itemNo(), startDate, endDate);
}

void
FareByRuleItemHistoricalDAO::load()
{
  StartupLoaderNoDB<FareByRuleItemInfo, FareByRuleItemHistoricalDAO>();
}

void
FareByRuleItemHistoricalDAO::destroy(FareByRuleItemHistoricalKey key,
                                     std::vector<FareByRuleItemInfo*>* recs)
{
  std::vector<FareByRuleItemInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
FareByRuleItemHistoricalDAO::compress(const std::vector<FareByRuleItemInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<FareByRuleItemInfo*>*
FareByRuleItemHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<FareByRuleItemInfo>(compressed);
}

std::string
FareByRuleItemHistoricalDAO::_name("FareByRuleItemHistorical");
std::string
FareByRuleItemHistoricalDAO::_cacheClass("Fares");
DAOHelper<FareByRuleItemHistoricalDAO>
FareByRuleItemHistoricalDAO::_helper(_name);
FareByRuleItemHistoricalDAO* FareByRuleItemHistoricalDAO::_instance = nullptr;

} // namespace tse
