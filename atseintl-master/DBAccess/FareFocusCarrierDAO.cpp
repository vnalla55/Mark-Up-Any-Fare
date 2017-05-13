#include "DBAccess/FareFocusCarrierDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareFocusCarrierInfo.h"
#include "DBAccess/Queries/QueryGetFareFocusCarrier.h"

namespace tse
{
log4cxx::LoggerPtr
FareFocusCarrierDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FareFocusCarrierDAO"));

FareFocusCarrierDAO&
FareFocusCarrierDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareFocusCarrierInfo*>&
getFareFocusCarrierData(uint64_t carrierItemNo,
                        DateTime adjustedTicketDate,
                        DeleteList& deleteList,
                        DateTime ticketDate,
                        bool isHistorical)
{
  if (isHistorical)
  {
    FareFocusCarrierHistoricalDAO& dao(FareFocusCarrierHistoricalDAO::instance());
    const std::vector<FareFocusCarrierInfo*>& ret(dao.get(deleteList, carrierItemNo, ticketDate));

    return ret;
  }
  else
  {
    FareFocusCarrierDAO& dao(FareFocusCarrierDAO::instance());
    const std::vector<FareFocusCarrierInfo*>& ret(dao.get(deleteList, carrierItemNo, adjustedTicketDate, ticketDate));

    return ret;
  }
}

const std::vector<FareFocusCarrierInfo*>&
FareFocusCarrierDAO::get(DeleteList& del,
                         uint64_t carrierItemNo,
                         DateTime adjustedTicketDate,
                         DateTime ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  FareFocusCarrierKey key(carrierItemNo);
  DAOCache::pointer_type ptr(cache().get(key));
  IsNotCurrentG<FareFocusCarrierInfo> isNotCurrent(ticketDate);
  return *(applyFilter(del, ptr, isNotCurrent));
}

std::vector<FareFocusCarrierInfo*>*
FareFocusCarrierDAO::create(FareFocusCarrierKey key)
{
  std::vector<FareFocusCarrierInfo*>* ret(new std::vector<FareFocusCarrierInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get(this->cacheClass()));
  try
  {
    QueryGetFareFocusCarrier q(dbAdapter->getAdapter());
    q.findFareFocusCarrier(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareFocusCarrierDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareFocusCarrierKey
FareFocusCarrierDAO::createKey(const FareFocusCarrierInfo* info)
{
  return FareFocusCarrierKey(info->carrierItemNo());
}

void
FareFocusCarrierDAO::destroy(FareFocusCarrierKey, std::vector<FareFocusCarrierInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FareFocusCarrierDAO::_name("FareFocusCarrier");
std::string
FareFocusCarrierDAO::_cacheClass("Rules");
DAOHelper<FareFocusCarrierDAO>
FareFocusCarrierDAO::_helper(_name);
FareFocusCarrierDAO*
FareFocusCarrierDAO::_instance(nullptr);

// Historical DAO Object
log4cxx::LoggerPtr
FareFocusCarrierHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareFocusCarrierHistoricalDAO"));

FareFocusCarrierHistoricalDAO&
FareFocusCarrierHistoricalDAO::instance()
{
  if (nullptr == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareFocusCarrierInfo*>&
FareFocusCarrierHistoricalDAO::get(DeleteList& del,
                                   uint64_t carrierItemNo,
                                   const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  FareFocusCarrierHistoricalKey key(carrierItemNo);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr(cache().get(key));
  del.copy(ptr);
  std::vector<FareFocusCarrierInfo*>* ret(new std::vector<FareFocusCarrierInfo*>);
  del.adopt(ret);
  IsNotCurrentDH<FareFocusCarrierInfo> isNotCurrent(ticketDate);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), isNotCurrent);
  return *ret;
}

std::vector<FareFocusCarrierInfo*>*
FareFocusCarrierHistoricalDAO::create(FareFocusCarrierHistoricalKey key)
{
  std::vector<FareFocusCarrierInfo*>* ret(new std::vector<FareFocusCarrierInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get("Historical", true));
  try
  {
    QueryGetFareFocusCarrierHistorical q(dbAdapter->getAdapter());
    q.findFareFocusCarrier(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareFocusCarrierHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

FareFocusCarrierHistoricalKey
FareFocusCarrierHistoricalDAO::createKey(const FareFocusCarrierInfo* info,
                                         const DateTime& startDate,
                                         const DateTime& endDate)
{
  return FareFocusCarrierHistoricalKey(info->carrierItemNo(), startDate, endDate);
}

void
FareFocusCarrierHistoricalDAO::destroy(FareFocusCarrierHistoricalKey,
                                       std::vector<FareFocusCarrierInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FareFocusCarrierHistoricalDAO::_name("FareFocusCarrierHistorical");
std::string
FareFocusCarrierHistoricalDAO::_cacheClass("Rules");
DAOHelper<FareFocusCarrierHistoricalDAO>
FareFocusCarrierHistoricalDAO::_helper(_name);
FareFocusCarrierHistoricalDAO*
FareFocusCarrierHistoricalDAO::_instance(nullptr);

} // namespace tse

