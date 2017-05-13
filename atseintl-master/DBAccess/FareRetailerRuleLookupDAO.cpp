#include "DBAccess/FareRetailerRuleLookupDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareRetailerRuleLookupInfo.h"
#include "DBAccess/Queries/QueryGetFareRetailerRuleLookup.h"

namespace tse
{
log4cxx::LoggerPtr
FareRetailerRuleLookupDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareRetailerRuleLookupDAO"));

FareRetailerRuleLookupDAO&
FareRetailerRuleLookupDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const FareRetailerRuleLookupInfo*
getFareRetailerRuleLookupData(Indicator applicationType,
                              const PseudoCityCode& sourcePcc,
                              const PseudoCityCode& pcc,
                              DeleteList& deleteList,
                              DateTime ticketDate,
                              bool isHistorical)
{
  if (isHistorical)
  {
    FareRetailerRuleLookupHistoricalDAO& dao(FareRetailerRuleLookupHistoricalDAO::instance());

    return dao.get(deleteList, applicationType, sourcePcc, pcc, ticketDate);
  }
  else
  {
    FareRetailerRuleLookupDAO& dao(FareRetailerRuleLookupDAO::instance());

    return dao.get(deleteList, applicationType, sourcePcc, pcc, ticketDate);
  }
}

const FareRetailerRuleLookupInfo*
FareRetailerRuleLookupDAO::get(DeleteList& del,
                               Indicator applicationType,
                               const PseudoCityCode& sourcePcc,
                               const PseudoCityCode& pcc,
                               DateTime ticketDate)
{
  ++_codeCoverageGetCallCount;

  FareRetailerRuleLookupKey key(applicationType, sourcePcc, pcc);
  DAOCache::pointer_type ptr(cache().get(key));
  FareRetailerRuleLookupInfo* ret(nullptr);
  if (!ptr->empty())
  {
    ret = ptr->front();
  }
  return ret;
}

std::vector<FareRetailerRuleLookupInfo*>*
FareRetailerRuleLookupDAO::create(FareRetailerRuleLookupKey key)
{
  std::vector<FareRetailerRuleLookupInfo*>* ret(new std::vector<FareRetailerRuleLookupInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get(this->cacheClass()));
  try
  {
    QueryGetFareRetailerRuleLookup q(dbAdapter->getAdapter());
    q.findFareRetailerRuleLookup(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareRetailerRuleLookupDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareRetailerRuleLookupKey
FareRetailerRuleLookupDAO::createKey(const FareRetailerRuleLookupInfo* info)
{
  return FareRetailerRuleLookupKey(info->applicationType(), info->sourcePcc(), info->pcc());
}

void
FareRetailerRuleLookupDAO::destroy(FareRetailerRuleLookupKey, std::vector<FareRetailerRuleLookupInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FareRetailerRuleLookupDAO::_name("FareRetailerRuleLookup");
std::string
FareRetailerRuleLookupDAO::_cacheClass("Rules");
DAOHelper<FareRetailerRuleLookupDAO>
FareRetailerRuleLookupDAO::_helper(_name);
FareRetailerRuleLookupDAO*
FareRetailerRuleLookupDAO::_instance(nullptr);

// Historical DAO Object
log4cxx::LoggerPtr
FareRetailerRuleLookupHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareRetailerRuleLookupHistoricalDAO"));

FareRetailerRuleLookupHistoricalDAO&
FareRetailerRuleLookupHistoricalDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const FareRetailerRuleLookupInfo*
FareRetailerRuleLookupHistoricalDAO::get(DeleteList& del,
                                         Indicator applicationType,
                                         const PseudoCityCode& sourcePcc,
                                         const PseudoCityCode& pcc,
                                         DateTime ticketDate)
{
  ++_codeCoverageGetCallCount;

  FareRetailerRuleLookupHistoricalKey key(applicationType, sourcePcc, pcc);
  DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  DAOCache::pointer_type ptr(cache().get(key));
  del.copy(ptr);
  FareRetailerRuleLookupInfo* ret(nullptr);
  if (!ptr->empty())
  {
    ret = ptr->front();
  }
  return ret;
}

std::vector<FareRetailerRuleLookupInfo*>*
FareRetailerRuleLookupHistoricalDAO::create(FareRetailerRuleLookupHistoricalKey key)
{
  std::vector<FareRetailerRuleLookupInfo*>* ret(new std::vector<FareRetailerRuleLookupInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get("Historical", true));
  try
  {
    QueryGetFareRetailerRuleLookupHistorical q(dbAdapter->getAdapter());
    q.findFareRetailerRuleLookup(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareRetailerRuleLookupHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareRetailerRuleLookupHistoricalKey
FareRetailerRuleLookupHistoricalDAO::createKey(const FareRetailerRuleLookupInfo* info,
                                               const DateTime& startDate,
                                               const DateTime& endDate)
{
  return FareRetailerRuleLookupHistoricalKey(info->applicationType(), info->sourcePcc(), info->pcc(), startDate, endDate);
}

void
FareRetailerRuleLookupHistoricalDAO::destroy(FareRetailerRuleLookupHistoricalKey,
                                             std::vector<FareRetailerRuleLookupInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FareRetailerRuleLookupHistoricalDAO::_name("FareRetailerRuleLookupHistorical");
std::string
FareRetailerRuleLookupHistoricalDAO::_cacheClass("Rules");
DAOHelper<FareRetailerRuleLookupHistoricalDAO>
FareRetailerRuleLookupHistoricalDAO::_helper(_name);
FareRetailerRuleLookupHistoricalDAO*
FareRetailerRuleLookupHistoricalDAO::_instance(nullptr);

} // namespace tse
