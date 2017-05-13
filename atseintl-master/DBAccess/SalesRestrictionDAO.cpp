//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/SalesRestrictionDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetSalesRestriction.h"
#include "DBAccess/SalesRestriction.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

namespace tse
{
log4cxx::LoggerPtr
SalesRestrictionDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SalesRestrictionDAO"));
SalesRestrictionDAO&
SalesRestrictionDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const SalesRestriction*
getSalesRestrictionData(const VendorCode& vendor,
                        int itemNo,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    SalesRestrictionHistoricalDAO& dao = SalesRestrictionHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    SalesRestrictionDAO& dao = SalesRestrictionDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const SalesRestriction*
SalesRestrictionDAO::get(DeleteList& del,
                         const VendorCode& vendor,
                         int itemNo,
                         const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  SalesRestrictionKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentG<SalesRestriction> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (LIKELY(isCurrent(*iter) && NotInhibit<SalesRestriction>(*iter)))
      return *iter;
  }
  return nullptr;
}

std::vector<SalesRestriction*>*
SalesRestrictionDAO::create(SalesRestrictionKey key)
{
  std::vector<SalesRestriction*>* ret = new std::vector<SalesRestriction*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSalesRestriction sr(dbAdapter->getAdapter());
    sr.findSalesRestriction(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SalesRestrictionDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SalesRestrictionDAO::destroy(SalesRestrictionKey key, std::vector<SalesRestriction*>* recs)
{
  std::vector<SalesRestriction*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
SalesRestrictionDAO::compress(const std::vector<SalesRestriction*>* vect) const
{
  return compressVector(vect);
}

std::vector<SalesRestriction*>*
SalesRestrictionDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<SalesRestriction>(compressed);
}

std::string
SalesRestrictionDAO::_name("SalesRestriction");
std::string
SalesRestrictionDAO::_cacheClass("Rules");
DAOHelper<SalesRestrictionDAO>
SalesRestrictionDAO::_helper(_name);
SalesRestrictionDAO* SalesRestrictionDAO::_instance = nullptr;

SalesRestrictionKey
SalesRestrictionDAO::createKey(SalesRestriction* info)
{
  return SalesRestrictionKey(info->vendor(), info->itemNo());
}

void
SalesRestrictionDAO::load()
{
  StartupLoaderNoDB<SalesRestriction, SalesRestrictionDAO>();
}

// --------------------------------------------------
// Historical DAO: SalesRestrictionHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
SalesRestrictionHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SalesRestrictionHistoricalDAO"));
SalesRestrictionHistoricalDAO&
SalesRestrictionHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const SalesRestriction*
SalesRestrictionHistoricalDAO::get(DeleteList& del,
                                   const VendorCode& vendor,
                                   int itemNo,
                                   const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  SalesRestrictionHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<SalesRestriction> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<SalesRestriction>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<SalesRestriction*>*
SalesRestrictionHistoricalDAO::create(SalesRestrictionHistoricalKey key)
{
  std::vector<SalesRestriction*>* ret = new std::vector<SalesRestriction*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);

  try
  {
    QueryGetSalesRestrictionHistorical sr(dbAdapter->getAdapter());
    sr.findSalesRestriction(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SalesRestrictionHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

SalesRestrictionHistoricalKey
SalesRestrictionHistoricalDAO::createKey(SalesRestriction* info,
                                         const DateTime& startDate,
                                         const DateTime& endDate)
{
  return SalesRestrictionHistoricalKey(info->vendor(), info->itemNo(), startDate, endDate);
}

void
SalesRestrictionHistoricalDAO::load()
{
  StartupLoaderNoDB<SalesRestriction, SalesRestrictionHistoricalDAO>();
}

void
SalesRestrictionHistoricalDAO::destroy(SalesRestrictionHistoricalKey key,
                                       std::vector<SalesRestriction*>* recs)
{
  std::vector<SalesRestriction*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
SalesRestrictionHistoricalDAO::compress(const std::vector<SalesRestriction*>* vect) const
{
  return compressVector(vect);
}

std::vector<SalesRestriction*>*
SalesRestrictionHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<SalesRestriction>(compressed);
}

std::string
SalesRestrictionHistoricalDAO::_name("SalesRestrictionHistorical");
std::string
SalesRestrictionHistoricalDAO::_cacheClass("Rules");
DAOHelper<SalesRestrictionHistoricalDAO>
SalesRestrictionHistoricalDAO::_helper(_name);
SalesRestrictionHistoricalDAO* SalesRestrictionHistoricalDAO::_instance = nullptr;

} // namespace tse
