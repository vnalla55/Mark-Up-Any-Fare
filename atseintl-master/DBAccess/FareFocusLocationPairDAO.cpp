#include "DBAccess/FareFocusLocationPairDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareFocusLocationPairInfo.h"
#include "DBAccess/Queries/QueryGetFareFocusLocationPair.h"

namespace tse
{
log4cxx::LoggerPtr
FareFocusLocationPairDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareFocusLocationPairDAO"));

FareFocusLocationPairDAO&
FareFocusLocationPairDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const FareFocusLocationPairInfo*
getFareFocusLocationPairData(uint64_t locationPairItemNo,
                         DateTime adjustedTicketDate,
                         DeleteList& deleteList,
                         DateTime ticketDate,
                         bool isHistorical)
{
  if (isHistorical)
  {
    FareFocusLocationPairHistoricalDAO& dao(FareFocusLocationPairHistoricalDAO::instance());

    return dao.get(deleteList, locationPairItemNo, ticketDate);
  }
  else
  {
    FareFocusLocationPairDAO& dao(FareFocusLocationPairDAO::instance());

    return dao.get(deleteList, locationPairItemNo, adjustedTicketDate, ticketDate);
  }
}

const FareFocusLocationPairInfo*
FareFocusLocationPairDAO::get(DeleteList& del,
                           uint64_t locationPairItemNo,
                           DateTime adjustedTicketDate,
                           DateTime ticketDate)
{
  ++_codeCoverageGetCallCount;

  FareFocusLocationPairKey key(locationPairItemNo);
  DAOCache::pointer_type ptr(cache().get(key));
  FareFocusLocationPairInfo* ret(nullptr);
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsCurrentG<FareFocusLocationPairInfo>(ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<FareFocusLocationPairInfo*>*
FareFocusLocationPairDAO::create(FareFocusLocationPairKey key)
{
  std::vector<FareFocusLocationPairInfo*>* ret(new std::vector<FareFocusLocationPairInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get(this->cacheClass()));
  try
  {
    QueryGetFareFocusLocationPair q(dbAdapter->getAdapter());
    q.findFareFocusLocationPair(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareFocusLocationPairDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareFocusLocationPairKey
FareFocusLocationPairDAO::createKey(const FareFocusLocationPairInfo* info)
{
  return FareFocusLocationPairKey(info->locationPairItemNo());
}

void
FareFocusLocationPairDAO::destroy(FareFocusLocationPairKey, std::vector<FareFocusLocationPairInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FareFocusLocationPairDAO::_name("FareFocusLocationPair");
std::string
FareFocusLocationPairDAO::_cacheClass("Rules");
DAOHelper<FareFocusLocationPairDAO>
FareFocusLocationPairDAO::_helper(_name);
FareFocusLocationPairDAO*
FareFocusLocationPairDAO::_instance(nullptr);

// Historical DAO Object
log4cxx::LoggerPtr
FareFocusLocationPairHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareFocusLocationPairHistoricalDAO"));

FareFocusLocationPairHistoricalDAO&
FareFocusLocationPairHistoricalDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const FareFocusLocationPairInfo*
FareFocusLocationPairHistoricalDAO::get(DeleteList& del,
                                     uint64_t locationPairItemNo,
                                     DateTime ticketDate)
{
  ++_codeCoverageGetCallCount;

  FareFocusLocationPairHistoricalKey key(locationPairItemNo);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr(cache().get(key));
  FareFocusLocationPairInfo* ret(nullptr);
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsCurrentDH<FareFocusLocationPairInfo>(ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<FareFocusLocationPairInfo*>*
FareFocusLocationPairHistoricalDAO::create(FareFocusLocationPairHistoricalKey key)
{
  std::vector<FareFocusLocationPairInfo*>* ret(new std::vector<FareFocusLocationPairInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get("Historical", true));
  try
  {
    QueryGetFareFocusLocationPairHistorical q(dbAdapter->getAdapter());
    q.findFareFocusLocationPair(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareFocusLocationPairHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareFocusLocationPairHistoricalKey
FareFocusLocationPairHistoricalDAO::createKey(const FareFocusLocationPairInfo* info,
                                           const DateTime& startDate,
                                           const DateTime& endDate)
{
  return FareFocusLocationPairHistoricalKey(info->locationPairItemNo(), startDate, endDate);
}

void
FareFocusLocationPairHistoricalDAO::destroy(FareFocusLocationPairHistoricalKey,
                                         std::vector<FareFocusLocationPairInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FareFocusLocationPairHistoricalDAO::_name("FareFocusLocationPairHistorical");
std::string
FareFocusLocationPairHistoricalDAO::_cacheClass("Rules");
DAOHelper<FareFocusLocationPairHistoricalDAO>
FareFocusLocationPairHistoricalDAO::_helper(_name);
FareFocusLocationPairHistoricalDAO*
FareFocusLocationPairHistoricalDAO::_instance(nullptr);

} // namespace tse
