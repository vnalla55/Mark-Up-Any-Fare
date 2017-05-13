//-------------------------------------------------------------------------------
// Copyright 2004, 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/RoutingForMarketDAO.h"

#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetRoutingForMarket.h"
#include "DBAccess/RoutingKeyInfo.h"

namespace tse
{
log4cxx::LoggerPtr
RoutingForMarketDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.RoutingForMarketDAO"));

RoutingForMarketDAO&
RoutingForMarketDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<RoutingKeyInfo*>&
getRoutingForMarketData(const LocCode& market1,
                        const LocCode& market2,
                        const CarrierCode& carrier,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical)
{
  if (isHistorical)
  {
    RoutingForMarketHistoricalDAO& dao = RoutingForMarketHistoricalDAO::instance();
    return dao.get(deleteList, market1, market2, carrier, ticketDate);
  }
  else
  {
    RoutingForMarketDAO& dao = RoutingForMarketDAO::instance();
    return dao.get(deleteList, market1, market2, carrier, ticketDate);
  }
}

const std::vector<RoutingKeyInfo*>&
RoutingForMarketDAO::get(DeleteList& del,
                         const LocCode& market1,
                         const LocCode& market2,
                         const CarrierCode& carrier,
                         const DateTime& ticketDate)
{
  RoutingForMarketKey key(market1, market2, carrier);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  return *ptr;
}

std::vector<RoutingKeyInfo*>*
RoutingForMarketDAO::create(RoutingForMarketKey key)
{
  std::vector<RoutingKeyInfo*>* ret = new std::vector<RoutingKeyInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetRoutingForDomMarket rfdm(dbAdapter->getAdapter());
    rfdm.findRoutingForMarket(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in RoutingForMarketDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
RoutingForMarketDAO::destroy(RoutingForMarketKey key, std::vector<RoutingKeyInfo*>* recs)
{
  std::vector<RoutingKeyInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
RoutingForMarketDAO::_name("RoutingForMarket");
std::string
RoutingForMarketDAO::_cacheClass("Routing");
DAOHelper<RoutingForMarketDAO>
RoutingForMarketDAO::_helper(_name);
RoutingForMarketDAO* RoutingForMarketDAO::_instance = nullptr;

// Historical Stuff
log4cxx::LoggerPtr
RoutingForMarketHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.RoutingForMarketHistoricalDAO"));

RoutingForMarketHistoricalDAO&
RoutingForMarketHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<RoutingKeyInfo*>&
RoutingForMarketHistoricalDAO::get(DeleteList& del,
                                   const LocCode& market1,
                                   const LocCode& market2,
                                   const CarrierCode& carrier,
                                   const DateTime& ticketDate)
{
  RoutingForMarketHistoricalKey key(market1, market2, carrier);
  DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<RoutingKeyInfo*>* ret = new std::vector<RoutingKeyInfo*>;
  del.adopt(ret);
  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotCurrentH<RoutingKeyInfo>(ticketDate));
  return *ret;
}

std::vector<RoutingKeyInfo*>*
RoutingForMarketHistoricalDAO::create(RoutingForMarketHistoricalKey key)
{
  std::vector<RoutingKeyInfo*>* ret = new std::vector<RoutingKeyInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetRoutingForDomMarketHistorical rfdm(dbAdapter->getAdapter());
    rfdm.findRoutingForMarket(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in RoutingForMarketHistDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
RoutingForMarketHistoricalDAO::destroy(RoutingForMarketHistoricalKey key,
                                       std::vector<RoutingKeyInfo*>* recs)
{
  std::vector<RoutingKeyInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
RoutingForMarketHistoricalDAO::_name("RoutingForMarketHistorical");
std::string
RoutingForMarketHistoricalDAO::_cacheClass("Routing");
DAOHelper<RoutingForMarketHistoricalDAO>
RoutingForMarketHistoricalDAO::_helper(_name);
RoutingForMarketHistoricalDAO* RoutingForMarketHistoricalDAO::_instance = nullptr;

} // namespace tse
