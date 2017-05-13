#include "DBAccess/FareRetailerRuleDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareRetailerRuleInfo.h"
#include "DBAccess/Queries/QueryGetFareRetailerRule.h"

namespace tse
{
log4cxx::LoggerPtr
FareRetailerRuleDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FareRetailerRuleDAO"));

FareRetailerRuleDAO&
FareRetailerRuleDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const FareRetailerRuleInfo*
getFareRetailerRuleData(uint64_t fareRetailerRuleId,
                        DateTime adjustedTicketDate,
                        DeleteList& deleteList,
                        DateTime ticketDate,
                        bool isHistorical)
{
  if (isHistorical)
  {
    FareRetailerRuleHistoricalDAO& dao(FareRetailerRuleHistoricalDAO::instance());

    return dao.get(deleteList, fareRetailerRuleId, ticketDate);
  }
  else
  {
    FareRetailerRuleDAO& dao(FareRetailerRuleDAO::instance());

    return dao.get(deleteList, fareRetailerRuleId, adjustedTicketDate, ticketDate);
  }
}

const FareRetailerRuleInfo*
FareRetailerRuleDAO::get(DeleteList& del,
                         uint64_t fareRetailerRuleId,
                         DateTime adjustedTicketDate,
                         DateTime ticketDate)
{
  ++_codeCoverageGetCallCount;

  FareRetailerRuleKey key(fareRetailerRuleId);
  DAOCache::pointer_type ptr(cache().get(key));
  FareRetailerRuleInfo* ret(nullptr);

  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveNow<FareRetailerRuleInfo>(adjustedTicketDate, ticketDate));
  if (i != ptr->end())
    ret = *i;

  return ret;
}

std::vector<FareRetailerRuleInfo*>*
FareRetailerRuleDAO::create(FareRetailerRuleKey key)
{
  std::vector<FareRetailerRuleInfo*>* ret(new std::vector<FareRetailerRuleInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get(this->cacheClass()));
  try
  {
    QueryGetFareRetailerRule q(dbAdapter->getAdapter());
    q.findFareRetailerRule(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareRetailerRuleDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareRetailerRuleKey
FareRetailerRuleDAO::createKey(const FareRetailerRuleInfo* info)
{
  return FareRetailerRuleKey(info->fareRetailerRuleId());
}

void
FareRetailerRuleDAO::destroy(FareRetailerRuleKey, std::vector<FareRetailerRuleInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FareRetailerRuleDAO::_name("FareRetailerRule");
std::string
FareRetailerRuleDAO::_cacheClass("Rules");
DAOHelper<FareRetailerRuleDAO>
FareRetailerRuleDAO::_helper(_name);
FareRetailerRuleDAO*
FareRetailerRuleDAO::_instance(nullptr);

// Historical DAO Object
log4cxx::LoggerPtr
FareRetailerRuleHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareRetailerRuleHistoricalDAO"));

FareRetailerRuleHistoricalDAO&
FareRetailerRuleHistoricalDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const FareRetailerRuleInfo*
FareRetailerRuleHistoricalDAO::get(DeleteList& del,
                                   uint64_t fareRetailerRuleId,
                                    DateTime ticketDate)
{
  ++_codeCoverageGetCallCount;

  FareRetailerRuleHistoricalKey key(fareRetailerRuleId);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr(cache().get(key));
  FareRetailerRuleInfo* ret(nullptr);
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveDH<FareRetailerRuleInfo>(key._b, key._c, ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<FareRetailerRuleInfo*>*
FareRetailerRuleHistoricalDAO::create(FareRetailerRuleHistoricalKey key)
{
  std::vector<FareRetailerRuleInfo*>* ret(new std::vector<FareRetailerRuleInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get("Historical", true));
  try
  {
    QueryGetFareRetailerRuleHistorical q(dbAdapter->getAdapter());
    q.findFareRetailerRule(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareRetailerRuleHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareRetailerRuleHistoricalKey
FareRetailerRuleHistoricalDAO::createKey(const FareRetailerRuleInfo* info,
                                         const DateTime& startDate,
                                         const DateTime& endDate)
{
  return FareRetailerRuleHistoricalKey(info->fareRetailerRuleId(), startDate, endDate);
}

void
FareRetailerRuleHistoricalDAO::destroy(FareRetailerRuleHistoricalKey,
                                       std::vector<FareRetailerRuleInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FareRetailerRuleHistoricalDAO::_name("FareRetailerRuleHistorical");
std::string
FareRetailerRuleHistoricalDAO::_cacheClass("Rules");
DAOHelper<FareRetailerRuleHistoricalDAO>
FareRetailerRuleHistoricalDAO::_helper(_name);
FareRetailerRuleHistoricalDAO*
FareRetailerRuleHistoricalDAO::_instance(nullptr);

} // namespace tse


