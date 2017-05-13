//------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly
// prohibited.
//------------------------------------------------------------------------------
//

#include "DBAccess/MarriedCabinDAO.h"

#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/MarriedCabin.h"
#include "DBAccess/Queries/QueryGetMarriedCabin.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
MarriedCabinDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.MarriedCabinDAO"));

MarriedCabinDAO&
MarriedCabinDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<MarriedCabin*>&
getMarriedCabinsData(const CarrierCode& carrier,
                     const BookingCode& premiumCabin,
                     const DateTime& versionDate,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical)
{
  if (isHistorical)
  {
    MarriedCabinHistoricalDAO& dao = MarriedCabinHistoricalDAO::instance();
    return dao.get(deleteList, carrier, premiumCabin, versionDate, ticketDate);
  }
  else
  {
    MarriedCabinDAO& dao = MarriedCabinDAO::instance();
    return dao.get(deleteList, carrier, premiumCabin, versionDate, ticketDate);
  }
}

const std::vector<MarriedCabin*>&
MarriedCabinDAO::get(DeleteList& del,
                     const CarrierCode& carrier,
                     const BookingCode& premiumCabin,
                     const DateTime& versionDate,
                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  MarriedCabinKey key(carrier, premiumCabin);
  DAOCache::pointer_type ptr = cache().get(key);
  /*
  del.copy(ptr);

  std::vector<MarriedCabin*>* ret = new std::vector<MarriedCabin*>;
  del.adopt(ret);

  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret),
                 IsNotCurrentG<MarriedCabin>(ticketDate));


  return *ret;
  */
  return *(applyFilter(del, ptr, IsNotCurrentG<MarriedCabin>(ticketDate)));
}

std::vector<MarriedCabin*>*
MarriedCabinDAO::create(MarriedCabinKey key)
{
  std::vector<MarriedCabin*>* ret = new std::vector<MarriedCabin*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMarriedCabin mc(dbAdapter->getAdapter());
    mc.findMarriedCabins(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MarriedCabinDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MarriedCabinDAO::destroy(MarriedCabinKey key, std::vector<MarriedCabin*>* recs)
{
  for (auto& rec : *recs)
  {
    delete rec;
  }
  delete recs;
}

std::string
MarriedCabinDAO::_name("MarriedCabin");

MarriedCabinDAO* MarriedCabinDAO::_instance = nullptr;

DAOHelper<MarriedCabinDAO>
MarriedCabinDAO::_helper(MarriedCabinDAO::_name);

MarriedCabinKey
MarriedCabinDAO::createKey(MarriedCabin* info)
{
  return MarriedCabinKey(info->carrier(), info->premiumCabin());
}

void
MarriedCabinDAO::load()
{
  StartupLoaderNoDB<MarriedCabin, MarriedCabinDAO>();
}

// --------------------------------------------------
// Historical DAO: MarriedCabinHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
MarriedCabinHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.MarriedCabinHistoricalDAO"));
MarriedCabinHistoricalDAO&
MarriedCabinHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<MarriedCabin*>&
MarriedCabinHistoricalDAO::get(DeleteList& del,
                               const CarrierCode& carrier,
                               const BookingCode& premiumCabin,
                               const DateTime& versionDate,
                               const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  MarriedCabinKey key(carrier, premiumCabin);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<MarriedCabin*>* ret = new std::vector<MarriedCabin*>;
  del.adopt(ret);

  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotCurrentH<MarriedCabin>(ticketDate));

  return *ret;
}

std::vector<MarriedCabin*>*
MarriedCabinHistoricalDAO::create(MarriedCabinKey key)
{
  std::vector<MarriedCabin*>* ret = new std::vector<MarriedCabin*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMarriedCabinHistorical mc(dbAdapter->getAdapter());
    mc.findMarriedCabins(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MarriedCabinHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MarriedCabinHistoricalDAO::destroy(MarriedCabinKey key, std::vector<MarriedCabin*>* recs)
{
  for (auto& rec : *recs)
  {
    delete rec;
  }
  delete recs;
}

struct MarriedCabinHistoricalDAO::groupByKey
{
public:
  MarriedCabinKey prevKey;

  DAOCache& cache;

  groupByKey()
    : prevKey(INVALID_CARRIERCODE, ""), cache(MarriedCabinHistoricalDAO::instance().cache()), ptr(nullptr)
  {
  }

  std::vector<MarriedCabin*>* ptr;

  void operator()(MarriedCabin* info)
  {
    MarriedCabinKey key(info->carrier(), info->premiumCabin());
    if (!(key == prevKey))
    {
      ptr = new std::vector<MarriedCabin*>;
      cache.put(key, ptr);
      prevKey = key;
    }
    ptr->push_back(info);
  }
};

void
MarriedCabinHistoricalDAO::load()
{
  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<MarriedCabin*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllMarriedCabinHistorical sj(dbAdapter->getAdapter());
    sj.findAllMarriedCabins(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MarriedCabinHistoricalDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::for_each(recs.begin(), recs.end(), groupByKey());
}

std::string
MarriedCabinHistoricalDAO::_name("MarriedCabinHistorical");

MarriedCabinHistoricalDAO* MarriedCabinHistoricalDAO::_instance = nullptr;

DAOHelper<MarriedCabinHistoricalDAO>
MarriedCabinHistoricalDAO::_helper(MarriedCabinHistoricalDAO::_name);
} // namespace tse
