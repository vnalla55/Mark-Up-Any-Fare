//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/FltTrkCntryGrpDAO.h"

#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FltTrkCntryGrp.h"
#include "DBAccess/Queries/QueryGetFltTrk.h"

namespace tse
{
log4cxx::LoggerPtr
FltTrkCntryGrpDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FltTrkCntryGrpDAO"));

FltTrkCntryGrpDAO&
FltTrkCntryGrpDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const FltTrkCntryGrp*
getFltTrkCntryGrpData(const CarrierCode& carrier,
                      const DateTime& date,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    FltTrkCntryGrpHistoricalDAO& dao = FltTrkCntryGrpHistoricalDAO::instance();
    return dao.get(deleteList, carrier, date, ticketDate);
  }
  else
  {
    FltTrkCntryGrpDAO& dao = FltTrkCntryGrpDAO::instance();
    return dao.get(deleteList, carrier, date, ticketDate);
  }
}

const FltTrkCntryGrp*
FltTrkCntryGrpDAO::get(DeleteList& del,
                       const CarrierCode& key,
                       const DateTime& date,
                       const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(CarrierKey(key));
  del.copy(ptr);
  FltTrkCntryGrp* ret = nullptr;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveG<FltTrkCntryGrp>(date, ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

CarrierKey
FltTrkCntryGrpDAO::createKey(FltTrkCntryGrp* info)
{
  return CarrierKey(info->carrier());
}

std::vector<FltTrkCntryGrp*>*
FltTrkCntryGrpDAO::create(CarrierKey key)
{
  std::vector<FltTrkCntryGrp*>* ret = new std::vector<FltTrkCntryGrp*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFltTrk ft(dbAdapter->getAdapter());
    ft.findFltTrkCntryGrp(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FltTrkCntryGrpDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FltTrkCntryGrpDAO::destroy(CarrierKey key, std::vector<FltTrkCntryGrp*>* recs)
{
  destroyContainer(recs);
}

void
FltTrkCntryGrpDAO::load()
{
  StartupLoader<QueryGetAllFltTrk, FltTrkCntryGrp, FltTrkCntryGrpDAO>();
}

std::string
FltTrkCntryGrpDAO::_name("FltTrkCntryGrp");
std::string
FltTrkCntryGrpDAO::_cacheClass("Rules");

DAOHelper<FltTrkCntryGrpDAO>
FltTrkCntryGrpDAO::_helper(_name);

FltTrkCntryGrpDAO* FltTrkCntryGrpDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: FltTrkCntryGrpHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
FltTrkCntryGrpHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FltTrkCntryGrpHistoricalDAO"));
FltTrkCntryGrpHistoricalDAO&
FltTrkCntryGrpHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const FltTrkCntryGrp*
FltTrkCntryGrpHistoricalDAO::get(DeleteList& del,
                                 const CarrierCode& key,
                                 const DateTime& date,
                                 const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  FltTrkCntryGrp* ret = nullptr;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveHist<FltTrkCntryGrp>(date, ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<FltTrkCntryGrp*>*
FltTrkCntryGrpHistoricalDAO::create(CarrierCode key)
{
  std::vector<FltTrkCntryGrp*>* ret = new std::vector<FltTrkCntryGrp*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFltTrkHistorical ft(dbAdapter->getAdapter());
    ft.findFltTrkCntryGrp(*ret, key);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FltTrkCntryGrpHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FltTrkCntryGrpHistoricalDAO::destroy(CarrierCode key, std::vector<FltTrkCntryGrp*>* recs)
{
  destroyContainer(recs);
}

struct FltTrkCntryGrpHistoricalDAO::groupByKey
{
public:
  CarrierCode prevKey;

  DAOCache& cache;

  groupByKey()
    : prevKey(INVALID_CARRIERCODE), cache(FltTrkCntryGrpHistoricalDAO::instance().cache()), ptr(nullptr)
  {
  }

  std::vector<FltTrkCntryGrp*>* ptr;

  void operator()(FltTrkCntryGrp* info)
  {
    if (info->carrier() != prevKey)
    {
      ptr = new std::vector<FltTrkCntryGrp*>;
      cache.put(info->carrier(), ptr);
      prevKey = info->carrier();
    }
    ptr->push_back(info);
  }
};

void
FltTrkCntryGrpHistoricalDAO::load()
{
  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<FltTrkCntryGrp*> vector;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllFltTrkHistorical ft(dbAdapter->getAdapter());
    ft.findAllFltTrkCntryGrp(vector);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FltTrkCntryGrpDAO::load");
    deleteVectorOfPointers(vector);
    throw;
  }

  std::for_each(vector.begin(), vector.end(), groupByKey());
}

std::string
FltTrkCntryGrpHistoricalDAO::_name("FltTrkCntryGrpHistorical");
std::string
FltTrkCntryGrpHistoricalDAO::_cacheClass("Rules");
DAOHelper<FltTrkCntryGrpHistoricalDAO>
FltTrkCntryGrpHistoricalDAO::_helper(_name);

FltTrkCntryGrpHistoricalDAO* FltTrkCntryGrpHistoricalDAO::_instance = nullptr;

} // namespace tse
