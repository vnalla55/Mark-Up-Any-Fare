//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/CircleTripProvisionDAO.h"

#include "Common/Logger.h"
#include "DBAccess/CircleTripProvision.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetCircleTripProvision.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
CircleTripProvisionDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.CircleTripProvisionDAO"));

CircleTripProvisionDAO&
CircleTripProvisionDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const CircleTripProvision*
getCircleTripProvisionData(const LocCode& market1,
                           const LocCode& market2,
                           const DateTime& date,
                           DeleteList& deleteList,
                           const DateTime& ticketDate,
                           bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    CircleTripProvisionHistoricalDAO& dao = CircleTripProvisionHistoricalDAO::instance();
    return dao.get(deleteList, market1, market2, date, ticketDate);
  }
  else
  {
    CircleTripProvisionDAO& dao = CircleTripProvisionDAO::instance();
    return dao.get(deleteList, market1, market2, date, ticketDate);
  }
}

const CircleTripProvision*
CircleTripProvisionDAO::get(DeleteList& del,
                            const LocCode& market1,
                            const LocCode& market2,
                            const DateTime& date,
                            const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  const CircleTripProvisionKey& key = market1 < market2 ? CircleTripProvisionKey(market1, market2)
                                                        : CircleTripProvisionKey(market2, market1);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  CircleTripProvision* ret = nullptr;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveG<CircleTripProvision>(date, ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

void
CircleTripProvisionDAO::load()
{
  StartupLoader<QueryGetAllCircleTripProvision, CircleTripProvision, CircleTripProvisionDAO>();
}

CircleTripProvisionKey
CircleTripProvisionDAO::createKey(CircleTripProvision* info)
{
  return CircleTripProvisionKey(info->market1(), info->market2());
}

std::vector<CircleTripProvision*>*
CircleTripProvisionDAO::create(CircleTripProvisionKey key)
{
  std::vector<CircleTripProvision*>* ret = new std::vector<CircleTripProvision*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCircleTripProvision ctp(dbAdapter->getAdapter());
    ctp.findCircleTripProvision(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CircleTripProvisionDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

sfc::CompressedData*
CircleTripProvisionDAO::compress(const std::vector<CircleTripProvision*>* vect) const
{
  return compressVector(vect);
}

std::vector<CircleTripProvision*>*
CircleTripProvisionDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<CircleTripProvision>(compressed);
}

void
CircleTripProvisionDAO::destroy(CircleTripProvisionKey key, std::vector<CircleTripProvision*>* recs)
{
  std::vector<CircleTripProvision*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
CircleTripProvisionDAO::_name("CircleTripProvision");
std::string
CircleTripProvisionDAO::_cacheClass("Rules");

DAOHelper<CircleTripProvisionDAO>
CircleTripProvisionDAO::_helper(_name);

CircleTripProvisionDAO* CircleTripProvisionDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: CircleTripProvisionHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
CircleTripProvisionHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.CircleTripProvisionHistoricalDAO"));
CircleTripProvisionHistoricalDAO&
CircleTripProvisionHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const CircleTripProvision*
CircleTripProvisionHistoricalDAO::get(DeleteList& del,
                                      const LocCode& market1,
                                      const LocCode& market2,
                                      const DateTime& date,
                                      const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  const CircleTripProvisionKey& key = market1 < market2 ? CircleTripProvisionKey(market1, market2)
                                                        : CircleTripProvisionKey(market2, market1);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  CircleTripProvision* ret = nullptr;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveHist<CircleTripProvision>(date, ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<CircleTripProvision*>*
CircleTripProvisionHistoricalDAO::create(CircleTripProvisionKey key)
{
  std::vector<CircleTripProvision*>* ret = new std::vector<CircleTripProvision*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCircleTripProvisionHistorical ctp(dbAdapter->getAdapter());
    ctp.findCircleTripProvision(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CircleTripProvisionHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CircleTripProvisionHistoricalDAO::destroy(CircleTripProvisionKey key,
                                          std::vector<CircleTripProvision*>* recs)
{
  std::vector<CircleTripProvision*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

struct CircleTripProvisionHistoricalDAO::groupByKey
{
public:
  CircleTripProvisionKey prevKey;

  DAOCache& cache;

  groupByKey() : cache(CircleTripProvisionHistoricalDAO::instance().cache()), ptr(nullptr) {}

  std::vector<CircleTripProvision*>* ptr;

  void operator()(CircleTripProvision* info)
  {
    CircleTripProvisionKey key(info->market1(), info->market2());
    if (!(key == prevKey))
    {
      ptr = new std::vector<CircleTripProvision*>;
      cache.put(key, ptr);
      prevKey = key;
    }
    ptr->push_back(info);
  }
};
sfc::CompressedData*
CircleTripProvisionHistoricalDAO::compress(const std::vector<CircleTripProvision*>* vect) const
{
  return compressVector(vect);
}

std::vector<CircleTripProvision*>*
CircleTripProvisionHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<CircleTripProvision>(compressed);
}

void
CircleTripProvisionHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<CircleTripProvision*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllCircleTripProvisionHistorical ctp(dbAdapter->getAdapter());
    ctp.findAllCircleTripProvision(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CircleTripProvisionHistoricalDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::for_each(recs.begin(), recs.end(), groupByKey());
}

std::string
CircleTripProvisionHistoricalDAO::_name("CircleTripProvisionHistorical");
std::string
CircleTripProvisionHistoricalDAO::_cacheClass("Rules");
DAOHelper<CircleTripProvisionHistoricalDAO>
CircleTripProvisionHistoricalDAO::_helper(_name);
CircleTripProvisionHistoricalDAO* CircleTripProvisionHistoricalDAO::_instance = nullptr;

} // namespace tse
