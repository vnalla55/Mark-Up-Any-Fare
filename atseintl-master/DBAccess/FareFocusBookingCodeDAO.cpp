#include "DBAccess/FareFocusBookingCodeDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareFocusBookingCodeInfo.h"
#include "DBAccess/Queries/QueryGetFareFocusBookingCode.h"

namespace tse
{
log4cxx::LoggerPtr
FareFocusBookingCodeDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareFocusBookingCodeDAO"));

FareFocusBookingCodeDAO&
FareFocusBookingCodeDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareFocusBookingCodeInfo*>&
getFareFocusBookingCodeData(uint64_t bookingCodeItemNo,
                            DateTime adjustedTicketDate,
                            DeleteList& deleteList,
                            DateTime ticketDate,
                            bool isHistorical)
{
  if (isHistorical)
  {
    FareFocusBookingCodeHistoricalDAO& dao(FareFocusBookingCodeHistoricalDAO::instance());

    const std::vector<FareFocusBookingCodeInfo*>& ret(dao.get(deleteList, bookingCodeItemNo, ticketDate));

    return ret;
  }
  else
  {
    FareFocusBookingCodeDAO& dao(FareFocusBookingCodeDAO::instance());

    const std::vector<FareFocusBookingCodeInfo*>& ret(dao.get(deleteList, bookingCodeItemNo, adjustedTicketDate, ticketDate));

    return ret;
  }
}

const std::vector<FareFocusBookingCodeInfo*>&
FareFocusBookingCodeDAO::get(DeleteList& del,
                             uint64_t bookingCodeItemNo,
                             DateTime adjustedTicketDate,
                             DateTime ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  FareFocusBookingCodeKey key(bookingCodeItemNo);
  DAOCache::pointer_type ptr(cache().get(key));
  IsNotCurrentG<FareFocusBookingCodeInfo> isNotCurrent(ticketDate);
  return *(applyFilter(del, ptr, isNotCurrent));
}

std::vector<FareFocusBookingCodeInfo*>*
FareFocusBookingCodeDAO::create(FareFocusBookingCodeKey key)
{
  std::vector<FareFocusBookingCodeInfo*>* ret(new std::vector<FareFocusBookingCodeInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get(this->cacheClass()));
  try
  {
    QueryGetFareFocusBookingCode q(dbAdapter->getAdapter());
    q.findFareFocusBookingCode(*ret, key._a);
    /*
    std::cerr << "  !! QueryGetFareFocusBookingCode: " << ' ' << ret->size() << " !!" << std::endl;
    std::vector<FareFocusBookingCodeInfo*> *all(new std::vector<FareFocusBookingCodeInfo*>);
    QueryGetAllFareFocusBookingCode qall(dbAdapter->getAdapter());
    qall.findAllFareFocusBookingCode(*all);
    std::cerr << "  !! QueryGetAllFareFocusBookingCode: " << ' ' << all->size() << " !!" << std::endl;
    */
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareFocusBookingCodeDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareFocusBookingCodeKey
FareFocusBookingCodeDAO::createKey(const FareFocusBookingCodeInfo* info)
{
  return FareFocusBookingCodeKey(info->bookingCodeItemNo());
}

void
FareFocusBookingCodeDAO::destroy(FareFocusBookingCodeKey, std::vector<FareFocusBookingCodeInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FareFocusBookingCodeDAO::_name("FareFocusBookingCode");
std::string
FareFocusBookingCodeDAO::_cacheClass("Rules");
DAOHelper<FareFocusBookingCodeDAO>
FareFocusBookingCodeDAO::_helper(_name);
FareFocusBookingCodeDAO*
FareFocusBookingCodeDAO::_instance(nullptr);

// Historical DAO Object
log4cxx::LoggerPtr
FareFocusBookingCodeHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareFocusBookingCodeHistoricalDAO"));

FareFocusBookingCodeHistoricalDAO&
FareFocusBookingCodeHistoricalDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareFocusBookingCodeInfo*>&
FareFocusBookingCodeHistoricalDAO::get(DeleteList& del,
                                       uint64_t bookingCodeItemNo,
                                       const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  FareFocusBookingCodeHistoricalKey key(bookingCodeItemNo);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr(cache().get(key));
  del.copy(ptr);
  std::vector<FareFocusBookingCodeInfo*>* ret(new std::vector<FareFocusBookingCodeInfo*>);
  del.adopt(ret);
  IsNotCurrentDH<FareFocusBookingCodeInfo> isNotCurrent(ticketDate);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), isNotCurrent);
  return *ret;
}

std::vector<FareFocusBookingCodeInfo*>*
FareFocusBookingCodeHistoricalDAO::create(FareFocusBookingCodeHistoricalKey key)
{
  std::vector<FareFocusBookingCodeInfo*>* ret(new std::vector<FareFocusBookingCodeInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get("Historical", true));
  try
  {
    QueryGetFareFocusBookingCodeHistorical q(dbAdapter->getAdapter());
    q.findFareFocusBookingCode(*ret, key._a, key._b, key._c);
    //std::cerr << "!!!! QueryGetFareFocusBookingCodeHistorical: " << ' ' << ret->size() << std::endl;
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareFocusBookingCodeHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareFocusBookingCodeHistoricalKey
FareFocusBookingCodeHistoricalDAO::createKey(const FareFocusBookingCodeInfo* info,
                                             const DateTime& startDate,
                                             const DateTime& endDate)
{
  return FareFocusBookingCodeHistoricalKey(info->bookingCodeItemNo(), startDate, endDate);
}

void
FareFocusBookingCodeHistoricalDAO::destroy(FareFocusBookingCodeHistoricalKey,
                                           std::vector<FareFocusBookingCodeInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FareFocusBookingCodeHistoricalDAO::_name("FareFocusBookingCodeHistorical");
std::string
FareFocusBookingCodeHistoricalDAO::_cacheClass("Rules");
DAOHelper<FareFocusBookingCodeHistoricalDAO>
FareFocusBookingCodeHistoricalDAO::_helper(_name);
FareFocusBookingCodeHistoricalDAO*
FareFocusBookingCodeHistoricalDAO::_instance(nullptr);

} // namespace tse
