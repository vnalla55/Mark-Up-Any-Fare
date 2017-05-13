//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/SurchargesDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetSurchargesInfo.h"
#include "DBAccess/SurchargesInfo.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

namespace tse
{
log4cxx::LoggerPtr
SurchargesDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SurchargesDAO"));
SurchargesDAO&
SurchargesDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const SurchargesInfo*
getSurchargesData(const VendorCode& vendor,
                  int itemNo,
                  DeleteList& deleteList,
                  const DateTime& ticketDate,
                  bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    SurchargesHistoricalDAO& dao = SurchargesHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    SurchargesDAO& dao = SurchargesDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const SurchargesInfo*
SurchargesDAO::get(DeleteList& del,
                   const VendorCode& vendor,
                   int itemNo,
                   const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  SurchargesKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (UNLIKELY(ptr->empty()))
    return nullptr;

  del.copy(ptr);

  IsCurrentG<SurchargesInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (LIKELY(isCurrent(*iter) && NotInhibit<SurchargesInfo>(*iter)))
      return *iter;
  }
  return nullptr;
}

std::vector<SurchargesInfo*>*
SurchargesDAO::create(SurchargesKey key)
{
  std::vector<SurchargesInfo*>* ret = new std::vector<SurchargesInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSurchargesInfo si(dbAdapter->getAdapter());
    si.findSurchargesInfo(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SurchargesDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SurchargesDAO::destroy(SurchargesKey key, std::vector<SurchargesInfo*>* recs)
{
  std::vector<SurchargesInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

SurchargesKey
SurchargesDAO::createKey(SurchargesInfo* info)
{
  return SurchargesKey(info->vendor(), info->itemNo());
}

void
SurchargesDAO::load()
{
  StartupLoaderNoDB<SurchargesInfo, SurchargesDAO>();
}

sfc::CompressedData*
SurchargesDAO::compress(const std::vector<SurchargesInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<SurchargesInfo*>*
SurchargesDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<SurchargesInfo>(compressed);
}

std::string
SurchargesDAO::_name("Surcharges");
std::string
SurchargesDAO::_cacheClass("Rules");
DAOHelper<SurchargesDAO>
SurchargesDAO::_helper(_name);
SurchargesDAO* SurchargesDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: SurchargesHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
SurchargesHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SurchargesHistoricalDAO"));
SurchargesHistoricalDAO&
SurchargesHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const SurchargesInfo*
SurchargesHistoricalDAO::get(DeleteList& del,
                             const VendorCode& vendor,
                             int itemNo,
                             const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  SurchargesHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<SurchargesInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<SurchargesInfo>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<SurchargesInfo*>*
SurchargesHistoricalDAO::create(SurchargesHistoricalKey key)
{
  std::vector<SurchargesInfo*>* ret = new std::vector<SurchargesInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSurchargesInfoHistorical si(dbAdapter->getAdapter());
    si.findSurchargesInfo(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SurchargesHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SurchargesHistoricalDAO::destroy(SurchargesHistoricalKey key, std::vector<SurchargesInfo*>* recs)
{
  std::vector<SurchargesInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

SurchargesHistoricalKey
SurchargesHistoricalDAO::createKey(const SurchargesInfo* info,
                                   const DateTime& startDate,
                                   const DateTime& endDate)
{
  return SurchargesHistoricalKey(info->vendor(), info->itemNo(), startDate, endDate);
}

void
SurchargesHistoricalDAO::load()
{
  StartupLoaderNoDB<SurchargesInfo, SurchargesHistoricalDAO>();
}

sfc::CompressedData*
SurchargesHistoricalDAO::compress(const std::vector<SurchargesInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<SurchargesInfo*>*
SurchargesHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<SurchargesInfo>(compressed);
}

std::string
SurchargesHistoricalDAO::_name("SurchargesHistorical");
std::string
SurchargesHistoricalDAO::_cacheClass("Rules");
DAOHelper<SurchargesHistoricalDAO>
SurchargesHistoricalDAO::_helper(_name);
SurchargesHistoricalDAO* SurchargesHistoricalDAO::_instance = nullptr;

} // namespace tse
