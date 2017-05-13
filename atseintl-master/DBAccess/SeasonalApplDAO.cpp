//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/SeasonalApplDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetSeasonalAppl.h"
#include "DBAccess/SeasonalAppl.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

namespace tse
{
log4cxx::LoggerPtr
SeasonalApplDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SeasonalApplDAO"));
SeasonalApplDAO&
SeasonalApplDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const SeasonalAppl*
getSeasonalApplData(const VendorCode& vendor,
                    int itemNo,
                    DeleteList& deleteList,
                    const DateTime& ticketDate,
                    bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    SeasonalApplHistoricalDAO& dao = SeasonalApplHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    SeasonalApplDAO& dao = SeasonalApplDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const SeasonalAppl*
SeasonalApplDAO::get(DeleteList& del,
                     const VendorCode& vendor,
                     int itemNo,
                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  SeasonalApplKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (UNLIKELY(ptr->empty()))
    return nullptr;

  del.copy(ptr);

  IsCurrentG<SeasonalAppl> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (LIKELY(isCurrent(*iter) && NotInhibit<SeasonalAppl>(*iter)))
      return *iter;
  }
  return nullptr;
}

std::vector<SeasonalAppl*>*
SeasonalApplDAO::create(SeasonalApplKey key)
{
  std::vector<SeasonalAppl*>* ret = new std::vector<SeasonalAppl*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSeasonalAppl sa(dbAdapter->getAdapter());
    sa.findSeasonalAppl(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SeasonalApplDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SeasonalApplDAO::destroy(SeasonalApplKey key, std::vector<SeasonalAppl*>* recs)
{
  std::vector<SeasonalAppl*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

SeasonalApplKey
SeasonalApplDAO::createKey(SeasonalAppl* info)
{
  return SeasonalApplKey(info->vendor(), info->itemNo());
}

sfc::CompressedData*
SeasonalApplDAO::compress(const std::vector<SeasonalAppl*>* vect) const
{
  return compressVector(vect);
}

std::vector<SeasonalAppl*>*
SeasonalApplDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<SeasonalAppl>(compressed);
}

void
SeasonalApplDAO::load()
{
  StartupLoaderNoDB<SeasonalAppl, SeasonalApplDAO>();
}

std::string
SeasonalApplDAO::_name("SeasonalAppl");
std::string
SeasonalApplDAO::_cacheClass("Rules");
DAOHelper<SeasonalApplDAO>
SeasonalApplDAO::_helper(_name);
SeasonalApplDAO* SeasonalApplDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: SeasonalApplHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
SeasonalApplHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SeasonalApplHistoricalDAO"));
SeasonalApplHistoricalDAO&
SeasonalApplHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const SeasonalAppl*
SeasonalApplHistoricalDAO::get(DeleteList& del,
                               const VendorCode& vendor,
                               int itemNo,
                               const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  SeasonalApplHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<SeasonalAppl> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<SeasonalAppl>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<SeasonalAppl*>*
SeasonalApplHistoricalDAO::create(SeasonalApplHistoricalKey key)
{
  std::vector<SeasonalAppl*>* ret = new std::vector<SeasonalAppl*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSeasonalApplHistorical sa(dbAdapter->getAdapter());
    sa.findSeasonalAppl(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SeasonalApplHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SeasonalApplHistoricalDAO::destroy(SeasonalApplHistoricalKey key, std::vector<SeasonalAppl*>* recs)
{
  std::vector<SeasonalAppl*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

SeasonalApplHistoricalKey
SeasonalApplHistoricalDAO::createKey(SeasonalAppl* info,
                                     const DateTime& startDate,
                                     const DateTime& endDate)
{
  return SeasonalApplHistoricalKey(info->vendor(), info->itemNo(), startDate, endDate);
}

void
SeasonalApplHistoricalDAO::load()
{
  StartupLoaderNoDB<SeasonalAppl, SeasonalApplHistoricalDAO>();
}

sfc::CompressedData*
SeasonalApplHistoricalDAO::compress(const std::vector<SeasonalAppl*>* vect) const
{
  return compressVector(vect);
}

std::vector<SeasonalAppl*>*
SeasonalApplHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<SeasonalAppl>(compressed);
}

std::string
SeasonalApplHistoricalDAO::_name("SeasonalApplHistorical");
std::string
SeasonalApplHistoricalDAO::_cacheClass("Rules");
DAOHelper<SeasonalApplHistoricalDAO>
SeasonalApplHistoricalDAO::_helper(_name);
SeasonalApplHistoricalDAO* SeasonalApplHistoricalDAO::_instance = nullptr;

} // namespace tse
