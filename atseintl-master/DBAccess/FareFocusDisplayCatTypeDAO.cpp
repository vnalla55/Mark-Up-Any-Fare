#include "DBAccess/FareFocusDisplayCatTypeDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareFocusDisplayCatTypeInfo.h"
#include "DBAccess/Queries/QueryGetFareFocusDisplayCatType.h"

namespace tse
{
log4cxx::LoggerPtr
FareFocusDisplayCatTypeDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareFocusDisplayCatTypeDAO"));

FareFocusDisplayCatTypeDAO&
FareFocusDisplayCatTypeDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const FareFocusDisplayCatTypeInfo*
getFareFocusDisplayCatTypeData(uint64_t displayCatTypeItemNo,
                         DateTime adjustedTicketDate,
                         DeleteList& deleteList,
                         DateTime ticketDate,
                         bool isHistorical)
{
  if (isHistorical)
  {
    FareFocusDisplayCatTypeHistoricalDAO& dao(FareFocusDisplayCatTypeHistoricalDAO::instance());

    return dao.get(deleteList, displayCatTypeItemNo, ticketDate);
  }
  else
  {
    FareFocusDisplayCatTypeDAO& dao(FareFocusDisplayCatTypeDAO::instance());

    return dao.get(deleteList, displayCatTypeItemNo, adjustedTicketDate, ticketDate);
  }
}

const FareFocusDisplayCatTypeInfo*
FareFocusDisplayCatTypeDAO::get(DeleteList& del,
                          uint64_t displayCatTypeItemNo,
                          DateTime adjustedTicketDate,
                          DateTime ticketDate)
{
  ++_codeCoverageGetCallCount;

  FareFocusDisplayCatTypeKey key(displayCatTypeItemNo);
  DAOCache::pointer_type ptr(cache().get(key));
  FareFocusDisplayCatTypeInfo* ret(nullptr);
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsCurrentG<FareFocusDisplayCatTypeInfo>(ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<FareFocusDisplayCatTypeInfo*>*
FareFocusDisplayCatTypeDAO::create(FareFocusDisplayCatTypeKey key)
{
  std::vector<FareFocusDisplayCatTypeInfo*>* ret(new std::vector<FareFocusDisplayCatTypeInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get(this->cacheClass()));
  try
  {
    QueryGetFareFocusDisplayCatType q(dbAdapter->getAdapter());
    q.findFareFocusDisplayCatType(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareFocusDisplayCatTypeDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareFocusDisplayCatTypeKey
FareFocusDisplayCatTypeDAO::createKey(const FareFocusDisplayCatTypeInfo* info)
{
  return FareFocusDisplayCatTypeKey(info->displayCatTypeItemNo());
}

void
FareFocusDisplayCatTypeDAO::destroy(FareFocusDisplayCatTypeKey, std::vector<FareFocusDisplayCatTypeInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FareFocusDisplayCatTypeDAO::_name("FareFocusDisplayCatType");
std::string
FareFocusDisplayCatTypeDAO::_cacheClass("Rules");
DAOHelper<FareFocusDisplayCatTypeDAO>
FareFocusDisplayCatTypeDAO::_helper(_name);
FareFocusDisplayCatTypeDAO*
FareFocusDisplayCatTypeDAO::_instance(nullptr);

// Historical DAO Object
log4cxx::LoggerPtr
FareFocusDisplayCatTypeHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareFocusDisplayCatTypeHistoricalDAO"));

FareFocusDisplayCatTypeHistoricalDAO&
FareFocusDisplayCatTypeHistoricalDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const FareFocusDisplayCatTypeInfo*
FareFocusDisplayCatTypeHistoricalDAO::get(DeleteList& del,
                                     uint64_t displayCatTypeItemNo,
                                     DateTime ticketDate)
{
  ++_codeCoverageGetCallCount;

  FareFocusDisplayCatTypeHistoricalKey key(displayCatTypeItemNo);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr(cache().get(key));
  FareFocusDisplayCatTypeInfo* ret(nullptr);
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsCurrentDH<FareFocusDisplayCatTypeInfo>(ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<FareFocusDisplayCatTypeInfo*>*
FareFocusDisplayCatTypeHistoricalDAO::create(FareFocusDisplayCatTypeHistoricalKey key)
{
  std::vector<FareFocusDisplayCatTypeInfo*>* ret(new std::vector<FareFocusDisplayCatTypeInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get("Historical", true));
  try
  {
    QueryGetFareFocusDisplayCatTypeHistorical q(dbAdapter->getAdapter());
    q.findFareFocusDisplayCatType(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareFocusDisplayCatTypeHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareFocusDisplayCatTypeHistoricalKey
FareFocusDisplayCatTypeHistoricalDAO::createKey(const FareFocusDisplayCatTypeInfo* info,
                                           const DateTime& startDate,
                                           const DateTime& endDate)
{
  return FareFocusDisplayCatTypeHistoricalKey(info->displayCatTypeItemNo(), startDate, endDate);
}

void
FareFocusDisplayCatTypeHistoricalDAO::destroy(FareFocusDisplayCatTypeHistoricalKey,
                                         std::vector<FareFocusDisplayCatTypeInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FareFocusDisplayCatTypeHistoricalDAO::_name("FareFocusDisplayCatTypeHistorical");
std::string
FareFocusDisplayCatTypeHistoricalDAO::_cacheClass("Rules");
DAOHelper<FareFocusDisplayCatTypeHistoricalDAO>
FareFocusDisplayCatTypeHistoricalDAO::_helper(_name);
FareFocusDisplayCatTypeHistoricalDAO*
FareFocusDisplayCatTypeHistoricalDAO::_instance(nullptr);

} // namespace tse
