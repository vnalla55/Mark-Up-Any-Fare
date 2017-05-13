#include "DBAccess/FareRetailerCalcDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareRetailerCalcInfo.h"
#include "DBAccess/Queries/QueryGetFareRetailerCalc.h"

namespace tse
{
log4cxx::LoggerPtr
FareRetailerCalcDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FareRetailerCalcDAO"));

FareRetailerCalcDAO&
FareRetailerCalcDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareRetailerCalcInfo*>&
getFareRetailerCalcData(uint64_t fareRetailerCalcItemNo,
                        DateTime adjustedTicketDate,
                        DeleteList& deleteList,
                        DateTime ticketDate,
                        bool isHistorical)
{
  if (isHistorical)
  {
    FareRetailerCalcHistoricalDAO& dao(FareRetailerCalcHistoricalDAO::instance());
    const std::vector<FareRetailerCalcInfo*>& ret(dao.get(deleteList, fareRetailerCalcItemNo, ticketDate));

    return ret;
  }
  else
  {
    FareRetailerCalcDAO& dao(FareRetailerCalcDAO::instance());
    const std::vector<FareRetailerCalcInfo*>& ret(dao.get(deleteList, fareRetailerCalcItemNo, adjustedTicketDate, ticketDate));

    return ret;
  }
}

const std::vector<FareRetailerCalcInfo*>&
FareRetailerCalcDAO::get(DeleteList& del,
                         uint64_t fareRetailerCalcItemNo,
                         DateTime adjustedTicketDate,
                         DateTime ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  FareRetailerCalcKey key(fareRetailerCalcItemNo);
  DAOCache::pointer_type ptr(cache().get(key));
  IsNotCurrentG<FareRetailerCalcInfo> isNotCurrent(ticketDate);
  return *(applyFilter(del, ptr, isNotCurrent));
}

std::vector<FareRetailerCalcInfo*>*
FareRetailerCalcDAO::create(FareRetailerCalcKey key)
{
  std::vector<FareRetailerCalcInfo*>* ret(new std::vector<FareRetailerCalcInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get(this->cacheClass()));
  try
  {
    QueryGetFareRetailerCalc q(dbAdapter->getAdapter());
    q.findFareRetailerCalc(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareRetailerCalcDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareRetailerCalcKey
FareRetailerCalcDAO::createKey(const FareRetailerCalcInfo* info)
{
  return FareRetailerCalcKey(info->fareRetailerCalcItemNo());
}

void
FareRetailerCalcDAO::destroy(FareRetailerCalcKey, std::vector<FareRetailerCalcInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FareRetailerCalcDAO::_name("FareRetailerCalc");
std::string
FareRetailerCalcDAO::_cacheClass("Rules");
DAOHelper<FareRetailerCalcDAO>
FareRetailerCalcDAO::_helper(_name);
FareRetailerCalcDAO*
FareRetailerCalcDAO::_instance(nullptr);

// Historical DAO Object
log4cxx::LoggerPtr
FareRetailerCalcHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareRetailerCalcHistoricalDAO"));

FareRetailerCalcHistoricalDAO&
FareRetailerCalcHistoricalDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareRetailerCalcInfo*>&
FareRetailerCalcHistoricalDAO::get(DeleteList& del,
                                   uint64_t fareRetailerCalcItemNo,
                                   const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  FareRetailerCalcHistoricalKey key(fareRetailerCalcItemNo);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr(cache().get(key));
  del.copy(ptr);
  std::vector<FareRetailerCalcInfo*>* ret(new std::vector<FareRetailerCalcInfo*>);
  del.adopt(ret);
  IsNotCurrentDH<FareRetailerCalcInfo> isNotCurrent(ticketDate);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), isNotCurrent);
  return *ret;
}

std::vector<FareRetailerCalcInfo*>*
FareRetailerCalcHistoricalDAO::create(FareRetailerCalcHistoricalKey key)
{
  std::vector<FareRetailerCalcInfo*>* ret(new std::vector<FareRetailerCalcInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get("Historical", true));
  try
  {
    QueryGetFareRetailerCalcHistorical q(dbAdapter->getAdapter());
    q.findFareRetailerCalc(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareRetailerCalcHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareRetailerCalcHistoricalKey
FareRetailerCalcHistoricalDAO::createKey(const FareRetailerCalcInfo* info,
                                         const DateTime& startDate,
                                         const DateTime& endDate)
{
  return FareRetailerCalcHistoricalKey(info->fareRetailerCalcItemNo(), startDate, endDate);
}

void
FareRetailerCalcHistoricalDAO::destroy(FareRetailerCalcHistoricalKey,
                                       std::vector<FareRetailerCalcInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FareRetailerCalcHistoricalDAO::_name("FareRetailerCalcHistorical");
std::string
FareRetailerCalcHistoricalDAO::_cacheClass("Rules");
DAOHelper<FareRetailerCalcHistoricalDAO>
FareRetailerCalcHistoricalDAO::_helper(_name);
FareRetailerCalcHistoricalDAO*
FareRetailerCalcHistoricalDAO::_instance(nullptr);

} // namespace tse



