//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/StopoversDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetStopoversInfo.h"
#include "DBAccess/StopoversInfo.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

namespace tse
{
log4cxx::LoggerPtr
StopoversDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.StopoversDAO"));
StopoversDAO&
StopoversDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const StopoversInfo*
getStopoversData(const VendorCode& vendor,
                 int itemNo,
                 DeleteList& deleteList,
                 const DateTime& ticketDate,
                 bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    StopoversHistoricalDAO& dao = StopoversHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    StopoversDAO& dao = StopoversDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const StopoversInfo*
StopoversDAO::get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  StopoversKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (UNLIKELY(ptr->empty()))
    return nullptr;

  del.copy(ptr);

  IsCurrentG<StopoversInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (LIKELY(isCurrent(*iter) && NotInhibit<StopoversInfo>(*iter)))
      return *iter;
  }
  return nullptr;
}

std::vector<StopoversInfo*>*
StopoversDAO::create(StopoversKey key)
{
  std::vector<StopoversInfo*>* ret = new std::vector<StopoversInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetStopoversInfo si(dbAdapter->getAdapter());
    si.findStopoversInfo(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in StopoversDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
StopoversDAO::destroy(StopoversKey key, std::vector<StopoversInfo*>* recs)
{
  std::vector<StopoversInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

StopoversKey
StopoversDAO::createKey(StopoversInfo* info)
{
  return StopoversKey(info->vendor(), info->itemNo());
}

void
StopoversDAO::load()
{
  StartupLoaderNoDB<StopoversInfo, StopoversDAO>();
}

sfc::CompressedData*
StopoversDAO::compress(const std::vector<StopoversInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<StopoversInfo*>*
StopoversDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<StopoversInfo>(compressed);
}

std::string
StopoversDAO::_name("Stopovers");
std::string
StopoversDAO::_cacheClass("Rules");
DAOHelper<StopoversDAO>
StopoversDAO::_helper(_name);
StopoversDAO* StopoversDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: StopoversHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
StopoversHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.StopoversHistoricalDAO"));
StopoversHistoricalDAO&
StopoversHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const StopoversInfo*
StopoversHistoricalDAO::get(DeleteList& del,
                            const VendorCode& vendor,
                            int itemNo,
                            const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  StopoversHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<StopoversInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<StopoversInfo>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<StopoversInfo*>*
StopoversHistoricalDAO::create(StopoversHistoricalKey key)
{
  std::vector<StopoversInfo*>* ret = new std::vector<StopoversInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetStopoversInfoHistorical si(dbAdapter->getAdapter());
    si.findStopoversInfo(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in StopoversHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
StopoversHistoricalDAO::destroy(StopoversHistoricalKey key, std::vector<StopoversInfo*>* recs)
{
  std::vector<StopoversInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

StopoversHistoricalKey
StopoversHistoricalDAO::createKey(const StopoversInfo* info,
                                  const DateTime& startDate,
                                  const DateTime& endDate)
{
  return StopoversHistoricalKey(info->vendor(), info->itemNo(), startDate, endDate);
}

void
StopoversHistoricalDAO::load()
{
  StartupLoaderNoDB<StopoversInfo, StopoversHistoricalDAO>();
}

sfc::CompressedData*
StopoversHistoricalDAO::compress(const std::vector<StopoversInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<StopoversInfo*>*
StopoversHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<StopoversInfo>(compressed);
}

std::string
StopoversHistoricalDAO::_name("StopoversHistorical");
std::string
StopoversHistoricalDAO::_cacheClass("Rules");
DAOHelper<StopoversHistoricalDAO>
StopoversHistoricalDAO::_helper(_name);
StopoversHistoricalDAO* StopoversHistoricalDAO::_instance = nullptr;

} // namespace tse
