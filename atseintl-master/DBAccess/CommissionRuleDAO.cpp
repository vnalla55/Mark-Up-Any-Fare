#include "DBAccess/CommissionRuleDAO.h"

#include "Common/Logger.h"
#include "DBAccess/CommissionRuleInfo.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/Queries/QueryGetCommissionRule.h"

namespace tse
{
log4cxx::LoggerPtr
CommissionRuleDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.CommissionRuleDAO"));

CommissionRuleDAO& CommissionRuleDAO::instance()
{
  if (0 == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<CommissionRuleInfo*>& getCommissionRuleData(const VendorCode& vendor,
                                                              uint64_t programId,
                                                              DeleteList& deleteList,
                                                              DateTime ticketDate,
                                                              bool isHistorical)
{
  if (isHistorical)
  {
    CommissionRuleHistoricalDAO& dao(CommissionRuleHistoricalDAO::instance());

    return dao.get(deleteList, vendor, programId, ticketDate);
  }
  else
  {
    CommissionRuleDAO& dao(CommissionRuleDAO::instance());

    return dao.get(deleteList, vendor, programId, ticketDate);
  }
}

const std::vector<CommissionRuleInfo*>& CommissionRuleDAO::get(DeleteList& del,
                                                               const VendorCode& vendor,
                                                               uint64_t programId,
                                                               DateTime ticketDate)
{
  ++_codeCoverageGetCallCount;
  CommissionRuleKey key(vendor, programId);
  DAOCache::pointer_type ptr(cache().get(key));
  IsNotEffectiveT<CommissionRuleInfo> isNotEffective(ticketDate);
  return *applyFilter(del, ptr, isNotEffective);
}

std::vector<CommissionRuleInfo*>* CommissionRuleDAO::create(CommissionRuleKey key)
{
  std::vector<CommissionRuleInfo*>* ret(new std::vector<CommissionRuleInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get(this->cacheClass()));
  try
  {
    QueryGetCommissionRule q(dbAdapter->getAdapter());
    q.findCommissionRule(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CommissionRuleDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

CommissionRuleKey CommissionRuleDAO::createKey(const CommissionRuleInfo* info)
{
  return CommissionRuleKey(info->vendor(), info->programId());
}

void CommissionRuleDAO::destroy(CommissionRuleKey,
                                std::vector<CommissionRuleInfo*>* recs)
{
  destroyContainer(recs);
}

std::string CommissionRuleDAO::_name("CommissionRule");
std::string CommissionRuleDAO::_cacheClass("Rules");
DAOHelper<CommissionRuleDAO> CommissionRuleDAO::_helper(_name);
CommissionRuleDAO* CommissionRuleDAO::_instance(0);

// Historical DAO Object
log4cxx::LoggerPtr CommissionRuleHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.CommissionRuleHistoricalDAO"));

CommissionRuleHistoricalDAO& CommissionRuleHistoricalDAO::instance()
{
  if (0 == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<CommissionRuleInfo*>& CommissionRuleHistoricalDAO::get(DeleteList& del,
                                                                         const VendorCode& vendor,
                                                                         uint64_t programId,
                                                                         DateTime ticketDate)
{
  ++_codeCoverageGetCallCount;

  CommissionRuleHistoricalKey key(vendor, programId);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr(cache().get(key));
  del.copy(ptr);
  std::vector<CommissionRuleInfo*>* ret(new std::vector<CommissionRuleInfo*>);
  del.adopt(ret);
  IsNotEffectiveH<CommissionRuleInfo> isNotEffective(ticketDate, ticketDate);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), isNotEffective);
  return *ret;
}

std::vector<CommissionRuleInfo*>* CommissionRuleHistoricalDAO::create(CommissionRuleHistoricalKey key)
{
  std::vector<CommissionRuleInfo*>* ret(new std::vector<CommissionRuleInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get("Historical", true));
  try
  {
    QueryGetCommissionRuleHistorical q(dbAdapter->getAdapter());
    q.findCommissionRule(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CommissionRuleHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

CommissionRuleHistoricalKey CommissionRuleHistoricalDAO::createKey(const CommissionRuleInfo* info,
                                                                   const DateTime& startDate,
                                                                   const DateTime& endDate)
{
  return CommissionRuleHistoricalKey(info->vendor(), info->programId(), startDate, endDate);
}

void CommissionRuleHistoricalDAO::destroy(CommissionRuleHistoricalKey,
                                          std::vector<CommissionRuleInfo*>* recs)
{
  destroyContainer(recs);
}

std::string CommissionRuleHistoricalDAO::_name("CommissionRuleHistorical");
std::string CommissionRuleHistoricalDAO::_cacheClass("Rules");
DAOHelper<CommissionRuleHistoricalDAO> CommissionRuleHistoricalDAO::_helper(_name);
CommissionRuleHistoricalDAO* CommissionRuleHistoricalDAO::_instance(0);

}// tse
