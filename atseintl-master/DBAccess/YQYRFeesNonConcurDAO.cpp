//-------------------------------------------------------------------------------
// Copyright 2004, 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/YQYRFeesNonConcurDAO.h"

#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetYQYRFeesNonConcur.h"
#include "DBAccess/YQYRFeesNonConcur.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
YQYRFeesNonConcurDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.YQYRFeesNonConcurDAO"));

YQYRFeesNonConcurDAO&
YQYRFeesNonConcurDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<YQYRFeesNonConcur*>&
getYQYRFeesNonConcurData(const CarrierCode& carrier,
                         const DateTime& date,
                         DeleteList& deleteList,
                         const DateTime& ticketDate,
                         bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    YQYRFeesNonConcurHistoricalDAO& dao = YQYRFeesNonConcurHistoricalDAO::instance();
    return dao.get(deleteList, carrier, date, ticketDate);
  }
  else
  {
    YQYRFeesNonConcurDAO& dao = YQYRFeesNonConcurDAO::instance();
    return dao.get(deleteList, carrier, date, ticketDate);
  }
}

const std::vector<YQYRFeesNonConcur*>&
YQYRFeesNonConcurDAO::get(DeleteList& del,
                          const CarrierCode& key,
                          const DateTime& date,
                          const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(CarrierKey(key));
  /*
  del.copy(ptr);
  std::vector<YQYRFeesNonConcur*>* ret = new std::vector<YQYRFeesNonConcur*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret),
  IsNotEffectiveG<YQYRFeesNonConcur>(date, ticketDate));
  return *ret;
  */
  return *(applyFilter(del, ptr, IsNotEffectiveG<YQYRFeesNonConcur>(date, ticketDate)));
}

void
YQYRFeesNonConcurDAO::load()
{
  StartupLoader<QueryGetAllYQYRFeesNonConcur, YQYRFeesNonConcur, YQYRFeesNonConcurDAO>();
}

CarrierKey
YQYRFeesNonConcurDAO::createKey(YQYRFeesNonConcur* info)
{
  return CarrierKey(info->carrier());
}

std::vector<YQYRFeesNonConcur*>*
YQYRFeesNonConcurDAO::create(CarrierKey key)
{
  std::vector<YQYRFeesNonConcur*>* ret = new std::vector<YQYRFeesNonConcur*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetYQYRFeesNonConcur yq(dbAdapter->getAdapter());
    yq.findYQYRFeesNonConcur(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in YQYRFeesNonConcurDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
YQYRFeesNonConcurDAO::destroy(CarrierKey key, std::vector<YQYRFeesNonConcur*>* recs)
{
  destroyContainer(recs);
}

std::string
YQYRFeesNonConcurDAO::_name("YQYRFeesNonConcur");
std::string
YQYRFeesNonConcurDAO::_cacheClass("Taxes");

DAOHelper<YQYRFeesNonConcurDAO>
YQYRFeesNonConcurDAO::_helper(_name);

YQYRFeesNonConcurDAO* YQYRFeesNonConcurDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: YQYRFeesNonConcurHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
YQYRFeesNonConcurHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.YQYRFeesNonConcurHistoricalDAO"));
YQYRFeesNonConcurHistoricalDAO&
YQYRFeesNonConcurHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<YQYRFeesNonConcur*>&
YQYRFeesNonConcurHistoricalDAO::get(DeleteList& del,
                                    const CarrierCode& key,
                                    const DateTime& date,
                                    const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  YQYRFeesNonConcurHistoricalKey cacheKey(key);
  DAOUtils::getDateRange(ticketDate, cacheKey._b, cacheKey._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(cacheKey);
  del.copy(ptr);
  std::vector<YQYRFeesNonConcur*>* ret = new std::vector<YQYRFeesNonConcur*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveHist<YQYRFeesNonConcur>(date, ticketDate));
  return *ret;
}

std::vector<YQYRFeesNonConcur*>*
YQYRFeesNonConcurHistoricalDAO::create(YQYRFeesNonConcurHistoricalKey key)
{
  std::vector<YQYRFeesNonConcur*>* ret = new std::vector<YQYRFeesNonConcur*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetYQYRFeesNonConcurHistorical yq(dbAdapter->getAdapter());
    yq.findYQYRFeesNonConcur(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in YQYRFeesNonConcurHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
YQYRFeesNonConcurHistoricalDAO::destroy(YQYRFeesNonConcurHistoricalKey key,
                                        std::vector<YQYRFeesNonConcur*>* recs)
{
  std::vector<YQYRFeesNonConcur*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
YQYRFeesNonConcurHistoricalDAO::_name("YQYRFeesNonConcurHistorical");
std::string
YQYRFeesNonConcurHistoricalDAO::_cacheClass("Taxes");
DAOHelper<YQYRFeesNonConcurHistoricalDAO>
YQYRFeesNonConcurHistoricalDAO::_helper(_name);

YQYRFeesNonConcurHistoricalDAO* YQYRFeesNonConcurHistoricalDAO::_instance = nullptr;

} // namespace tse
