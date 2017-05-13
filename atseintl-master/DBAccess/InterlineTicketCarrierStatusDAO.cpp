//----------------------------------------------------------------------------
//          File:           InterlineTicketCarrierStatusDAO.cpp
//          Description:    InterlineTicketCarrierStatusDAO
//          Created:        02/3/2012
//          Authors:        M Dantas
//
//          Updates:
//
//     (c)2010, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#include "DBAccess/InterlineTicketCarrierStatusDAO.h"

#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/InterlineTicketCarrierStatus.h"
#include "DBAccess/Queries/QueryGetInterlineTicketCarrierStatus.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
InterlineTicketCarrierStatusDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.InterlineTicketCarrierStatusDAO"));

InterlineTicketCarrierStatusDAO&
InterlineTicketCarrierStatusDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}
const std::vector<InterlineTicketCarrierStatus*>&
getInterlineTicketCarrierStatusData(const CarrierCode& carrier,
                                    const CrsCode& crsCode,
                                    const DateTime& date,
                                    DeleteList& deleteList,
                                    const DateTime& ticketDate,
                                    bool isHistorical)
{
  if (isHistorical)
  {
    InterlineTicketCarrierStatusHistoricalDAO& dao =
        InterlineTicketCarrierStatusHistoricalDAO::instance();
    return dao.get(deleteList, carrier, crsCode, date, ticketDate);
  }
  else
  {
    InterlineTicketCarrierStatusDAO& dao = InterlineTicketCarrierStatusDAO::instance();
    return dao.get(deleteList, carrier, crsCode, date, ticketDate);
  }
}

const std::vector<InterlineTicketCarrierStatus*>&
InterlineTicketCarrierStatusDAO::get(DeleteList& del,
                                     const CarrierCode& carrier,
                                     const CrsCode& crsCode,
                                     const DateTime& date,
                                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;
  IETKey key(carrier, crsCode);
  DAOCache::pointer_type ptr = cache().get(key);
  return *(applyFilter(
      del, ptr, IsNotEffective<InterlineTicketCarrierStatus>(date, ticketDate, false)));
}

void
InterlineTicketCarrierStatusDAO::load()
{
  StartupLoader<QueryGetAllInterlineTicketCarrierStatus,
                InterlineTicketCarrierStatus,
                InterlineTicketCarrierStatusDAO>();
}

IETKey
InterlineTicketCarrierStatusDAO::createKey(InterlineTicketCarrierStatus* info)
{
  return IETKey(info->carrier(), info->crsCode());
}

std::vector<InterlineTicketCarrierStatus*>*
InterlineTicketCarrierStatusDAO::create(IETKey key)
{
  std::vector<InterlineTicketCarrierStatus*>* ret = new std::vector<InterlineTicketCarrierStatus*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetInterlineTicketCarrierStatus itcs(dbAdapter->getAdapter());
    itcs.findInterlineTicketCarrierStatus(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in InterlineTicketCarrierStatusDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
InterlineTicketCarrierStatusDAO::destroy(IETKey key,
                                         std::vector<InterlineTicketCarrierStatus*>* recs)
{
  destroyContainer(recs);
}

std::string
InterlineTicketCarrierStatusDAO::_name("InterlineTicketCarrierStatus");
std::string
InterlineTicketCarrierStatusDAO::_cacheClass("Common");
DAOHelper<InterlineTicketCarrierStatusDAO>
InterlineTicketCarrierStatusDAO::_helper(_name);
InterlineTicketCarrierStatusDAO* InterlineTicketCarrierStatusDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: InterlineTicketCarrierStatusHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
InterlineTicketCarrierStatusHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.InterlineTicketCarrierStatusHistoricalDAO"));

InterlineTicketCarrierStatusHistoricalDAO&
InterlineTicketCarrierStatusHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<InterlineTicketCarrierStatus*>&
InterlineTicketCarrierStatusHistoricalDAO::get(DeleteList& del,
                                               const CarrierCode& carrier,
                                               const CrsCode& crsCode,
                                               const DateTime& date,
                                               const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  IETKey key(carrier, crsCode);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<InterlineTicketCarrierStatus*>* ret = new std::vector<InterlineTicketCarrierStatus*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveHist<InterlineTicketCarrierStatus>(date, ticketDate));
  return *ret;
}

struct InterlineTicketCarrierStatusHistoricalDAO::groupByKey
{
public:
  IETKey prevKey;
  DAOCache& cache;

  groupByKey() : cache(InterlineTicketCarrierStatusHistoricalDAO::instance().cache()), ptr(nullptr) {}

  std::vector<InterlineTicketCarrierStatus*>* ptr;

  void operator()(InterlineTicketCarrierStatus* info)
  {
    IETKey key(info->carrier(), info->crsCode());
    if (!(key == prevKey))
    {
      ptr = new std::vector<InterlineTicketCarrierStatus*>;
      cache.put(key, ptr);
      prevKey = key;
    }
    ptr->push_back(info);
  }
};

void
InterlineTicketCarrierStatusHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<InterlineTicketCarrierStatus*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllInterlineTicketCarrierStatusHistorical itcs(dbAdapter->getAdapter());
    itcs.findAllInterlineTicketCarrierStatus(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in InterlineTicketCarrierStatusHistoricalDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::for_each(recs.begin(), recs.end(), groupByKey());
}

std::vector<InterlineTicketCarrierStatus*>*
InterlineTicketCarrierStatusHistoricalDAO::create(IETKey key)
{
  std::vector<InterlineTicketCarrierStatus*>* ret = new std::vector<InterlineTicketCarrierStatus*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetInterlineTicketCarrierStatusHistorical itcs(dbAdapter->getAdapter());
    itcs.findInterlineTicketCarrierStatus(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in InterlineTicketCarrierStatusHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
InterlineTicketCarrierStatusHistoricalDAO::destroy(IETKey key,
                                                   std::vector<InterlineTicketCarrierStatus*>* recs)
{
  destroyContainer(recs);
}

std::string
InterlineTicketCarrierStatusHistoricalDAO::_name("InterlineTicketCarrierStatusHistorical");
std::string
InterlineTicketCarrierStatusHistoricalDAO::_cacheClass("Common");
DAOHelper<InterlineTicketCarrierStatusHistoricalDAO>
InterlineTicketCarrierStatusHistoricalDAO::_helper(_name);
InterlineTicketCarrierStatusHistoricalDAO* InterlineTicketCarrierStatusHistoricalDAO::_instance = nullptr;

} // namespace tse
