//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/PenaltyDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/PenaltyInfo.h"
#include "DBAccess/Queries/QueryGetPenaltyInfo.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

namespace tse
{
log4cxx::LoggerPtr
PenaltyDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.PenaltyDAO"));

PenaltyDAO&
PenaltyDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const PenaltyInfo*
getPenaltyData(const VendorCode& vendor,
               int itemNo,
               DeleteList& deleteList,
               const DateTime& ticketDate,
               bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    PenaltyHistoricalDAO& dao = PenaltyHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    PenaltyDAO& dao = PenaltyDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const PenaltyInfo*
PenaltyDAO::get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  PenaltyKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentG<PenaltyInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (LIKELY(isCurrent(*iter) && NotInhibit<PenaltyInfo>(*iter)))
      return *iter;
  }
  return nullptr;
}

std::vector<PenaltyInfo*>*
PenaltyDAO::create(PenaltyKey key)
{
  std::vector<PenaltyInfo*>* ret = new std::vector<PenaltyInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();

  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());

  try
  {
    QueryGetPenaltyInfo pi(dbAdapter->getAdapter());
    pi.findPenaltyInfo(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in PenaltyDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

sfc::CompressedData*
PenaltyDAO::compress(const std::vector<PenaltyInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<PenaltyInfo*>*
PenaltyDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<PenaltyInfo>(compressed);
}

void
PenaltyDAO::destroy(PenaltyKey key, std::vector<PenaltyInfo*>* recs)
{
  std::vector<PenaltyInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

PenaltyKey
PenaltyDAO::createKey(PenaltyInfo* info)
{
  return PenaltyKey(info->vendor(), info->itemNo());
}

void
PenaltyDAO::load()
{
  StartupLoaderNoDB<PenaltyInfo, PenaltyDAO>();
}

std::string
PenaltyDAO::_name("Penalty");
std::string
PenaltyDAO::_cacheClass("Rules");
DAOHelper<PenaltyDAO>
PenaltyDAO::_helper(_name);
PenaltyDAO* PenaltyDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: PenaltyHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
PenaltyHistoricalDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.PenaltyHistoricalDAO"));
PenaltyHistoricalDAO&
PenaltyHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const PenaltyInfo*
PenaltyHistoricalDAO::get(DeleteList& del,
                          const VendorCode& vendor,
                          int itemNo,
                          const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  PenaltyHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<PenaltyInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<PenaltyInfo>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<PenaltyInfo*>*
PenaltyHistoricalDAO::create(PenaltyHistoricalKey key)
{
  std::vector<PenaltyInfo*>* ret = new std::vector<PenaltyInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetPenaltyInfoHistorical pi(dbAdapter->getAdapter());
    pi.findPenaltyInfo(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in PenaltyHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
PenaltyHistoricalDAO::destroy(PenaltyHistoricalKey key, std::vector<PenaltyInfo*>* recs)
{
  std::vector<PenaltyInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
PenaltyHistoricalDAO::compress(const std::vector<PenaltyInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<PenaltyInfo*>*
PenaltyHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<PenaltyInfo>(compressed);
}

PenaltyHistoricalKey
PenaltyHistoricalDAO::createKey(PenaltyInfo* info,
                                const DateTime& startDate,
                                const DateTime& endDate)
{
  return PenaltyHistoricalKey(info->vendor(), info->itemNo(), startDate, endDate);
}

void
PenaltyHistoricalDAO::load()
{
  StartupLoaderNoDB<PenaltyInfo, PenaltyHistoricalDAO>();
}

std::string
PenaltyHistoricalDAO::_name("PenaltyHistorical");
std::string
PenaltyHistoricalDAO::_cacheClass("Rules");
DAOHelper<PenaltyHistoricalDAO>
PenaltyHistoricalDAO::_helper(_name);
PenaltyHistoricalDAO* PenaltyHistoricalDAO::_instance = nullptr;

} // namespace tse
