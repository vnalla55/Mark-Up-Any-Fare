#include "DBAccess/CommissionProgramDAO.h"

#include "Common/Logger.h"
#include "DBAccess/CommissionProgramInfo.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/Queries/QueryGetCommissionLocation.h"
#include "DBAccess/Queries/QueryGetCommissionMarket.h"
#include "DBAccess/Queries/QueryGetCommissionProgram.h"
#include "DBAccess/Queries/QueryGetCommissionTvlDate.h"

namespace tse
{
namespace
{
void populateLocations(DBAdapterPool::pointer_type dbAdapter,
                       std::vector<CommissionLocSegInfo*>& result,
                       const VendorCode& vendor,
                       int64_t itemNo)
{
  if (itemNo > -1)
  {
    QueryGetCommissionLocation q(dbAdapter->getAdapter());
    q.findCommissionLocation(result, vendor, itemNo);
  }
}

void populateLocationsHistorical(DBAdapterPool::pointer_type dbAdapter,
                                 std::vector<CommissionLocSegInfo*>& result,
                                 const VendorCode& vendor,
                                 int64_t itemNo,
                                 DateTime startDate,
                                 DateTime endDate)
{
  if (itemNo > -1)
  {
    QueryGetCommissionLocationHistorical q(dbAdapter->getAdapter());
    q.findCommissionLocation(result, vendor, itemNo, startDate, endDate);
  }
}

void populateMarkets(DBAdapterPool::pointer_type dbAdapter,
                     std::vector<CommissionMarketSegInfo*>& result,
                     const VendorCode& vendor,
                     int64_t itemNo)
{
  if (itemNo > -1)
  {
    QueryGetCommissionMarket q(dbAdapter->getAdapter());
    q.findCommissionMarket(result, vendor, itemNo);
  }
}

void populateMarketsHistorical(DBAdapterPool::pointer_type dbAdapter,
                               std::vector<CommissionMarketSegInfo*>& result,
                               const VendorCode& vendor,
                               int64_t itemNo,
                               DateTime startDate,
                               DateTime endDate)
{
  if (itemNo > -1)
  {
    QueryGetCommissionMarketHistorical q(dbAdapter->getAdapter());
    q.findCommissionMarket(result, vendor, itemNo, startDate, endDate);
  }
}

void populateTvlDates(DBAdapterPool::pointer_type dbAdapter,
                      std::vector<CommissionTravelDatesSegInfo*>& result,
                      const VendorCode& vendor,
                      int64_t itemNo)
{
  if (itemNo > -1)
  {
    QueryGetCommissionTvlDate q(dbAdapter->getAdapter());
    q.findCommissionTvlDate(result, vendor, itemNo);
  }
}

void populateTvlDatesHistorical(DBAdapterPool::pointer_type dbAdapter,
                                std::vector<CommissionTravelDatesSegInfo*>& result,
                                const VendorCode& vendor,
                                int64_t itemNo,
                                DateTime startDate,
                                DateTime endDate)
{
  if (itemNo > -1)
  {
    QueryGetCommissionTvlDateHistorical q(dbAdapter->getAdapter());
    q.findCommissionTvlDate(result, vendor, itemNo, startDate, endDate);
  }
}

}// namespace

log4cxx::LoggerPtr CommissionProgramDAO::_logger(
  log4cxx::Logger::getLogger("atseintl.DBAccess.CommissionProgramDAO"));

CommissionProgramDAO& CommissionProgramDAO::instance()
{
  if (0 == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<CommissionProgramInfo*>& getCommissionProgramData(const VendorCode& vendor,
                                                                    uint64_t contractId,
                                                                    DeleteList& deleteList,
                                                                    DateTime ticketDate,
                                                                    bool isHistorical)
{
  if (isHistorical)
  {
    CommissionProgramHistoricalDAO& dao(CommissionProgramHistoricalDAO::instance());

    return dao.get(deleteList, vendor, contractId, ticketDate);
  }
  else
  {
    CommissionProgramDAO& dao(CommissionProgramDAO::instance());

    return dao.get(deleteList, vendor, contractId, ticketDate);
  }
}

const std::vector<CommissionProgramInfo*>& CommissionProgramDAO::get(DeleteList& del,
                                                                     const VendorCode& vendor,
                                                                     uint64_t contractId,
                                                                     DateTime ticketDate)
{
  ++_codeCoverageGetCallCount;
  CommissionProgramKey key(vendor, contractId);
  DAOCache::pointer_type ptr(cache().get(key));
  IsNotEffectiveT<CommissionProgramInfo> isNotEffective(ticketDate);
  return *applyFilter(del, ptr, isNotEffective);
}

std::vector<CommissionProgramInfo*>* CommissionProgramDAO::create(CommissionProgramKey key)
{
  std::vector<CommissionProgramInfo*>* ret(new std::vector<CommissionProgramInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get(this->cacheClass()));
  try
  {
    QueryGetCommissionProgram q(dbAdapter->getAdapter());
    q.findCommissionProgram(*ret, key._a, key._b);
    for (auto info : *ret)
    {
      populateLocations(dbAdapter, info->pointOfSale(), info->vendor(), info->pointOfSaleItemNo());
      populateLocations(dbAdapter, info->pointOfOrigin(), info->vendor(), info->pointOfOriginItemNo());
      populateMarkets(dbAdapter, info->markets(), info->vendor(), info->marketItemNo());
      populateTvlDates(dbAdapter, info->travelDates(), info->vendor(), info->travelDatesItemNo());
    }
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CommissionProgramDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

CommissionProgramKey CommissionProgramDAO::createKey(const CommissionProgramInfo* info)
{
  return CommissionProgramKey(info->vendor(), info->contractId());
}

void CommissionProgramDAO::destroy(CommissionProgramKey,
                                   std::vector<CommissionProgramInfo*>* recs)
{
  destroyContainer(recs);
}

std::string CommissionProgramDAO::_name("CommissionProgram");
std::string CommissionProgramDAO::_cacheClass("Rules");
DAOHelper<CommissionProgramDAO> CommissionProgramDAO::_helper(_name);
CommissionProgramDAO* CommissionProgramDAO::_instance(0);

// Historical DAO Object
log4cxx::LoggerPtr CommissionProgramHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.CommissionProgramHistoricalDAO"));

CommissionProgramHistoricalDAO& CommissionProgramHistoricalDAO::instance()
{
  if (0 == _instance)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<CommissionProgramInfo*>& CommissionProgramHistoricalDAO::get(DeleteList& del,
                                                                               const VendorCode& vendor,
                                                                               uint64_t contractId,
                                                                               DateTime ticketDate)
{
  ++_codeCoverageGetCallCount;

  CommissionProgramHistoricalKey key(vendor, contractId);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr(cache().get(key));
  del.copy(ptr);
  std::vector<CommissionProgramInfo*>* ret(new std::vector<CommissionProgramInfo*>);
  del.adopt(ret);
  IsNotEffectiveH<CommissionProgramInfo> isNotEffective(ticketDate, ticketDate);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), isNotEffective);
  return *ret;
}

std::vector<CommissionProgramInfo*>* CommissionProgramHistoricalDAO::create(CommissionProgramHistoricalKey key)
{
  std::vector<CommissionProgramInfo*>* ret(new std::vector<CommissionProgramInfo*>);

  DBAdapterPool& dbAdapterPool(DBAdapterPool::instance());
  DBAdapterPool::pointer_type dbAdapter(dbAdapterPool.get("Historical", true));
  try
  {
    QueryGetCommissionProgramHistorical q(dbAdapter->getAdapter());
    q.findCommissionProgram(*ret, key._a, key._b, key._c, key._d);
    for (auto info : *ret)
    {
      populateLocationsHistorical(dbAdapter,
                                  info->pointOfSale(),
                                  info->vendor(),
                                  info->pointOfSaleItemNo(),
                                  key._c,
                                  key._d);
      populateLocationsHistorical(dbAdapter,
                                  info->pointOfOrigin(),
                                  info->vendor(),
                                  info->pointOfOriginItemNo(),
                                  key._c,
                                  key._d);    
      populateMarketsHistorical(dbAdapter,
                                info->markets(),
                                info->vendor(),
                                info->marketItemNo(),
                                key._c,
                                key._d);    
      populateTvlDatesHistorical(dbAdapter,
                                 info->travelDates(),
                                 info->vendor(),
                                 info->travelDatesItemNo(),
                                 key._c,
                                 key._d);
    }
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CommissionProgramHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

CommissionProgramHistoricalKey CommissionProgramHistoricalDAO::createKey(const CommissionProgramInfo* info,
                                                                         const DateTime& startDate,
                                                                         const DateTime& endDate)
{
  return CommissionProgramHistoricalKey(info->vendor(), info->contractId(), startDate, endDate);
}

void CommissionProgramHistoricalDAO::destroy(CommissionProgramHistoricalKey,
                                             std::vector<CommissionProgramInfo*>* recs)
{
  destroyContainer(recs);
}

std::string CommissionProgramHistoricalDAO::_name("CommissionProgramHistorical");
std::string CommissionProgramHistoricalDAO::_cacheClass("Rules");
DAOHelper<CommissionProgramHistoricalDAO> CommissionProgramHistoricalDAO::_helper(_name);
CommissionProgramHistoricalDAO* CommissionProgramHistoricalDAO::_instance(0);

}// tse
