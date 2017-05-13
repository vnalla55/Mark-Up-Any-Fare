//-------------------------------------------------------------------------------
// Copyright 2004, 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/YQYRFeesDAO.h"

#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DBServer.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetYQYRFees.h"
#include "DBAccess/YQYRFees.h"

#include <algorithm>
#include <functional>

namespace tse
{

log4cxx::LoggerPtr
YQYRFeesDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.YQYRFeesDAO"));

YQYRFeesDAO&
YQYRFeesDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<YQYRFees*>&
getYQYRFeesData(const CarrierCode& key,
                DeleteList& deleteList,
                const DateTime& ticketDate,
                bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    YQYRFeesHistoricalDAO& dao = YQYRFeesHistoricalDAO::instance();
    return dao.get(deleteList, key, ticketDate);
  }
  else
  {
    YQYRFeesDAO& dao = YQYRFeesDAO::instance();
    return dao.get(deleteList, key);
  }
}

const std::vector<YQYRFees*>&
YQYRFeesDAO::get(DeleteList& del, const CarrierCode& key)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(CarrierKey(key));
  del.copy(ptr);
  return *ptr;
}

void
YQYRFeesDAO::load()
{
  StartupLoader<QueryGetAllYQYRFees, YQYRFees, YQYRFeesDAO>();
}

CarrierKey
YQYRFeesDAO::createKey(YQYRFees* info)
{
  return CarrierKey(info->carrier());
}

std::vector<YQYRFees*>*
YQYRFeesDAO::create(CarrierKey key)
{
  std::vector<YQYRFees*>* ret = new std::vector<YQYRFees*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetYQYRFees yqf(dbAdapter->getAdapter());
    yqf.findYQYRFees(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in YQYRFeesDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
YQYRFeesDAO::destroy(CarrierKey key, std::vector<YQYRFees*>* recs)
{
  destroyContainer(recs);
}

sfc::CompressedData*
YQYRFeesDAO::compress(const std::vector<YQYRFees*>* vect) const
{
  return compressVector(vect);
}

std::vector<YQYRFees*>*
YQYRFeesDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<YQYRFees>(compressed);
}

std::string
YQYRFeesDAO::_name("YQYRFees");
std::string
YQYRFeesDAO::_cacheClass("Taxes");

DAOHelper<YQYRFeesDAO>
YQYRFeesDAO::_helper(_name);

YQYRFeesDAO* YQYRFeesDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: YQYRFeesHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
YQYRFeesHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.YQYRFeesHistoricalDAO"));
YQYRFeesHistoricalDAO&
YQYRFeesHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<YQYRFees*>&
YQYRFeesHistoricalDAO::get(DeleteList& del, const CarrierCode& key, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  YQYRFeesHistoricalKey cacheKey(key);
  DAOUtils::getDateRange(ticketDate, cacheKey._b, cacheKey._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(cacheKey);
  del.copy(ptr);
  std::vector<YQYRFees*>* ret = new std::vector<YQYRFees*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), IsNotEffectiveYQHist(ticketDate));

  return *ret;
}

std::vector<YQYRFees*>*
YQYRFeesHistoricalDAO::create(YQYRFeesHistoricalKey key)
{
  std::vector<YQYRFees*>* ret = new std::vector<YQYRFees*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetYQYRFeesHistorical yqf(dbAdapter->getAdapter());
    yqf.findYQYRFees(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in YQYRFeesHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}
sfc::CompressedData*
YQYRFeesHistoricalDAO::compress(const std::vector<YQYRFees*>* vect) const
{
  return compressVector(vect);
}

std::vector<YQYRFees*>*
YQYRFeesHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<YQYRFees>(compressed);
}

void
YQYRFeesHistoricalDAO::destroy(YQYRFeesHistoricalKey key, std::vector<YQYRFees*>* recs)
{
  std::vector<YQYRFees*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
YQYRFeesHistoricalDAO::_name("YQYRFeesHistorical");
std::string
YQYRFeesHistoricalDAO::_cacheClass("Taxes");
DAOHelper<YQYRFeesHistoricalDAO>
YQYRFeesHistoricalDAO::_helper(_name);

YQYRFeesHistoricalDAO* YQYRFeesHistoricalDAO::_instance = nullptr;

} // namespace tse
