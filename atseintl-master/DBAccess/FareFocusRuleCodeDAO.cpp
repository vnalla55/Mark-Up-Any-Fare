#include "DBAccess/FareFocusRuleCodeDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareFocusRuleCodeInfo.h"
#include "DBAccess/Queries/QueryGetFareFocusRuleCode.h"

namespace tse
{
log4cxx::LoggerPtr
FareFocusRuleCodeDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FareFocusRuleCodeDAO"));

FareFocusRuleCodeDAO&
FareFocusRuleCodeDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareFocusRuleCodeInfo*>&
getFareFocusRuleCodeData(uint64_t ruleCdItemNo,
                         DateTime adjustedTicketDate,
                         DeleteList& deleteList,
                         DateTime ticketDate,
                         bool isHistorical)
{
  if (isHistorical)
  {
    FareFocusRuleCodeHistoricalDAO& dao(FareFocusRuleCodeHistoricalDAO::instance());
    const std::vector<FareFocusRuleCodeInfo*>& ret(dao.get(deleteList, ruleCdItemNo, ticketDate));

    return ret;
  }
  else
  {
    FareFocusRuleCodeDAO& dao(FareFocusRuleCodeDAO::instance());
    const std::vector<FareFocusRuleCodeInfo*>& ret(dao.get(deleteList, ruleCdItemNo, adjustedTicketDate, ticketDate));

    return ret;
  }
}

const std::vector<FareFocusRuleCodeInfo*>&
FareFocusRuleCodeDAO::get(DeleteList& del,
                          uint64_t ruleCdItemNo,
                          DateTime adjustedTicketDate,
                          DateTime ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  FareFocusRuleCodeKey key(ruleCdItemNo);
  DAOCache::pointer_type ptr(cache().get(key));
  IsNotCurrentG<FareFocusRuleCodeInfo> isNotCurrent(ticketDate);
  return *(applyFilter(del, ptr, isNotCurrent));
}

std::vector<FareFocusRuleCodeInfo*>*
FareFocusRuleCodeDAO::create(FareFocusRuleCodeKey key)
{
  std::vector<FareFocusRuleCodeInfo*>* ret(new std::vector<FareFocusRuleCodeInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get(this->cacheClass()));
  try
  {
    QueryGetFareFocusRuleCode q(dbAdapter->getAdapter());
    q.findFareFocusRuleCode(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareFocusRuleCodeDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareFocusRuleCodeKey
FareFocusRuleCodeDAO::createKey(const FareFocusRuleCodeInfo* info)
{
  return FareFocusRuleCodeKey(info->ruleCdItemNo());
}

void
FareFocusRuleCodeDAO::destroy(FareFocusRuleCodeKey, std::vector<FareFocusRuleCodeInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FareFocusRuleCodeDAO::_name("FareFocusRuleCode");
std::string
FareFocusRuleCodeDAO::_cacheClass("Rules");
DAOHelper<FareFocusRuleCodeDAO>
FareFocusRuleCodeDAO::_helper(_name);
FareFocusRuleCodeDAO*
FareFocusRuleCodeDAO::_instance(nullptr);

// Historical DAO Object
log4cxx::LoggerPtr
FareFocusRuleCodeHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareFocusRuleCodeHistoricalDAO"));

FareFocusRuleCodeHistoricalDAO&
FareFocusRuleCodeHistoricalDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareFocusRuleCodeInfo*>&
FareFocusRuleCodeHistoricalDAO::get(DeleteList& del,
                                    uint64_t ruleCdItemNo,
                                    const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  FareFocusRuleCodeHistoricalKey key(ruleCdItemNo);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr(cache().get(key));
  del.copy(ptr);
  std::vector<FareFocusRuleCodeInfo*>* ret(new std::vector<FareFocusRuleCodeInfo*>);
  del.adopt(ret);
  IsNotCurrentDH<FareFocusRuleCodeInfo> isNotCurrent(ticketDate);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), isNotCurrent);
  return *ret;
}

std::vector<FareFocusRuleCodeInfo*>*
FareFocusRuleCodeHistoricalDAO::create(FareFocusRuleCodeHistoricalKey key)
{
  std::vector<FareFocusRuleCodeInfo*>* ret(new std::vector<FareFocusRuleCodeInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get("Historical", true));
  try
  {
    QueryGetFareFocusRuleCodeHistorical q(dbAdapter->getAdapter());
    q.findFareFocusRuleCode(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareFocusRuleCodeHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareFocusRuleCodeHistoricalKey
FareFocusRuleCodeHistoricalDAO::createKey(const FareFocusRuleCodeInfo* info,
                                          const DateTime& startDate,
                                          const DateTime& endDate)
{
  return FareFocusRuleCodeHistoricalKey(info->ruleCdItemNo(), startDate, endDate);
}

void
FareFocusRuleCodeHistoricalDAO::destroy(FareFocusRuleCodeHistoricalKey,
                                        std::vector<FareFocusRuleCodeInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FareFocusRuleCodeHistoricalDAO::_name("FareFocusRuleCodeHistorical");
std::string
FareFocusRuleCodeHistoricalDAO::_cacheClass("Rules");
DAOHelper<FareFocusRuleCodeHistoricalDAO>
FareFocusRuleCodeHistoricalDAO::_helper(_name);
FareFocusRuleCodeHistoricalDAO*
FareFocusRuleCodeHistoricalDAO::_instance(nullptr);

} // namespace tse

