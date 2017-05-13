#include "DBAccess/RBDByCabinDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetRBDByCabin.h"
#include "DBAccess/RBDByCabinInfo.h"

namespace tse
{
/*
namespace
{
void displayQueryResults(const char* type,
                         const std::vector<RBDByCabinInfo*>& vect)
{
  std::string diag;
  for (const auto elem : vect)
  {
    diag += elem->writeObject();
  }
  std::cerr << type << ' ' << vect.size() << '\n' << diag << std::endl;
}
}// namespace
*/

log4cxx::LoggerPtr RBDByCabinDAO::_logger(
  log4cxx::Logger::getLogger("atseintl.DBAccess.RBDByCabinDAO"));

RBDByCabinDAO& RBDByCabinDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<RBDByCabinInfo*>&
getRBDByCabinData(const VendorCode& vendor,
                  const CarrierCode& carrier,
                  DateTime tvlDate,
                  DeleteList& deleteList,
                  DateTime ticketDate,
                  bool isHistorical)
{
  if (isHistorical)
  {
    RBDByCabinHistoricalDAO& dao(RBDByCabinHistoricalDAO::instance());
    const std::vector<RBDByCabinInfo*>& ret(dao.get(deleteList, vendor, carrier, tvlDate, ticketDate));
    return ret;
  }
  else
  {
    RBDByCabinDAO& dao(RBDByCabinDAO::instance());
    const std::vector<RBDByCabinInfo*>& ret(dao.get(deleteList, vendor, carrier, tvlDate, ticketDate));
    return ret;
  }
}

const std::vector<RBDByCabinInfo*>&
RBDByCabinDAO::get(DeleteList& del,
                   const VendorCode& vendor,
                   const CarrierCode& carrier,
                   DateTime tvlDate,
                   DateTime ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  RBDByCabinKey key(vendor, carrier);
  DAOCache::pointer_type ptr(cache().get(key));
  IsNotEffectiveG<RBDByCabinInfo> isNotEffective(tvlDate, ticketDate);
  return *(applyFilter(del, ptr, isNotEffective));
}

void RBDByCabinDAO::load()
{
  StartupLoader<QueryGetAllRBDByCabin, RBDByCabinInfo, RBDByCabinDAO>();
}

std::vector<RBDByCabinInfo*>*
RBDByCabinDAO::create(RBDByCabinKey key)
{
  std::vector<RBDByCabinInfo*>* ret(new std::vector<RBDByCabinInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get(this->cacheClass()));
  try
  {
    QueryGetRBDByCabin q(dbAdapter->getAdapter());
    q.findRBDByCabin(*ret, key._a, key._b);
    //displayQueryResults("Non historical", *ret);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in RBDByCabinDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

RBDByCabinKey
RBDByCabinDAO::createKey(const RBDByCabinInfo* info)
{
  return RBDByCabinKey(info->vendor(), info->carrier());
}

void
RBDByCabinDAO::destroy(RBDByCabinKey,
                       std::vector<RBDByCabinInfo*>* recs)
{
  destroyContainer(recs);
}

std::string RBDByCabinDAO::_name("RBDByCabin");
std::string RBDByCabinDAO::_cacheClass("Rules");
DAOHelper<RBDByCabinDAO> RBDByCabinDAO::_helper(_name);
RBDByCabinDAO* RBDByCabinDAO::_instance(nullptr);

// Historical DAO Object
log4cxx::LoggerPtr RBDByCabinHistoricalDAO::_logger(
  log4cxx::Logger::getLogger("atseintl.DBAccess.RBDByCabinHistoricalDAO"));

RBDByCabinHistoricalDAO& RBDByCabinHistoricalDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<RBDByCabinInfo*>&
RBDByCabinHistoricalDAO::get(DeleteList& del,
                             const VendorCode& vendor,
                             const CarrierCode& carrier,
                             DateTime tvlDate,
                             DateTime ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  RBDByCabinHistoricalKey key(vendor, carrier);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr(cache().get(key));
  del.copy(ptr);
  std::vector<RBDByCabinInfo*>* ret(new std::vector<RBDByCabinInfo*>);
  del.adopt(ret);
  IsNotEffectiveH<RBDByCabinInfo> isNotEffective(tvlDate, ticketDate);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), isNotEffective);
  return *ret;
}

std::vector<RBDByCabinInfo*>*
RBDByCabinHistoricalDAO::create(RBDByCabinHistoricalKey key)
{
  std::vector<RBDByCabinInfo*>* ret(new std::vector<RBDByCabinInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get("Historical", true));
  try
  {
    QueryGetRBDByCabinHistorical q(dbAdapter->getAdapter());
    q.findRBDByCabin(*ret, key._a, key._b, key._c, key._d);
    /*
    //displayQueryResults("Historical", *ret);
    std::vector<RBDByCabinInfo*> *all(new std::vector<RBDByCabinInfo*>);
    QueryGetAllRBDByCabin qall(dbAdapter->getAdapter());
    qall.findAllRBDByCabin(*all);    
    displayQueryResults("All", *all);
    */
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in RBDByCabinHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

RBDByCabinHistoricalKey
RBDByCabinHistoricalDAO::createKey(const RBDByCabinInfo* info,
                                   const DateTime& startDate,
                                   const DateTime& endDate)
{
  return RBDByCabinHistoricalKey(info->vendor(), info->carrier(), startDate, endDate);
}

void
RBDByCabinHistoricalDAO::destroy(RBDByCabinHistoricalKey,
                                 std::vector<RBDByCabinInfo*>* recs)
{
  destroyContainer(recs);
}

std::string RBDByCabinHistoricalDAO::_name("RBDByCabinHistorical");
std::string RBDByCabinHistoricalDAO::_cacheClass("Rules");
DAOHelper<RBDByCabinHistoricalDAO> RBDByCabinHistoricalDAO::_helper(_name);
RBDByCabinHistoricalDAO* RBDByCabinHistoricalDAO::_instance(nullptr);

} // namespace tse
