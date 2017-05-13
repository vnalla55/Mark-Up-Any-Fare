//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/CarrierPreferenceDAO.h"

#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetCarrierPref.h"

namespace tse
{
log4cxx::LoggerPtr
CarrierPreferenceDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.CarrierPreferenceDAO"));

CarrierPreferenceDAO&
CarrierPreferenceDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const CarrierPreference*
getCarrierPreferenceData(const CarrierCode& carrier,
                         const DateTime& date,
                         DeleteList& deleteList,
                         const DateTime& ticketDate,
                         bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    CarrierPreferenceHistoricalDAO& dao = CarrierPreferenceHistoricalDAO::instance();
    return dao.get(deleteList, carrier, date, ticketDate);
  }
  else
  {
    CarrierPreferenceDAO& dao = CarrierPreferenceDAO::instance();
    return dao.get(deleteList, carrier, date, ticketDate);
  }
}

const CarrierPreference*
CarrierPreferenceDAO::get(DeleteList& del,
                          const CarrierCode& key,
                          const DateTime& date,
                          const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  CarrierPreference* ret = nullptr;
  DAOCache::pointer_type ptr = cache().get(CarrierKey(key));
  DAOCache::value_type::iterator i =
      std::find_if(ptr->begin(), ptr->end(), IsEffectiveG<CarrierPreference>(date, ticketDate));
  if (i != ptr->end())
  {
    ret = *i;
  }
  else
  {
    ptr = cache().get(CarrierKey(""));
    i = std::find_if(ptr->begin(), ptr->end(), IsEffectiveG<CarrierPreference>(date, ticketDate));
    if (LIKELY(i != ptr->end()))
      ret = *i;
  }
  if (LIKELY(ret != nullptr))
    del.copy(ptr);
  return ret;
}

CarrierKey
CarrierPreferenceDAO::createKey(CarrierPreference* info)
{
  return CarrierKey(info->carrier());
}

std::vector<CarrierPreference*>*
CarrierPreferenceDAO::create(CarrierKey key)
{
  std::vector<CarrierPreference*>* ret = new std::vector<CarrierPreference*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCarrierPref cp(dbAdapter->getAdapter());
    cp.findCarrierPref(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CarrierPreferenceDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CarrierPreferenceDAO::destroy(CarrierKey key, std::vector<CarrierPreference*>* recs)
{
  destroyContainer(recs);
}

void
CarrierPreferenceDAO::load()
{
  StartupLoader<QueryGetAllCarrierPref, CarrierPreference, CarrierPreferenceDAO>();
}

std::string
CarrierPreferenceDAO::_name("CarrierPreference");
std::string
CarrierPreferenceDAO::_cacheClass("Common");

DAOHelper<CarrierPreferenceDAO>
CarrierPreferenceDAO::_helper(_name);

CarrierPreferenceDAO* CarrierPreferenceDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: CarrierPreferenceHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
CarrierPreferenceHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.CarrierPreferenceHistoricalDAO"));
CarrierPreferenceHistoricalDAO&
CarrierPreferenceHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const CarrierPreference*
CarrierPreferenceHistoricalDAO::get(DeleteList& del,
                                    const CarrierCode& key,
                                    const DateTime& date,
                                    const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  CarrierPreference* ret = nullptr;
  DAOCache::pointer_type ptr = cache().get(key);
  DAOCache::value_type::iterator i =
      std::find_if(ptr->begin(), ptr->end(), IsEffectiveHist<CarrierPreference>(date, ticketDate));
  if (i != ptr->end())
  {
    ret = *i;
  }
  else
  {
    ptr = cache().get("");
    i = std::find_if(
        ptr->begin(), ptr->end(), IsEffectiveHist<CarrierPreference>(date, ticketDate));
    if (i != ptr->end())
      ret = *i;
  }
  if (ret != nullptr)
    del.copy(ptr);
  return ret;
}

std::vector<CarrierPreference*>*
CarrierPreferenceHistoricalDAO::create(CarrierCode key)
{
  std::vector<CarrierPreference*>* ret = new std::vector<CarrierPreference*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCarrierPrefHistorical cp(dbAdapter->getAdapter());
    cp.findCarrierPref(*ret, key);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CarrierPreferenceHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CarrierPreferenceHistoricalDAO::destroy(CarrierCode key, std::vector<CarrierPreference*>* recs)
{
  destroyContainer(recs);
}

struct CarrierPreferenceHistoricalDAO::groupByKey
{
public:
  CarrierCode prevKey;
  DAOCache& cache;

  groupByKey()
    : prevKey(INVALID_CARRIERCODE),
      cache(CarrierPreferenceHistoricalDAO::instance().cache()),
      ptr(nullptr)
  {
  }

  std::vector<CarrierPreference*>* ptr;

  void operator()(CarrierPreference* info)
  {
    if (info->carrier() != prevKey)
    {
      ptr = new std::vector<CarrierPreference*>;
      cache.put(info->carrier(), ptr);
      prevKey = info->carrier();
    }
    ptr->push_back(info);
  }
};

void
CarrierPreferenceHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<CarrierPreference*> vector;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllCarrierPrefHistorical cp(dbAdapter->getAdapter());
    cp.findAllCarrierPref(vector);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CarrierPreferenceHistoricalDAO::load");
    deleteVectorOfPointers(vector);
    throw;
  }

  std::for_each(vector.begin(), vector.end(), groupByKey());
}

std::string
CarrierPreferenceHistoricalDAO::_name("CarrierPreferenceHistorical");
std::string
CarrierPreferenceHistoricalDAO::_cacheClass("Common");
DAOHelper<CarrierPreferenceHistoricalDAO>
CarrierPreferenceHistoricalDAO::_helper(_name);
CarrierPreferenceHistoricalDAO* CarrierPreferenceHistoricalDAO::_instance = nullptr;

} // namespace tse
