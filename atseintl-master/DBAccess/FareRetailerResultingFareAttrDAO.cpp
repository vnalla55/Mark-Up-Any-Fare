#include "DBAccess/FareRetailerResultingFareAttrDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareRetailerResultingFareAttrInfo.h"
#include "DBAccess/Queries/QueryGetFareRetailerResultingFareAttr.h"

namespace tse
{
log4cxx::LoggerPtr
FareRetailerResultingFareAttrDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareRetailerResultingFareAttrDAO"));

FareRetailerResultingFareAttrDAO&
FareRetailerResultingFareAttrDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const FareRetailerResultingFareAttrInfo*
getFareRetailerResultingFareAttrData(uint64_t resultingFareAttrItemNo,
                                     DateTime adjustedTicketDate,
                                     DeleteList& deleteList,
                                     DateTime ticketDate,
                                     bool isHistorical)
{
  if (isHistorical)
  {
    FareRetailerResultingFareAttrHistoricalDAO& dao(FareRetailerResultingFareAttrHistoricalDAO::instance());

    return dao.get(deleteList, resultingFareAttrItemNo, ticketDate);
  }
  else
  {
    FareRetailerResultingFareAttrDAO& dao(FareRetailerResultingFareAttrDAO::instance());

    return dao.get(deleteList, resultingFareAttrItemNo, adjustedTicketDate, ticketDate);
  }
}

const FareRetailerResultingFareAttrInfo*
FareRetailerResultingFareAttrDAO::get(DeleteList& del,
                                      uint64_t resultingFareAttrItemNo,
                                      DateTime adjustedTicketDate,
                                      DateTime ticketDate)
{
  ++_codeCoverageGetCallCount;

  FareRetailerResultingFareAttrKey key(resultingFareAttrItemNo);
  DAOCache::pointer_type ptr(cache().get(key));
  FareRetailerResultingFareAttrInfo* ret(nullptr);
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsCurrentG<FareRetailerResultingFareAttrInfo>(ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<FareRetailerResultingFareAttrInfo*>*
FareRetailerResultingFareAttrDAO::create(FareRetailerResultingFareAttrKey key)
{
  std::vector<FareRetailerResultingFareAttrInfo*>* ret(new std::vector<FareRetailerResultingFareAttrInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get(this->cacheClass()));
  try
  {
    QueryGetFareRetailerResultingFareAttr q(dbAdapter->getAdapter());
    q.findFareRetailerResultingFareAttr(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareRetailerResultingFareAttrDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareRetailerResultingFareAttrKey
FareRetailerResultingFareAttrDAO::createKey(const FareRetailerResultingFareAttrInfo* info)
{
  return FareRetailerResultingFareAttrKey(info->resultingFareAttrItemNo());
}

void
FareRetailerResultingFareAttrDAO::destroy(FareRetailerResultingFareAttrKey, std::vector<FareRetailerResultingFareAttrInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FareRetailerResultingFareAttrDAO::_name("FareRetailerResultingFareAttr");
std::string
FareRetailerResultingFareAttrDAO::_cacheClass("Rules");
DAOHelper<FareRetailerResultingFareAttrDAO>
FareRetailerResultingFareAttrDAO::_helper(_name);
FareRetailerResultingFareAttrDAO*
FareRetailerResultingFareAttrDAO::_instance(nullptr);

// Historical DAO Object
log4cxx::LoggerPtr
FareRetailerResultingFareAttrHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareRetailerResultingFareAttrHistoricalDAO"));

FareRetailerResultingFareAttrHistoricalDAO&
FareRetailerResultingFareAttrHistoricalDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const FareRetailerResultingFareAttrInfo*
FareRetailerResultingFareAttrHistoricalDAO::get(DeleteList& del,
                                                uint64_t resultingFareAttrItemNo,
                                                DateTime ticketDate)
{
  ++_codeCoverageGetCallCount;

  FareRetailerResultingFareAttrHistoricalKey key(resultingFareAttrItemNo);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr(cache().get(key));
  FareRetailerResultingFareAttrInfo* ret(nullptr);
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsCurrentDH<FareRetailerResultingFareAttrInfo>(ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<FareRetailerResultingFareAttrInfo*>*
FareRetailerResultingFareAttrHistoricalDAO::create(FareRetailerResultingFareAttrHistoricalKey key)
{
  std::vector<FareRetailerResultingFareAttrInfo*>* ret(new std::vector<FareRetailerResultingFareAttrInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get("Historical", true));
  try
  {
    QueryGetFareRetailerResultingFareAttrHistorical q(dbAdapter->getAdapter());
    q.findFareRetailerResultingFareAttr(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareRetailerResultingFareAttrHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareRetailerResultingFareAttrHistoricalKey
FareRetailerResultingFareAttrHistoricalDAO::createKey(const FareRetailerResultingFareAttrInfo* info,
                                                      const DateTime& startDate,
                                                      const DateTime& endDate)
{
  return FareRetailerResultingFareAttrHistoricalKey(info->resultingFareAttrItemNo(), startDate, endDate);
}

void
FareRetailerResultingFareAttrHistoricalDAO::destroy(FareRetailerResultingFareAttrHistoricalKey,
                                       std::vector<FareRetailerResultingFareAttrInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FareRetailerResultingFareAttrHistoricalDAO::_name("FareRetailerResultingFareAttrHistorical");
std::string
FareRetailerResultingFareAttrHistoricalDAO::_cacheClass("Rules");
DAOHelper<FareRetailerResultingFareAttrHistoricalDAO>
FareRetailerResultingFareAttrHistoricalDAO::_helper(_name);
FareRetailerResultingFareAttrHistoricalDAO*
FareRetailerResultingFareAttrHistoricalDAO::_instance(nullptr);

} // namespace tse