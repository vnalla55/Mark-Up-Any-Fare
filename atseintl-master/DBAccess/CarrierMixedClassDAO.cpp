//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/CarrierMixedClassDAO.h"

#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DBAccess/CarrierMixedClass.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetCarrierMixedClass.h"

namespace tse
{
log4cxx::LoggerPtr
CarrierMixedClassDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.CarrierMixedClassDAO"));

CarrierMixedClassDAO&
CarrierMixedClassDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<CarrierMixedClass*>&
getCarrierMixedClassData(const CarrierCode& carrier,
                         const DateTime& date,
                         DeleteList& deleteList,
                         const DateTime& ticketDate,
                         bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    CarrierMixedClassHistoricalDAO& dao = CarrierMixedClassHistoricalDAO::instance();
    return dao.get(deleteList, carrier, date, ticketDate);
  }
  else
  {
    CarrierMixedClassDAO& dao = CarrierMixedClassDAO::instance();
    return dao.get(deleteList, carrier, date, ticketDate);
  }
}

const std::vector<CarrierMixedClass*>&
CarrierMixedClassDAO::get(DeleteList& del,
                          const CarrierCode& key,
                          const DateTime& date,
                          const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(CarrierKey(key));
  /*
  del.copy(ptr);
  std::vector<CarrierMixedClass*>* ret = new std::vector<CarrierMixedClass*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret),
  IsNotEffectiveG<CarrierMixedClass>(date, ticketDate));
  return *ret;
  */
  return *(applyFilter(del, ptr, IsNotEffectiveG<CarrierMixedClass>(date, ticketDate)));
}

CarrierKey
CarrierMixedClassDAO::createKey(CarrierMixedClass* info)
{
  return CarrierKey(info->carrier());
}

std::vector<CarrierMixedClass*>*
CarrierMixedClassDAO::create(CarrierKey key)
{
  std::vector<CarrierMixedClass*>* ret = new std::vector<CarrierMixedClass*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCarrierMixedClass cmc(dbAdapter->getAdapter());
    cmc.findCarrierMixedClass(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CarrierMixedClassDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CarrierMixedClassDAO::destroy(CarrierKey key, std::vector<CarrierMixedClass*>* recs)
{
  destroyContainer(recs);
}

void
CarrierMixedClassDAO::load()
{
  StartupLoader<QueryGetAllCarrierMixedClass, CarrierMixedClass, CarrierMixedClassDAO>();
}

std::string
CarrierMixedClassDAO::_name("CarrierMixedClass");
std::string
CarrierMixedClassDAO::_cacheClass("BookingCode");

DAOHelper<CarrierMixedClassDAO>
CarrierMixedClassDAO::_helper(_name);

CarrierMixedClassDAO* CarrierMixedClassDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: CarrierMixedClassHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
CarrierMixedClassHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.CarrierMixedClassHistoricalDAO"));
CarrierMixedClassHistoricalDAO&
CarrierMixedClassHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<CarrierMixedClass*>&
CarrierMixedClassHistoricalDAO::get(DeleteList& del,
                                    const CarrierCode& key,
                                    const DateTime& date,
                                    const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<CarrierMixedClass*>* ret = new std::vector<CarrierMixedClass*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveHist<CarrierMixedClass>(date, ticketDate));
  return *ret;
}

std::vector<CarrierMixedClass*>*
CarrierMixedClassHistoricalDAO::create(CarrierCode key)
{
  std::vector<CarrierMixedClass*>* ret = new std::vector<CarrierMixedClass*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCarrierMixedClassHistorical cmc(dbAdapter->getAdapter());
    cmc.findCarrierMixedClass(*ret, key);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CarrierMixedClassHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CarrierMixedClassHistoricalDAO::destroy(CarrierCode key, std::vector<CarrierMixedClass*>* recs)
{
  destroyContainer(recs);
}

struct CarrierMixedClassHistoricalDAO::groupByKey
{
public:
  CarrierCode prevKey;

  DAOCache& cache;

  groupByKey()
    : prevKey(INVALID_CARRIERCODE),
      cache(CarrierMixedClassHistoricalDAO::instance().cache()),
      ptr(nullptr)
  {
  }

  std::vector<CarrierMixedClass*>* ptr;

  void operator()(CarrierMixedClass* info)
  {
    if (info->carrier() != prevKey)
    {
      ptr = new std::vector<CarrierMixedClass*>;
      cache.put(info->carrier(), ptr);
      prevKey = info->carrier();
    }
    ptr->push_back(info);
  }
};

void
CarrierMixedClassHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<CarrierMixedClass*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllCarrierMixedClassHistorical cmc(dbAdapter->getAdapter());
    cmc.findAllCarrierMixedClass(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CarrierMixedClassDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::for_each(recs.begin(), recs.end(), groupByKey());
}

std::string
CarrierMixedClassHistoricalDAO::_name("CarrierMixedClassHistorical");
std::string
CarrierMixedClassHistoricalDAO::_cacheClass("BookingCode");
DAOHelper<CarrierMixedClassHistoricalDAO>
CarrierMixedClassHistoricalDAO::_helper(_name);
CarrierMixedClassHistoricalDAO* CarrierMixedClassHistoricalDAO::_instance = nullptr;

} // namespace tse
