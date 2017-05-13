#include "DBAccess/FareFocusAccountCdDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareFocusAccountCdInfo.h"
#include "DBAccess/Queries/QueryGetFareFocusAccountCd.h"

namespace tse
{
log4cxx::LoggerPtr
FareFocusAccountCdDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareFocusAccountCdDAO"));

FareFocusAccountCdDAO&
FareFocusAccountCdDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const FareFocusAccountCdInfo*
getFareFocusAccountCdData(uint64_t accountCdItemNo,
                         DateTime adjustedTicketDate,
                         DeleteList& deleteList,
                         DateTime ticketDate,
                         bool isHistorical)
{
  if (isHistorical)
  {
    FareFocusAccountCdHistoricalDAO& dao(FareFocusAccountCdHistoricalDAO::instance());

    return dao.get(deleteList, accountCdItemNo, ticketDate);
  }
  else
  {
    FareFocusAccountCdDAO& dao(FareFocusAccountCdDAO::instance());

    return dao.get(deleteList, accountCdItemNo, adjustedTicketDate, ticketDate);
  }
}

const FareFocusAccountCdInfo*
FareFocusAccountCdDAO::get(DeleteList& del,
                           uint64_t accountCdItemNo,
                           DateTime adjustedTicketDate,
                           DateTime ticketDate)
{
  ++_codeCoverageGetCallCount;

  FareFocusAccountCdKey key(accountCdItemNo);
  DAOCache::pointer_type ptr(cache().get(key));
  FareFocusAccountCdInfo* ret(nullptr);
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsCurrentG<FareFocusAccountCdInfo>(ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<FareFocusAccountCdInfo*>*
FareFocusAccountCdDAO::create(FareFocusAccountCdKey key)
{
  std::vector<FareFocusAccountCdInfo*>* ret(new std::vector<FareFocusAccountCdInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get(this->cacheClass()));
  try
  {
    QueryGetFareFocusAccountCd q(dbAdapter->getAdapter());
    q.findFareFocusAccountCd(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareFocusAccountCdDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareFocusAccountCdKey
FareFocusAccountCdDAO::createKey(const FareFocusAccountCdInfo* info)
{
  return FareFocusAccountCdKey(info->accountCdItemNo());
}

void
FareFocusAccountCdDAO::destroy(FareFocusAccountCdKey, std::vector<FareFocusAccountCdInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FareFocusAccountCdDAO::_name("FareFocusAccountCd");
std::string
FareFocusAccountCdDAO::_cacheClass("Rules");
DAOHelper<FareFocusAccountCdDAO>
FareFocusAccountCdDAO::_helper(_name);
FareFocusAccountCdDAO*
FareFocusAccountCdDAO::_instance(nullptr);

// Historical DAO Object
log4cxx::LoggerPtr
FareFocusAccountCdHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareFocusAccountCdHistoricalDAO"));

FareFocusAccountCdHistoricalDAO&
FareFocusAccountCdHistoricalDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const FareFocusAccountCdInfo*
FareFocusAccountCdHistoricalDAO::get(DeleteList& del,
                                     uint64_t accountCdItemNo,
                                     DateTime ticketDate)
{
  ++_codeCoverageGetCallCount;

  FareFocusAccountCdHistoricalKey key(accountCdItemNo);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr(cache().get(key));
  FareFocusAccountCdInfo* ret(nullptr);
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsCurrentDH<FareFocusAccountCdInfo>(ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<FareFocusAccountCdInfo*>*
FareFocusAccountCdHistoricalDAO::create(FareFocusAccountCdHistoricalKey key)
{
  std::vector<FareFocusAccountCdInfo*>* ret(new std::vector<FareFocusAccountCdInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get("Historical", true));
  try
  {
    QueryGetFareFocusAccountCdHistorical q(dbAdapter->getAdapter());
    q.findFareFocusAccountCd(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareFocusAccountCdHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareFocusAccountCdHistoricalKey
FareFocusAccountCdHistoricalDAO::createKey(const FareFocusAccountCdInfo* info,
                                           const DateTime& startDate,
                                           const DateTime& endDate)
{
  return FareFocusAccountCdHistoricalKey(info->accountCdItemNo(), startDate, endDate);
}

void
FareFocusAccountCdHistoricalDAO::destroy(FareFocusAccountCdHistoricalKey,
                                         std::vector<FareFocusAccountCdInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FareFocusAccountCdHistoricalDAO::_name("FareFocusAccountCdHistorical");
std::string
FareFocusAccountCdHistoricalDAO::_cacheClass("Rules");
DAOHelper<FareFocusAccountCdHistoricalDAO>
FareFocusAccountCdHistoricalDAO::_helper(_name);
FareFocusAccountCdHistoricalDAO*
FareFocusAccountCdHistoricalDAO::_instance(nullptr);

} // namespace tse
