#include "DBAccess/FareFocusRoutingDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareFocusRoutingInfo.h"
#include "DBAccess/Queries/QueryGetFareFocusRouting.h"

namespace tse
{
log4cxx::LoggerPtr
FareFocusRoutingDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareFocusRoutingDAO"));

FareFocusRoutingDAO&
FareFocusRoutingDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const FareFocusRoutingInfo*
getFareFocusRoutingData(uint64_t routingItemNo,
                         DateTime adjustedTicketDate,
                         DeleteList& deleteList,
                         DateTime ticketDate,
                         bool isHistorical)
{
  if (isHistorical)
  {
    FareFocusRoutingHistoricalDAO& dao(FareFocusRoutingHistoricalDAO::instance());

    return dao.get(deleteList, routingItemNo, ticketDate);
  }
  else
  {
    FareFocusRoutingDAO& dao(FareFocusRoutingDAO::instance());

    return dao.get(deleteList, routingItemNo, adjustedTicketDate, ticketDate);
  }
}

const FareFocusRoutingInfo*
FareFocusRoutingDAO::get(DeleteList& del,
                           uint64_t routingItemNo,
                           DateTime adjustedTicketDate,
                           DateTime ticketDate)
{
  ++_codeCoverageGetCallCount;

  FareFocusRoutingKey key(routingItemNo);
  DAOCache::pointer_type ptr(cache().get(key));
  FareFocusRoutingInfo* ret(nullptr);
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsCurrentG<FareFocusRoutingInfo>(ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<FareFocusRoutingInfo*>*
FareFocusRoutingDAO::create(FareFocusRoutingKey key)
{
  std::vector<FareFocusRoutingInfo*>* ret(new std::vector<FareFocusRoutingInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get(this->cacheClass()));
  try
  {
    QueryGetFareFocusRouting q(dbAdapter->getAdapter());
    q.findFareFocusRouting(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareFocusRoutingDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareFocusRoutingKey
FareFocusRoutingDAO::createKey(const FareFocusRoutingInfo* info)
{
  return FareFocusRoutingKey(info->routingItemNo());
}

void
FareFocusRoutingDAO::destroy(FareFocusRoutingKey, std::vector<FareFocusRoutingInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FareFocusRoutingDAO::_name("FareFocusRouting");
std::string
FareFocusRoutingDAO::_cacheClass("Rules");
DAOHelper<FareFocusRoutingDAO>
FareFocusRoutingDAO::_helper(_name);
FareFocusRoutingDAO*
FareFocusRoutingDAO::_instance(nullptr);

// Historical DAO Object
log4cxx::LoggerPtr
FareFocusRoutingHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareFocusRoutingHistoricalDAO"));

FareFocusRoutingHistoricalDAO&
FareFocusRoutingHistoricalDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const FareFocusRoutingInfo*
FareFocusRoutingHistoricalDAO::get(DeleteList& del,
                                     uint64_t routingItemNo,
                                     DateTime ticketDate)
{
  ++_codeCoverageGetCallCount;

  FareFocusRoutingHistoricalKey key(routingItemNo);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr(cache().get(key));
  FareFocusRoutingInfo* ret(nullptr);
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsCurrentDH<FareFocusRoutingInfo>(ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<FareFocusRoutingInfo*>*
FareFocusRoutingHistoricalDAO::create(FareFocusRoutingHistoricalKey key)
{
  std::vector<FareFocusRoutingInfo*>* ret(new std::vector<FareFocusRoutingInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get("Historical", true));
  try
  {
    QueryGetFareFocusRoutingHistorical q(dbAdapter->getAdapter());
    q.findFareFocusRouting(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareFocusRoutingHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareFocusRoutingHistoricalKey
FareFocusRoutingHistoricalDAO::createKey(const FareFocusRoutingInfo* info,
                                           const DateTime& startDate,
                                           const DateTime& endDate)
{
  return FareFocusRoutingHistoricalKey(info->routingItemNo(), startDate, endDate);
}

void
FareFocusRoutingHistoricalDAO::destroy(FareFocusRoutingHistoricalKey,
                                         std::vector<FareFocusRoutingInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FareFocusRoutingHistoricalDAO::_name("FareFocusRoutingHistorical");
std::string
FareFocusRoutingHistoricalDAO::_cacheClass("Rules");
DAOHelper<FareFocusRoutingHistoricalDAO>
FareFocusRoutingHistoricalDAO::_helper(_name);
FareFocusRoutingHistoricalDAO*
FareFocusRoutingHistoricalDAO::_instance(nullptr);

} // namespace tse
