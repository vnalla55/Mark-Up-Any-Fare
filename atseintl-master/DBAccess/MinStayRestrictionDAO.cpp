//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/MinStayRestrictionDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/MinStayRestriction.h"
#include "DBAccess/Queries/QueryGetMinStayRestriction.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
MinStayRestrictionDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.MinStayRestrictionDAO"));

MinStayRestrictionDAO&
MinStayRestrictionDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const MinStayRestriction*
getMinStayRestrictionData(const VendorCode& vendor,
                          int itemNo,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    MinStayRestrictionHistoricalDAO& dao = MinStayRestrictionHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    MinStayRestrictionDAO& dao = MinStayRestrictionDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const MinStayRestriction*
MinStayRestrictionDAO::get(DeleteList& del,
                           const VendorCode& vendor,
                           int itemNo,
                           const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  MinStayRestrictionKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (UNLIKELY(ptr->empty()))
    return nullptr;

  del.copy(ptr);

  IsCurrentG<MinStayRestriction> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (LIKELY(isCurrent(*iter) && NotInhibit<MinStayRestriction>(*iter)))
      return *iter;
  }
  return nullptr;
}

std::vector<MinStayRestriction*>*
MinStayRestrictionDAO::create(MinStayRestrictionKey key)
{
  std::vector<MinStayRestriction*>* ret = new std::vector<MinStayRestriction*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMinStayRestriction msr(dbAdapter->getAdapter());
    msr.findMinStayRestriction(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MinStayRestrictionDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MinStayRestrictionDAO::destroy(MinStayRestrictionKey key, std::vector<MinStayRestriction*>* recs)
{
  std::vector<MinStayRestriction*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

MinStayRestrictionKey
MinStayRestrictionDAO::createKey(MinStayRestriction* info)
{
  return MinStayRestrictionKey(info->vendor(), info->itemNo());
}

void
MinStayRestrictionDAO::load()
{
  StartupLoaderNoDB<MinStayRestriction, MinStayRestrictionDAO>();
}

sfc::CompressedData*
MinStayRestrictionDAO::compress(const std::vector<MinStayRestriction*>* vect) const
{
  return compressVector(vect);
}

std::vector<MinStayRestriction*>*
MinStayRestrictionDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MinStayRestriction>(compressed);
}

std::string
MinStayRestrictionDAO::_name("MinStayRestriction");
std::string
MinStayRestrictionDAO::_cacheClass("Rules");
DAOHelper<MinStayRestrictionDAO>
MinStayRestrictionDAO::_helper(_name);
MinStayRestrictionDAO* MinStayRestrictionDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: MinStayRestrictionHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
MinStayRestrictionHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.MinStayRestrictionHistoricalDAO"));
MinStayRestrictionHistoricalDAO&
MinStayRestrictionHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const MinStayRestriction*
MinStayRestrictionHistoricalDAO::get(DeleteList& del,
                                     const VendorCode& vendor,
                                     int itemNo,
                                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  MinStayRestrictionHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<MinStayRestriction> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<MinStayRestriction>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<MinStayRestriction*>*
MinStayRestrictionHistoricalDAO::create(MinStayRestrictionHistoricalKey key)
{
  std::vector<MinStayRestriction*>* ret = new std::vector<MinStayRestriction*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMinStayRestrictionHistorical msr(dbAdapter->getAdapter());
    msr.findMinStayRestriction(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MinStayRestrictionHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MinStayRestrictionHistoricalDAO::destroy(MinStayRestrictionHistoricalKey key,
                                         std::vector<MinStayRestriction*>* recs)
{
  std::vector<MinStayRestriction*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
MinStayRestrictionHistoricalDAO::compress(const std::vector<MinStayRestriction*>* vect) const
{
  return compressVector(vect);
}

std::vector<MinStayRestriction*>*
MinStayRestrictionHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MinStayRestriction>(compressed);
}

std::string
MinStayRestrictionHistoricalDAO::_name("MinStayRestrictionHistorical");
std::string
MinStayRestrictionHistoricalDAO::_cacheClass("Rules");
DAOHelper<MinStayRestrictionHistoricalDAO>
MinStayRestrictionHistoricalDAO::_helper(_name);
MinStayRestrictionHistoricalDAO* MinStayRestrictionHistoricalDAO::_instance = nullptr;

} // namespace tse
