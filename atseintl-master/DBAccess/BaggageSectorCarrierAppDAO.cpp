//------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
#include "DBAccess/BaggageSectorCarrierAppDAO.h"

#include "Common/Logger.h"
#include "DBAccess/BaggageSectorCarrierApp.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetBaggageSectorCarrierApp.h"

namespace tse
{
log4cxx::LoggerPtr
BaggageSectorCarrierAppDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.BaggageSectorCarrierAppDAO"));

BaggageSectorCarrierAppDAO&
BaggageSectorCarrierAppDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<BaggageSectorCarrierApp*>&
BaggageSectorCarrierAppDAO::get(DeleteList& del,
                                const CarrierCode& carrier,
                                const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  Key key(carrier);
  DAOCache::pointer_type ptr = cache().get(key);

  return *applyFilter(del, ptr, IsNotEffectiveG<BaggageSectorCarrierApp>(ticketDate));
}

Key
BaggageSectorCarrierAppDAO::createKey(BaggageSectorCarrierApp* crxApp)
{
  return Key(crxApp->marketingCarrier());
}

std::vector<BaggageSectorCarrierApp*>*
BaggageSectorCarrierAppDAO::create(Key key)
{
  std::vector<BaggageSectorCarrierApp*>* ret = new std::vector<BaggageSectorCarrierApp*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetBaggageSectorCarrierApp ds(dbAdapter->getAdapter());
    ds.findBaggageSectorCarrierApp(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in BaggageSectorCarrierAppDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
BaggageSectorCarrierAppDAO::destroy(Key key, std::vector<BaggageSectorCarrierApp*>* recs)
{
  destroyContainer(recs);
}

void
BaggageSectorCarrierAppDAO::load()
{
  StartupLoader<QueryGetAllBaggageSectorCarrierApp,
                BaggageSectorCarrierApp,
                BaggageSectorCarrierAppDAO>();
}

std::string
BaggageSectorCarrierAppDAO::_name("BaggageSectorCarrierApp");
std::string
BaggageSectorCarrierAppDAO::_cacheClass("Common");

DAOHelper<BaggageSectorCarrierAppDAO>
BaggageSectorCarrierAppDAO::_helper(_name);

BaggageSectorCarrierAppDAO* BaggageSectorCarrierAppDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: BaggageSectorCarrierAppHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
BaggageSectorCarrierAppHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.BaggageSectorCarrierAppHistoricalDAO"));
BaggageSectorCarrierAppHistoricalDAO&
BaggageSectorCarrierAppHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<BaggageSectorCarrierApp*>&
BaggageSectorCarrierAppHistoricalDAO::get(DeleteList& del,
                                          const CarrierCode& carrier,
                                          const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  Key key(carrier);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<BaggageSectorCarrierApp*>* ret = new std::vector<BaggageSectorCarrierApp*>;
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveH<BaggageSectorCarrierApp>(ticketDate, ticketDate));
  del.adopt(ret);
  return *ret;
}

std::vector<BaggageSectorCarrierApp*>*
BaggageSectorCarrierAppHistoricalDAO::create(Key key)
{
  std::vector<BaggageSectorCarrierApp*>* ret = new std::vector<BaggageSectorCarrierApp*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetBaggageSectorCarrierAppHistorical ds(dbAdapter->getAdapter());
    ds.findBaggageSectorCarrierApp(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in BaggageSectorCarrierAppHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
BaggageSectorCarrierAppHistoricalDAO::destroy(Key key, std::vector<BaggageSectorCarrierApp*>* recs)
{
  destroyContainer(recs);
}

struct BaggageSectorCarrierAppHistoricalDAO::groupByKey
{
public:
  Key prevKey;
  DAOCache& cache;

  groupByKey() : cache(BaggageSectorCarrierAppHistoricalDAO::instance().cache()) {}

  std::vector<BaggageSectorCarrierApp*>* ptr = nullptr;

  void operator()(BaggageSectorCarrierApp* crxApp)
  {
    Key key(crxApp->marketingCarrier());
    if (!(key == prevKey))
    {
      ptr = new std::vector<BaggageSectorCarrierApp*>;
      cache.put(key, ptr);
      prevKey = key;
    }
    ptr->push_back(crxApp);
  }
};

void
BaggageSectorCarrierAppHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<BaggageSectorCarrierApp*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllBaggageSectorCarrierAppHistorical ds(dbAdapter->getAdapter());
    ds.findAllBaggageSectorCarrierApp(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in BaggageSectorCarrierAppHistoricalDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }
  std::for_each(recs.begin(), recs.end(), groupByKey());
}

std::string
BaggageSectorCarrierAppHistoricalDAO::_name("BaggageSectorCarrierAppHistorical");
std::string
BaggageSectorCarrierAppHistoricalDAO::_cacheClass("Common");
DAOHelper<BaggageSectorCarrierAppHistoricalDAO>
BaggageSectorCarrierAppHistoricalDAO::_helper(_name);

BaggageSectorCarrierAppHistoricalDAO* BaggageSectorCarrierAppHistoricalDAO::_instance = nullptr;

} // namespace tse
