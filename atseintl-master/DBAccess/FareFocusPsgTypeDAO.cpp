#include "DBAccess/FareFocusPsgTypeDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareFocusPsgTypeInfo.h"
#include "DBAccess/Queries/QueryGetFareFocusPsgType.h"

namespace tse
{
log4cxx::LoggerPtr
FareFocusPsgTypeDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FareFocusPsgTypeDAO"));

FareFocusPsgTypeDAO&
FareFocusPsgTypeDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const FareFocusPsgTypeInfo*
getFareFocusPsgTypeData(uint64_t psgTypeItemNo,
                         DateTime adjustedTicketDate,
                         DeleteList& deleteList,
                         DateTime ticketDate,
                         bool isHistorical)
{
  if (isHistorical)
  {
    FareFocusPsgTypeHistoricalDAO& dao(FareFocusPsgTypeHistoricalDAO::instance());

    return dao.get(deleteList, psgTypeItemNo, ticketDate);
  }
  else
  {
    FareFocusPsgTypeDAO& dao(FareFocusPsgTypeDAO::instance());

    return dao.get(deleteList, psgTypeItemNo, adjustedTicketDate, ticketDate);
  }
}

const FareFocusPsgTypeInfo*
FareFocusPsgTypeDAO::get(DeleteList& del,
                          uint64_t psgTypeItemNo,
                          DateTime adjustedTicketDate,
                          DateTime ticketDate)
{
  ++_codeCoverageGetCallCount;

  FareFocusPsgTypeKey key(psgTypeItemNo);
  DAOCache::pointer_type ptr(cache().get(key));
  FareFocusPsgTypeInfo* ret(nullptr);
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsCurrentG<FareFocusPsgTypeInfo>(ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<FareFocusPsgTypeInfo*>*
FareFocusPsgTypeDAO::create(FareFocusPsgTypeKey key)
{
  std::vector<FareFocusPsgTypeInfo*>* ret(new std::vector<FareFocusPsgTypeInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get(this->cacheClass()));
  try
  {
    QueryGetFareFocusPsgType q(dbAdapter->getAdapter());
    q.findFareFocusPsgType(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareFocusPsgTypeDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareFocusPsgTypeKey
FareFocusPsgTypeDAO::createKey(const FareFocusPsgTypeInfo* info)
{
  return FareFocusPsgTypeKey(info->psgTypeItemNo());
}

void
FareFocusPsgTypeDAO::destroy(FareFocusPsgTypeKey, std::vector<FareFocusPsgTypeInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FareFocusPsgTypeDAO::_name("FareFocusPsgType");
std::string
FareFocusPsgTypeDAO::_cacheClass("Rules");
DAOHelper<FareFocusPsgTypeDAO>
FareFocusPsgTypeDAO::_helper(_name);
FareFocusPsgTypeDAO*
FareFocusPsgTypeDAO::_instance(nullptr);

// Historical DAO Object
log4cxx::LoggerPtr
FareFocusPsgTypeHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareFocusPsgTypeHistoricalDAO"));

FareFocusPsgTypeHistoricalDAO&
FareFocusPsgTypeHistoricalDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const FareFocusPsgTypeInfo*
FareFocusPsgTypeHistoricalDAO::get(DeleteList& del,
                                     uint64_t psgTypeItemNo,
                                     DateTime ticketDate)
{
  ++_codeCoverageGetCallCount;

  FareFocusPsgTypeHistoricalKey key(psgTypeItemNo);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr(cache().get(key));
  FareFocusPsgTypeInfo* ret(nullptr);
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsCurrentDH<FareFocusPsgTypeInfo>(ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<FareFocusPsgTypeInfo*>*
FareFocusPsgTypeHistoricalDAO::create(FareFocusPsgTypeHistoricalKey key)
{
  std::vector<FareFocusPsgTypeInfo*>* ret(new std::vector<FareFocusPsgTypeInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get("Historical", true));
  try
  {
    QueryGetFareFocusPsgTypeHistorical q(dbAdapter->getAdapter());
    q.findFareFocusPsgType(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareFocusPsgTypeHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareFocusPsgTypeHistoricalKey
FareFocusPsgTypeHistoricalDAO::createKey(const FareFocusPsgTypeInfo* info,
                                           const DateTime& startDate,
                                           const DateTime& endDate)
{
  return FareFocusPsgTypeHistoricalKey(info->psgTypeItemNo(), startDate, endDate);
}

void
FareFocusPsgTypeHistoricalDAO::destroy(FareFocusPsgTypeHistoricalKey,
                                         std::vector<FareFocusPsgTypeInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FareFocusPsgTypeHistoricalDAO::_name("FareFocusPsgTypeHistorical");
std::string
FareFocusPsgTypeHistoricalDAO::_cacheClass("Rules");
DAOHelper<FareFocusPsgTypeHistoricalDAO>
FareFocusPsgTypeHistoricalDAO::_helper(_name);
FareFocusPsgTypeHistoricalDAO*
FareFocusPsgTypeHistoricalDAO::_instance(nullptr);

} // namespace tse
