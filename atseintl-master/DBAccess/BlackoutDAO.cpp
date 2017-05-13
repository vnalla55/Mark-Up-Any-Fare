//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/BlackoutDAO.h"

#include "Common/Logger.h"
#include "DBAccess/BlackoutInfo.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetBlackout.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
BlackoutDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.BlackoutDAO"));

BlackoutDAO&
BlackoutDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const BlackoutInfo*
getBlackoutData(const VendorCode& vendor,
                int itemNo,
                DeleteList& deleteList,
                const DateTime& ticketDate,
                bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    BlackoutHistoricalDAO& dao = BlackoutHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    BlackoutDAO& dao = BlackoutDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const BlackoutInfo*
BlackoutDAO::get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  BlackoutKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (UNLIKELY(ptr->empty()))
    return nullptr;

  del.copy(ptr);

  IsCurrentG<BlackoutInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (LIKELY(isCurrent(*iter) && NotInhibit<BlackoutInfo>(*iter)))
      return *iter;
  }
  return nullptr;
}

std::vector<BlackoutInfo*>*
BlackoutDAO::create(BlackoutKey key)
{
  std::vector<BlackoutInfo*>* ret = new std::vector<BlackoutInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetBlackout bo(dbAdapter->getAdapter());
    bo.findBlackoutInfo(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in BlackoutDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
BlackoutDAO::destroy(BlackoutKey key, std::vector<BlackoutInfo*>* recs)
{
  std::vector<BlackoutInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

BlackoutKey
BlackoutDAO::createKey(BlackoutInfo* info)
{
  return BlackoutKey(info->vendor(), info->itemNo());
}

void
BlackoutDAO::load()
{
  StartupLoaderNoDB<BlackoutInfo, BlackoutDAO>();
}

sfc::CompressedData*
BlackoutDAO::compress(const std::vector<BlackoutInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<BlackoutInfo*>*
BlackoutDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<BlackoutInfo>(compressed);
}

std::string
BlackoutDAO::_name("Blackout");
std::string
BlackoutDAO::_cacheClass("Rules");
DAOHelper<BlackoutDAO>
BlackoutDAO::_helper(_name);
BlackoutDAO* BlackoutDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: BlackoutHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
BlackoutHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.BlackoutHistoricalDAO"));
BlackoutHistoricalDAO&
BlackoutHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const BlackoutInfo*
BlackoutHistoricalDAO::get(DeleteList& del,
                           const VendorCode& vendor,
                           int itemNo,
                           const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  BlackoutHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<BlackoutInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<BlackoutInfo>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<BlackoutInfo*>*
BlackoutHistoricalDAO::create(BlackoutHistoricalKey key)
{
  std::vector<BlackoutInfo*>* ret = new std::vector<BlackoutInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetBlackoutHistorical bo(dbAdapter->getAdapter());
    bo.findBlackoutInfo(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in BlackoutHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
BlackoutHistoricalDAO::destroy(BlackoutHistoricalKey key, std::vector<BlackoutInfo*>* recs)
{
  std::vector<BlackoutInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
BlackoutHistoricalDAO::compress(const std::vector<BlackoutInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<BlackoutInfo*>*
BlackoutHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<BlackoutInfo>(compressed);
}

std::string
BlackoutHistoricalDAO::_name("BlackoutHistorical");
std::string
BlackoutHistoricalDAO::_cacheClass("Rules");
DAOHelper<BlackoutHistoricalDAO>
BlackoutHistoricalDAO::_helper(_name);

BlackoutHistoricalDAO* BlackoutHistoricalDAO::_instance = nullptr;

} // namespace tse
