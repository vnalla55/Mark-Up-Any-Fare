//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/DiscountDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/Queries/QueryGetDiscounts.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
DiscountDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.DiscountDAO"));

DiscountDAO&
DiscountDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const DiscountInfo*
getDiscountData(const VendorCode& vendor,
                int itemNo,
                int category,
                DeleteList& deleteList,
                const DateTime& ticketDate,
                bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    DiscountHistoricalDAO& dao = DiscountHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, category, ticketDate);
  }
  else
  {
    DiscountDAO& dao = DiscountDAO::instance();
    return dao.get(deleteList, vendor, itemNo, category, ticketDate);
  }
}

const DiscountInfo*
DiscountDAO::get(
    DeleteList& del, const VendorCode& vendor, int itemNo, int category, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  DiscountKey key(rcVendor, itemNo, category);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentG<DiscountInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (LIKELY(isCurrent(*iter) && NotInhibit<DiscountInfo>(*iter)))
      return *iter;
  }
  return nullptr;
}

std::vector<DiscountInfo*>*
DiscountDAO::create(DiscountKey key)
{
  std::vector<DiscountInfo*>* ret = new std::vector<DiscountInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetChdDiscount cd(dbAdapter->getAdapter());
    cd.findDiscountInfo(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in DiscountDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
DiscountDAO::destroy(DiscountKey key, std::vector<DiscountInfo*>* recs)
{
  std::vector<DiscountInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

DiscountKey
DiscountDAO::createKey(DiscountInfo* info)
{
  return DiscountKey(info->vendor(), info->itemNo(), info->category());
}

void
DiscountDAO::load()
{
  StartupLoaderNoDB<DiscountInfo, DiscountDAO>();
}

sfc::CompressedData*
DiscountDAO::compress(const std::vector<DiscountInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<DiscountInfo*>*
DiscountDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<DiscountInfo>(compressed);
}

std::string
DiscountDAO::_name("Discount");
std::string
DiscountDAO::_cacheClass("Rules");
DAOHelper<DiscountDAO>
DiscountDAO::_helper(_name);
DiscountDAO* DiscountDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: DiscountHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
DiscountHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.DiscountHistoricalDAO"));
DiscountHistoricalDAO&
DiscountHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const DiscountInfo*
DiscountHistoricalDAO::get(
    DeleteList& del, const VendorCode& vendor, int itemNo, int category, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  DiscountHistoricalKey key(rcVendor, itemNo, category);
  DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<DiscountInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<DiscountInfo>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<DiscountInfo*>*
DiscountHistoricalDAO::create(DiscountHistoricalKey key)
{
  std::vector<DiscountInfo*>* ret = new std::vector<DiscountInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetChdDiscountHistorical cd(dbAdapter->getAdapter());
    cd.findDiscountInfo(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in DiscountHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
DiscountHistoricalDAO::destroy(DiscountHistoricalKey key, std::vector<DiscountInfo*>* recs)
{
  std::vector<DiscountInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
DiscountHistoricalDAO::compress(const std::vector<DiscountInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<DiscountInfo*>*
DiscountHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<DiscountInfo>(compressed);
}

std::string
DiscountHistoricalDAO::_name("DiscountHistorical");
std::string
DiscountHistoricalDAO::_cacheClass("Rules");
DAOHelper<DiscountHistoricalDAO>
DiscountHistoricalDAO::_helper(_name);
DiscountHistoricalDAO* DiscountHistoricalDAO::_instance = nullptr;

} // namespace tse
