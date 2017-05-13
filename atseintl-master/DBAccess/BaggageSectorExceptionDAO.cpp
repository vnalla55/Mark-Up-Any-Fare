//------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
#include "DBAccess/BaggageSectorExceptionDAO.h"

#include "Common/Logger.h"
#include "DBAccess/BaggageSectorException.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetBaggageSectorException.h"

namespace tse
{
log4cxx::LoggerPtr
BaggageSectorExceptionDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.BaggageSectorExceptionDAO"));

BaggageSectorExceptionDAO&
BaggageSectorExceptionDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<BaggageSectorException*>&
BaggageSectorExceptionDAO::get(DeleteList& del,
                               const CarrierCode& carrier,
                               const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  BSEKey key(carrier);
  DAOCache::pointer_type ptr = cache().get(key);

  return *applyFilter(del, ptr, IsNotEffectiveG<BaggageSectorException>(ticketDate));
}

BSEKey
BaggageSectorExceptionDAO::createKey(BaggageSectorException* info)
{
  return BSEKey(info->carrier());
}

std::vector<BaggageSectorException*>*
BaggageSectorExceptionDAO::create(BSEKey key)
{
  std::vector<BaggageSectorException*>* ret = new std::vector<BaggageSectorException*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetBaggageSectorException ds(dbAdapter->getAdapter());
    ds.findBaggageSectorException(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in BaggageSectorExceptionDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
BaggageSectorExceptionDAO::destroy(BSEKey key, std::vector<BaggageSectorException*>* recs)
{
  destroyContainer(recs);
}

void
BaggageSectorExceptionDAO::load()
{
  StartupLoader<QueryGetAllBaggageSectorException,
                BaggageSectorException,
                BaggageSectorExceptionDAO>();
}

std::string
BaggageSectorExceptionDAO::_name("BaggageSectorException");
std::string
BaggageSectorExceptionDAO::_cacheClass("Common");

DAOHelper<BaggageSectorExceptionDAO>
BaggageSectorExceptionDAO::_helper(_name);

BaggageSectorExceptionDAO* BaggageSectorExceptionDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: BaggageSectorExceptionHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
BaggageSectorExceptionHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.BaggageSectorExceptionHistoricalDAO"));
BaggageSectorExceptionHistoricalDAO&
BaggageSectorExceptionHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<BaggageSectorException*>&
BaggageSectorExceptionHistoricalDAO::get(DeleteList& del,
                                         const CarrierCode& carrier,
                                         const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  BSEKey key(carrier);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<BaggageSectorException*>* ret = new std::vector<BaggageSectorException*>;
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveH<BaggageSectorException>(ticketDate, ticketDate));
  del.adopt(ret);
  return *ret;
}

std::vector<BaggageSectorException*>*
BaggageSectorExceptionHistoricalDAO::create(BSEKey key)
{
  std::vector<BaggageSectorException*>* ret = new std::vector<BaggageSectorException*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetBaggageSectorExceptionHistorical ds(dbAdapter->getAdapter());
    ds.findBaggageSectorException(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in BaggageSectorExceptionHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
BaggageSectorExceptionHistoricalDAO::destroy(BSEKey key, std::vector<BaggageSectorException*>* recs)
{
  destroyContainer(recs);
}

struct BaggageSectorExceptionHistoricalDAO::groupByKey
{
public:
  BSEKey prevKey;
  DAOCache& cache;

  groupByKey() : cache(BaggageSectorExceptionHistoricalDAO::instance().cache()), ptr(nullptr) {}

  std::vector<BaggageSectorException*>* ptr;

  void operator()(BaggageSectorException* info)
  {
    BSEKey key(info->carrier());
    if (!(key == prevKey))
    {
      ptr = new std::vector<BaggageSectorException*>;
      cache.put(key, ptr);
      prevKey = key;
    }
    ptr->push_back(info);
  }
};

void
BaggageSectorExceptionHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<BaggageSectorException*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllBaggageSectorExceptionHistorical ds(dbAdapter->getAdapter());
    ds.findAllBaggageSectorException(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in BaggageSectorExceptionHistoricalDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }
  std::for_each(recs.begin(), recs.end(), groupByKey());
}

std::string
BaggageSectorExceptionHistoricalDAO::_name("BaggageSectorExceptionHistorical");
std::string
BaggageSectorExceptionHistoricalDAO::_cacheClass("Common");
DAOHelper<BaggageSectorExceptionHistoricalDAO>
BaggageSectorExceptionHistoricalDAO::_helper(_name);

BaggageSectorExceptionHistoricalDAO* BaggageSectorExceptionHistoricalDAO::_instance = nullptr;

} // namespace tse
