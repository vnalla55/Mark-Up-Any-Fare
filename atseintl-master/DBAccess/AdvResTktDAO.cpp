//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/AdvResTktDAO.h"

#include "Common/Logger.h"
#include "DBAccess/AdvResTktInfo.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetAdvResTkt.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
AdvResTktDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.AdvResTktDAO"));

AdvResTktDAO&
AdvResTktDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const AdvResTktInfo*
getAdvResTktData(const VendorCode& vendor,
                 int itemNo,
                 DeleteList& deleteList,
                 const DateTime& ticketDate,
                 bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    AdvResTktHistoricalDAO& dao = AdvResTktHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    AdvResTktDAO& dao = AdvResTktDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const AdvResTktInfo*
AdvResTktDAO::get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  AdvResTktKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (UNLIKELY(ptr->empty()))
    return nullptr;

  del.copy(ptr);

  IsCurrentG<AdvResTktInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (LIKELY(isCurrent(*iter) && NotInhibit<AdvResTktInfo>(*iter)))
      return *iter;
  }
  return nullptr;
}

std::vector<AdvResTktInfo*>*
AdvResTktDAO::create(AdvResTktKey key)
{
  std::vector<AdvResTktInfo*>* ret = new std::vector<AdvResTktInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAdvResTkt art(dbAdapter->getAdapter());
    art.findAdvResTktInfo(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in AdvResTktDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
AdvResTktDAO::destroy(AdvResTktKey key, std::vector<AdvResTktInfo*>* recs)
{
  std::vector<AdvResTktInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

AdvResTktKey
AdvResTktDAO::createKey(AdvResTktInfo* info)
{
  return AdvResTktKey(info->vendor(), info->itemNo());
}

void
AdvResTktDAO::load()
{
  StartupLoaderNoDB<AdvResTktInfo, AdvResTktDAO>();
}

sfc::CompressedData*
AdvResTktDAO::compress(const std::vector<AdvResTktInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<AdvResTktInfo*>*
AdvResTktDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<AdvResTktInfo>(compressed);
}

std::string
AdvResTktDAO::_name("AdvResTkt");
std::string
AdvResTktDAO::_cacheClass("Rules");
DAOHelper<AdvResTktDAO>
AdvResTktDAO::_helper(_name);
AdvResTktDAO* AdvResTktDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: AdvResTktHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
AdvResTktHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.AdvResTktHistoricalDAO"));
AdvResTktHistoricalDAO&
AdvResTktHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const AdvResTktInfo*
AdvResTktHistoricalDAO::get(DeleteList& del,
                            const VendorCode& vendor,
                            int itemNo,
                            const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  AdvResTktHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<AdvResTktInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<AdvResTktInfo>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<AdvResTktInfo*>*
AdvResTktHistoricalDAO::create(AdvResTktHistoricalKey key)
{
  std::vector<AdvResTktInfo*>* ret = new std::vector<AdvResTktInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAdvResTktHistorical art(dbAdapter->getAdapter());
    art.findAdvResTktInfo(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in AdvResTktHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
AdvResTktHistoricalDAO::destroy(AdvResTktHistoricalKey key, std::vector<AdvResTktInfo*>* recs)
{
  std::vector<AdvResTktInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
AdvResTktHistoricalDAO::compress(const std::vector<AdvResTktInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<AdvResTktInfo*>*
AdvResTktHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<AdvResTktInfo>(compressed);
}

std::string
AdvResTktHistoricalDAO::_name("AdvResTktHistorical");
std::string
AdvResTktHistoricalDAO::_cacheClass("Rules");
DAOHelper<AdvResTktHistoricalDAO>
AdvResTktHistoricalDAO::_helper(_name);

AdvResTktHistoricalDAO* AdvResTktHistoricalDAO::_instance = nullptr;

} // namespace tse
