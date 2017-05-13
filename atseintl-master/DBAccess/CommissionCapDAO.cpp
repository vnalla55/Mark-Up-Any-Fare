//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/CommissionCapDAO.h"

#include "Common/Logger.h"
#include "DBAccess/CommissionCap.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetCommissionCap.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
CommissionCapDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.CommissionCapDAO"));

CommissionCapDAO&
CommissionCapDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<CommissionCap*>&
getCommissionCapData(const CarrierCode& carrier,
                     const CurrencyCode& cur,
                     const DateTime& date,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical)
{
  if (isHistorical)
  {
    CommissionCapHistoricalDAO& dao = CommissionCapHistoricalDAO::instance();
    return dao.get(deleteList, carrier, cur, date, ticketDate);
  }
  else
  {
    CommissionCapDAO& dao = CommissionCapDAO::instance();
    return dao.get(deleteList, carrier, cur, date, ticketDate);
  }
}

const std::vector<CommissionCap*>&
CommissionCapDAO::get(DeleteList& del,
                      const CarrierCode& carrier,
                      const CurrencyCode& cur,
                      const DateTime& date,
                      const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  CommissionCapKey key(carrier, cur);
  DAOCache::pointer_type ptr = cache().get(key);
  /*
  del.copy(ptr);
  std::vector<CommissionCap*>* ret = new std::vector<CommissionCap*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), IsNotEffectiveG<CommissionCap>(date,
  ticketDate));
  return *ret;
  */
  return *(applyFilter(del, ptr, IsNotEffectiveG<CommissionCap>(date, ticketDate)));
}

CommissionCapKey
CommissionCapDAO::createKey(CommissionCap* info)
{
  return CommissionCapKey(info->carrier(), info->cur());
}

std::vector<CommissionCap*>*
CommissionCapDAO::create(CommissionCapKey key)
{
  std::vector<CommissionCap*>* ret = new std::vector<CommissionCap*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCommissionCap cc(dbAdapter->getAdapter());
    cc.findCommissionCap(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CommissionCapDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CommissionCapDAO::destroy(CommissionCapKey key, std::vector<CommissionCap*>* recs)
{
  destroyContainer(recs);
}

void
CommissionCapDAO::load()
{
  StartupLoader<QueryGetAllCommissionCap, CommissionCap, CommissionCapDAO>();
}

std::string
CommissionCapDAO::_name("CommissionCap");
std::string
CommissionCapDAO::_cacheClass("Rules");
DAOHelper<CommissionCapDAO>
CommissionCapDAO::_helper(_name);
CommissionCapDAO* CommissionCapDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: CommissionCapHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
CommissionCapHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.CommissionCapHistoricalDAO"));
CommissionCapHistoricalDAO&
CommissionCapHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<CommissionCap*>&
CommissionCapHistoricalDAO::get(DeleteList& del,
                                const CarrierCode& carrier,
                                const CurrencyCode& cur,
                                const DateTime& date,
                                const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  CommissionCapKey key(carrier, cur);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<CommissionCap*>* ret = new std::vector<CommissionCap*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveHist<CommissionCap>(date, ticketDate));
  return *ret;
}

std::vector<CommissionCap*>*
CommissionCapHistoricalDAO::create(CommissionCapKey key)
{
  std::vector<CommissionCap*>* ret = new std::vector<CommissionCap*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCommissionCapHistorical cc(dbAdapter->getAdapter());
    cc.findCommissionCap(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CommissionCapHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CommissionCapHistoricalDAO::destroy(CommissionCapKey key, std::vector<CommissionCap*>* recs)
{
  destroyContainer(recs);
}

struct CommissionCapHistoricalDAO::groupByKey
{
public:
  CommissionCapKey prevKey;

  DAOCache& cache;

  groupByKey() : cache(CommissionCapHistoricalDAO::instance().cache()), ptr(nullptr) {}

  std::vector<CommissionCap*>* ptr;

  void operator()(CommissionCap* info)
  {
    CommissionCapKey key(info->carrier(), info->cur());
    if (!(key == prevKey))
    {
      ptr = new std::vector<CommissionCap*>;
      cache.put(key, ptr);
      prevKey = key;
    }

    ptr->push_back(info);
  }
};

void
CommissionCapHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<CommissionCap*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllCommissionCapHistorical cc(dbAdapter->getAdapter());
    cc.findAllCommissionCap(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CommissionCapHistoricalDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::for_each(recs.begin(), recs.end(), groupByKey());
}

std::string
CommissionCapHistoricalDAO::_name("CommissionCapHistorical");
std::string
CommissionCapHistoricalDAO::_cacheClass("Rules");
DAOHelper<CommissionCapHistoricalDAO>
CommissionCapHistoricalDAO::_helper(_name);
CommissionCapHistoricalDAO* CommissionCapHistoricalDAO::_instance = nullptr;

} // namespace tse
