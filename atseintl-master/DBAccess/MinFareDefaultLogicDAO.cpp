//-------------------------------------------------------------------------------
// Copyright 2004, 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/MinFareDefaultLogicDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/MinFareDefaultLogic.h"
#include "DBAccess/Queries/QueryGetMinFareDefaultLogic.h"

namespace tse
{
log4cxx::LoggerPtr
MinFareDefaultLogicDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.MinFareDefaultLogicDAO"));

MinFareDefaultLogicDAO&
MinFareDefaultLogicDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

struct MinFareDefaultLogicDAO::IsNotCurrentAndVendorMatchG
{
  const VendorCode _vendor;
  IsNotCurrentG<MinFareDefaultLogic> _isNotCurrent;

  IsNotCurrentAndVendorMatchG(const VendorCode& vendor, const DateTime& ticketDate)
    : _vendor(vendor), _isNotCurrent(ticketDate)
  {
  }

  bool operator()(const MinFareDefaultLogic* rec) const
  {
    return (rec->vendor() != _vendor || _isNotCurrent(rec));
  }
};

const std::vector<MinFareDefaultLogic*>&
getMinFareDefaultLogicData(const VendorCode& vendor,
                           const CarrierCode& carrier,
                           DeleteList& deleteList,
                           const DateTime& ticketDate,
                           bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    MinFareDefaultLogicHistoricalDAO& dao = MinFareDefaultLogicHistoricalDAO::instance();
    return dao.get(deleteList, vendor, carrier, ticketDate);
  }
  else
  {
    MinFareDefaultLogicDAO& dao = MinFareDefaultLogicDAO::instance();
    return dao.get(deleteList, vendor, carrier, ticketDate);
  }
}

const std::vector<MinFareDefaultLogic*>&
MinFareDefaultLogicDAO::get(DeleteList& del,
                            const VendorCode& vendor,
                            const CarrierCode& governingCarrier,
                            const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  std::vector<MinFareDefaultLogic*>* ret = new std::vector<MinFareDefaultLogic*>;
  del.adopt(ret);

  DAOCache::pointer_type ptr = cache().get(CarrierKey(governingCarrier));
  del.copy(ptr);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotCurrentAndVendorMatchG(vendor, ticketDate));
  if (LIKELY(!vendor.empty()))
  {
    remove_copy_if(
        ptr->begin(), ptr->end(), back_inserter(*ret), IsNotCurrentAndVendorMatchG("", ticketDate));
  }
  if (LIKELY(!governingCarrier.empty()))
  {
    ptr = cache().get(CarrierKey(""));
    del.copy(ptr);
    remove_copy_if(ptr->begin(),
                   ptr->end(),
                   back_inserter(*ret),
                   IsNotCurrentAndVendorMatchG(vendor, ticketDate));
    if (LIKELY(!vendor.empty()))
    {
      remove_copy_if(ptr->begin(),
                     ptr->end(),
                     back_inserter(*ret),
                     IsNotCurrentAndVendorMatchG("", ticketDate));
    }
  }
  return *ret;
}

CarrierKey
MinFareDefaultLogicDAO::createKey(MinFareDefaultLogic* info)
{
  return CarrierKey(info->governingCarrier());
}

std::vector<MinFareDefaultLogic*>*
MinFareDefaultLogicDAO::create(CarrierKey key)
{
  std::vector<MinFareDefaultLogic*>* ret = new std::vector<MinFareDefaultLogic*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMinFareDefaultLogic mfdl(dbAdapter->getAdapter());
    mfdl.findMinFareDefaultLogic(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MinFareDefaultLogicDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MinFareDefaultLogicDAO::load()
{
  StartupLoader<QueryGetAllMinFareDefaultLogic, MinFareDefaultLogic, MinFareDefaultLogicDAO>();
}

void
MinFareDefaultLogicDAO::destroy(CarrierKey key, std::vector<MinFareDefaultLogic*>* recs)
{
  destroyContainer(recs);
}

std::string
MinFareDefaultLogicDAO::_name("MinFareDefaultLogic");
std::string
MinFareDefaultLogicDAO::_cacheClass("MinFares");
DAOHelper<MinFareDefaultLogicDAO>
MinFareDefaultLogicDAO::_helper(_name);
MinFareDefaultLogicDAO* MinFareDefaultLogicDAO::_instance = nullptr;

/////////////////////////////////////////////////////
// Historical DAO Object
/////////////////////////////////////////////////////
log4cxx::LoggerPtr
MinFareDefaultLogicHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.MinFareDefaultLogicHistoricalDAO"));

MinFareDefaultLogicHistoricalDAO&
MinFareDefaultLogicHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<MinFareDefaultLogic*>&
MinFareDefaultLogicHistoricalDAO::get(DeleteList& del,
                                      const VendorCode& vendor,
                                      const CarrierCode& governingCarrier,
                                      const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  MinFareDefaultLogicHistoricalKey key(governingCarrier, vendor);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<MinFareDefaultLogic*>* ret = new std::vector<MinFareDefaultLogic*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotCurrentH<MinFareDefaultLogic>(ticketDate));
  return *ret;
}

std::vector<MinFareDefaultLogic*>*
MinFareDefaultLogicHistoricalDAO::create(MinFareDefaultLogicHistoricalKey key)
{
  std::vector<MinFareDefaultLogic*>* ret = new std::vector<MinFareDefaultLogic*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetMinFareDefaultLogicHistorical mfdl(dbAdapter->getAdapter());
    mfdl.findMinFareDefaultLogic(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MinFareDefaultLogicHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

struct MinFareDefaultLogicHistoricalDAO::groupByKey
{
public:
  MinFareDefaultLogicHistoricalKey prevKey;

  DAOCache& cache;

  groupByKey() : cache(MinFareDefaultLogicHistoricalDAO::instance().cache()), ptr(nullptr) {}

  std::vector<MinFareDefaultLogic*>* ptr;

  void operator()(MinFareDefaultLogic* info)
  {
    MinFareDefaultLogicHistoricalKey key(info->governingCarrier(), info->vendor());
    if (!(key == prevKey))
    {
      ptr = new std::vector<MinFareDefaultLogic*>;
      cache.put(key, ptr);
      prevKey = key;
    }
    ptr->push_back(info);
  }
};

void
MinFareDefaultLogicHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<MinFareDefaultLogic*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllMinFareDefaultLogicHistorical mfdl(dbAdapter->getAdapter());
    mfdl.findAllMinFareDefaultLogic(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in QueryGetAllMinFareDefaultLogicHistorical::load");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::for_each(recs.begin(), recs.end(), groupByKey());
}

void
MinFareDefaultLogicHistoricalDAO::destroy(MinFareDefaultLogicHistoricalKey key,
                                          std::vector<MinFareDefaultLogic*>* recs)
{
  std::vector<MinFareDefaultLogic*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
MinFareDefaultLogicHistoricalDAO::_name("MinFareDefaultLogicHistorical");
std::string
MinFareDefaultLogicHistoricalDAO::_cacheClass("MinFares");
DAOHelper<MinFareDefaultLogicHistoricalDAO>
MinFareDefaultLogicHistoricalDAO::_helper(_name);
MinFareDefaultLogicHistoricalDAO* MinFareDefaultLogicHistoricalDAO::_instance = nullptr;

} // namespace tse
