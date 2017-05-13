#include "DBAccess/FareFocusSecurityDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareFocusSecurityInfo.h"
#include "DBAccess/Queries/QueryGetFareFocusSecurity.h"

namespace tse
{
log4cxx::LoggerPtr
FareFocusSecurityDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FareFocusSecurityDAO"));

FareFocusSecurityDAO&
FareFocusSecurityDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const FareFocusSecurityInfo*
getFareFocusSecurityData(uint64_t securityItemNo,
                         DateTime adjustedTicketDate,
                         DeleteList& deleteList,
                         DateTime ticketDate,
                         bool isHistorical)
{
  if (isHistorical)
  {
    FareFocusSecurityHistoricalDAO& dao(FareFocusSecurityHistoricalDAO::instance());

    return dao.get(deleteList, securityItemNo, ticketDate);
  }
  else
  {
    FareFocusSecurityDAO& dao(FareFocusSecurityDAO::instance());

    return dao.get(deleteList, securityItemNo, adjustedTicketDate, ticketDate);
  }
}

const FareFocusSecurityInfo*
FareFocusSecurityDAO::get(DeleteList& del,
                          uint64_t securityItemNo,
                          DateTime adjustedTicketDate,
                          DateTime ticketDate)
{
  ++_codeCoverageGetCallCount;

  FareFocusSecurityKey key(securityItemNo);
  DAOCache::pointer_type ptr(cache().get(key));
  FareFocusSecurityInfo* ret(nullptr);
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsCurrentG<FareFocusSecurityInfo>(ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<FareFocusSecurityInfo*>*
FareFocusSecurityDAO::create(FareFocusSecurityKey key)
{
  std::vector<FareFocusSecurityInfo*>* ret(new std::vector<FareFocusSecurityInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get(this->cacheClass()));
  try
  {
    QueryGetFareFocusSecurity q(dbAdapter->getAdapter());
    q.findFareFocusSecurity(*ret, key._a);
    /*
    std::cerr << "  !! QueryGetFareFocusSecurity: " << ' ' << ret->size() << " !!" << std::endl;
    std::vector<FareFocusSecurityInfo*> *all(new std::vector<FareFocusSecurityInfo*>);
    QueryGetAllFareFocusSecurity qall(dbAdapter->getAdapter());
    qall.findAllFareFocusSecurity(*all);
    std::cerr << "  !! QueryGetAllFareFocusSecurity: " << ' ' << all->size() << " !!" << std::endl;
    */
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareFocusSecurityDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareFocusSecurityKey
FareFocusSecurityDAO::createKey(const FareFocusSecurityInfo* info)
{
  return FareFocusSecurityKey(info->securityItemNo());
}

void
FareFocusSecurityDAO::destroy(FareFocusSecurityKey, std::vector<FareFocusSecurityInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FareFocusSecurityDAO::_name("FareFocusSecurity");
std::string
FareFocusSecurityDAO::_cacheClass("Rules");
DAOHelper<FareFocusSecurityDAO>
FareFocusSecurityDAO::_helper(_name);
FareFocusSecurityDAO*
FareFocusSecurityDAO::_instance(nullptr);

// Historical DAO Object
log4cxx::LoggerPtr
FareFocusSecurityHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareFocusSecurityHistoricalDAO"));

FareFocusSecurityHistoricalDAO&
FareFocusSecurityHistoricalDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const FareFocusSecurityInfo*
FareFocusSecurityHistoricalDAO::get(DeleteList& del,
                                    uint64_t securityItemNo,
                                    DateTime ticketDate)
{
  ++_codeCoverageGetCallCount;

  FareFocusSecurityHistoricalKey key(securityItemNo);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr(cache().get(key));
  FareFocusSecurityInfo* ret(nullptr);
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsCurrentDH<FareFocusSecurityInfo>(ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<FareFocusSecurityInfo*>*
FareFocusSecurityHistoricalDAO::create(FareFocusSecurityHistoricalKey key)
{
  std::vector<FareFocusSecurityInfo*>* ret(new std::vector<FareFocusSecurityInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get("Historical", true));
  try
  {
    QueryGetFareFocusSecurityHistorical q(dbAdapter->getAdapter());
    q.findFareFocusSecurity(*ret, key._a, key._b, key._c);
    //std::cerr << "!!!! QueryGetFareFocusSecurityHistorical: " << ' ' << ret->size() << std::endl;
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareFocusSecurityHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareFocusSecurityHistoricalKey
FareFocusSecurityHistoricalDAO::createKey(const FareFocusSecurityInfo* info,
                                          const DateTime& startDate,
                                          const DateTime& endDate)
{
  return FareFocusSecurityHistoricalKey(info->securityItemNo(), startDate, endDate);
}

void
FareFocusSecurityHistoricalDAO::destroy(FareFocusSecurityHistoricalKey,
                                        std::vector<FareFocusSecurityInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FareFocusSecurityHistoricalDAO::_name("FareFocusSecurityHistorical");
std::string
FareFocusSecurityHistoricalDAO::_cacheClass("Rules");
DAOHelper<FareFocusSecurityHistoricalDAO>
FareFocusSecurityHistoricalDAO::_helper(_name);
FareFocusSecurityHistoricalDAO*
FareFocusSecurityHistoricalDAO::_instance(nullptr);

} // namespace tse
