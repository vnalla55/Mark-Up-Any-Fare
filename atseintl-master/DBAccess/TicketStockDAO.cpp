//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/TicketStockDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetTicketStock.h"
#include "DBAccess/TicketStock.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
TicketStockDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.TicketStockDAO"));

TicketStockDAO&
TicketStockDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<TicketStock*>&
getTicketStockData(int key,
                   const DateTime& date,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical)
{
  if (isHistorical)
  {
    TicketStockHistoricalDAO& dao = TicketStockHistoricalDAO::instance();
    return dao.get(deleteList, key, date, ticketDate);
  }
  else
  {
    TicketStockDAO& dao = TicketStockDAO::instance();
    return dao.get(deleteList, key, date, ticketDate);
  }
}

const std::vector<TicketStock*>&
TicketStockDAO::get(DeleteList& del, int key, const DateTime& date, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(IntKey(key));
  del.copy(ptr);
  std::vector<TicketStock*>* ret = new std::vector<TicketStock*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveG<TicketStock>(date, ticketDate));
  ret->erase(std::remove_if(ret->begin(), ret->end(), Inhibit<TicketStock>), ret->end());
  return *ret;
}

std::vector<TicketStock*>*
TicketStockDAO::create(IntKey key)
{
  std::vector<TicketStock*>* ret = new std::vector<TicketStock*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTicketStock ts(dbAdapter->getAdapter());
    ts.findTicketStock(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TicketStockDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TicketStockDAO::destroy(IntKey key, std::vector<TicketStock*>* recs)
{
  destroyContainer(recs);
}

void
TicketStockDAO::load()
{
  StartupLoader<QueryGetAllTicketStock, TicketStock, TicketStockDAO>();
}

IntKey
TicketStockDAO::createKey(TicketStock* info)
{
  return IntKey(info->tktStockCode());
}

std::string
TicketStockDAO::_name("TicketStock");
std::string
TicketStockDAO::_cacheClass("Common");

DAOHelper<TicketStockDAO>
TicketStockDAO::_helper(_name);

TicketStockDAO* TicketStockDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: TicketStockHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
TicketStockHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.TicketStockHistoricalDAO"));
TicketStockHistoricalDAO&
TicketStockHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<TicketStock*>&
TicketStockHistoricalDAO::get(DeleteList& del,
                              int key,
                              const DateTime& date,
                              const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<TicketStock*>* ret = new std::vector<TicketStock*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveHist<TicketStock>(date, ticketDate));
  ret->erase(std::remove_if(ret->begin(), ret->end(), Inhibit<TicketStock>), ret->end());
  return *ret;
}

std::vector<TicketStock*>*
TicketStockHistoricalDAO::create(int key)
{
  std::vector<TicketStock*>* ret = new std::vector<TicketStock*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTicketStockHistorical ts(dbAdapter->getAdapter());
    ts.findTicketStock(*ret, key);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TicketStockHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TicketStockHistoricalDAO::destroy(int key, std::vector<TicketStock*>* recs)
{
  destroyContainer(recs);
}

struct TicketStockHistoricalDAO::groupByKey
{
public:
  int prevKey;

  DAOCache& cache;

  groupByKey() : prevKey(-1), cache(TicketStockHistoricalDAO::instance().cache()), ptr(nullptr) {}

  std::vector<TicketStock*>* ptr;

  void operator()(TicketStock* info)
  {
    int key(info->tktStockCode());
    if (!(key == prevKey))
    {
      ptr = new std::vector<TicketStock*>;
      cache.put(key, ptr);
      prevKey = key;
    }

    ptr->push_back(info);
  }
};

void
TicketStockHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<TicketStock*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllTicketStockHistorical ts(dbAdapter->getAdapter());
    ts.findAllTicketStock(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TicketStockDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::for_each(recs.begin(), recs.end(), groupByKey());
}

std::string
TicketStockHistoricalDAO::_name("TicketStockHistorical");
std::string
TicketStockHistoricalDAO::_cacheClass("Common");
DAOHelper<TicketStockHistoricalDAO>
TicketStockHistoricalDAO::_helper(_name);

TicketStockHistoricalDAO* TicketStockHistoricalDAO::_instance = nullptr;

} // namespace tse
