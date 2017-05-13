//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/SectorSurchargeDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetSectorSurcharge.h"
#include "DBAccess/SectorSurcharge.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
SectorSurchargeDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SectorSurchargeDAO"));

SectorSurchargeDAO&
SectorSurchargeDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<SectorSurcharge*>&
getSectorSurchargeData(const CarrierCode& key,
                       const DateTime& date,
                       DeleteList& deleteList,
                       const DateTime& ticketDate,
                       bool isHistorical)
{
  if (isHistorical)
  {
    SectorSurchargeHistoricalDAO& dao = SectorSurchargeHistoricalDAO::instance();
    return dao.get(deleteList, key, date, ticketDate);
  }
  else
  {
    SectorSurchargeDAO& dao = SectorSurchargeDAO::instance();
    return dao.get(deleteList, key, date, ticketDate);
  }
}

const std::vector<SectorSurcharge*>&
SectorSurchargeDAO::get(DeleteList& del,
                        const CarrierCode& key,
                        const DateTime& date,
                        const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  std::vector<SectorSurcharge*>* ret = new std::vector<SectorSurcharge*>;
  del.adopt(ret);
  DAOCache::pointer_type ptr = cache().get(CarrierKey(key));
  if (!ptr->empty())
  {
    del.copy(ptr);
    remove_copy_if(ptr->begin(),
                   ptr->end(),
                   back_inserter(*ret),
                   IsNotEffectiveG<SectorSurcharge>(date, ticketDate));
  }
  if (LIKELY(!key.empty()))
  {
    ptr = cache().get(CarrierKey(""));
    if (LIKELY(!ptr->empty()))
    {
      del.copy(ptr);
      remove_copy_if(ptr->begin(),
                     ptr->end(),
                     back_inserter(*ret),
                     IsNotEffectiveG<SectorSurcharge>(date, ticketDate));
    }
  }
  return *ret;
}

CarrierKey
SectorSurchargeDAO::createKey(SectorSurcharge* info)
{
  return CarrierKey(info->carrier());
}

std::vector<SectorSurcharge*>*
SectorSurchargeDAO::create(CarrierKey key)
{
  std::vector<SectorSurcharge*>* ret = new std::vector<SectorSurcharge*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSectorSurchargeBase ssb(dbAdapter->getAdapter());
    ssb.findSectorSurcharge(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SectorSurchargeDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SectorSurchargeDAO::destroy(CarrierKey key, std::vector<SectorSurcharge*>* recs)
{
  destroyContainer(recs);
}

void
SectorSurchargeDAO::load()
{
  StartupLoader<QueryGetAllSectorSurchargeBase, SectorSurcharge, SectorSurchargeDAO>();
}

std::string
SectorSurchargeDAO::_name("SectorSurcharge");
std::string
SectorSurchargeDAO::_cacheClass("BookingCode");

DAOHelper<SectorSurchargeDAO>
SectorSurchargeDAO::_helper(_name);

SectorSurchargeDAO* SectorSurchargeDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: SectorSurchargeHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
SectorSurchargeHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SectorSurchargeHistoricalDAO"));
SectorSurchargeHistoricalDAO&
SectorSurchargeHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<SectorSurcharge*>&
SectorSurchargeHistoricalDAO::get(DeleteList& del,
                                  const CarrierCode& key,
                                  const DateTime& date,
                                  const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  std::vector<SectorSurcharge*>* ret = new std::vector<SectorSurcharge*>;
  del.adopt(ret);
  DAOCache::pointer_type ptr = cache().get(key);
  if (!ptr->empty())
  {
    del.copy(ptr);
    remove_copy_if(ptr->begin(),
                   ptr->end(),
                   back_inserter(*ret),
                   IsNotEffectiveHist<SectorSurcharge>(date, ticketDate));
  }
  if (!key.empty())
  {
    ptr = cache().get("");
    if (!ptr->empty())
    {
      del.copy(ptr);
      remove_copy_if(ptr->begin(),
                     ptr->end(),
                     back_inserter(*ret),
                     IsNotEffectiveHist<SectorSurcharge>(date, ticketDate));
    }
  }
  return *ret;
}

struct SectorSurchargeHistoricalDAO::groupByKey
{
public:
  CarrierCode prevKey;

  DAOCache& cache;

  groupByKey() : cache(SectorSurchargeHistoricalDAO::instance().cache()), ptr(nullptr) {}

  std::vector<SectorSurcharge*>* ptr;

  void operator()(SectorSurcharge* info)
  {
    CarrierCode key(info->carrier());
    if (!(key == prevKey) || ptr == nullptr)
    {
      ptr = new std::vector<SectorSurcharge*>;
      cache.put(key, ptr);
      prevKey = key;
    }

    ptr->push_back(info);
  }
};

void
SectorSurchargeHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<SectorSurcharge*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllSectorSurchargeBaseHistorical ssb(dbAdapter->getAdapter());
    ssb.findAllSectorSurcharge(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SectorSurchargeHistoricalDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::for_each(recs.begin(), recs.end(), groupByKey());
}

std::vector<SectorSurcharge*>*
SectorSurchargeHistoricalDAO::create(CarrierCode key)
{
  std::vector<SectorSurcharge*>* ret = new std::vector<SectorSurcharge*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSectorSurchargeBaseHistorical ssb(dbAdapter->getAdapter());
    ssb.findSectorSurcharge(*ret, key);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SectorSurchargeHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SectorSurchargeHistoricalDAO::destroy(CarrierCode key, std::vector<SectorSurcharge*>* recs)
{
  destroyContainer(recs);
}

std::string
SectorSurchargeHistoricalDAO::_name("SectorSurchargeHistorical");
std::string
SectorSurchargeHistoricalDAO::_cacheClass("BookingCode");
DAOHelper<SectorSurchargeHistoricalDAO>
SectorSurchargeHistoricalDAO::_helper(_name);
SectorSurchargeHistoricalDAO* SectorSurchargeHistoricalDAO::_instance = nullptr;

} // namespace tse
