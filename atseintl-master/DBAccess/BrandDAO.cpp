//------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
#include "DBAccess/BrandDAO.h"

#include "Common/Logger.h"
#include "DBAccess/Brand.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetBrands.h"

namespace tse
{
log4cxx::LoggerPtr
BrandDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.BrandDAO"));

BrandDAO&
BrandDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<Brand*>&
getBrandsData(const Indicator& userApplType,
              const UserApplCode& userAppl,
              const CarrierCode& carrier,
              const DateTime& travelDate,
              DeleteList& deleteList,
              const DateTime& ticketDate,
              bool isHistorical)
{
  if (isHistorical)
  {
    BrandHistoricalDAO& dao = BrandHistoricalDAO::instance();
    return dao.get(deleteList, userApplType, userAppl, carrier, travelDate, ticketDate);
  }
  else
  {
    BrandDAO& dao = BrandDAO::instance();
    return dao.get(deleteList, userApplType, userAppl, carrier, travelDate, ticketDate);
  }
}

const std::vector<Brand*>&
BrandDAO::get(DeleteList& del,
              const Indicator& userApplType,
              const UserApplCode& userAppl,
              const CarrierCode& carrier,
              const DateTime& travelDate,
              const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  BrandKey key(userApplType, userAppl, carrier);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<Brand*>* ret = new std::vector<Brand*>;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveG<Brand>(travelDate, ticketDate));
  while (i != ptr->end())
  {
    ret->push_back(*i);
    i = find_if(++i, ptr->end(), IsEffectiveG<Brand>(travelDate, ticketDate));
  }
  del.adopt(ret);
  return *ret;
}

BrandKey
BrandDAO::createKey(Brand* info)
{
  return BrandKey(info->userApplType(), info->userAppl(), info->carrier());
}

std::vector<Brand*>*
BrandDAO::create(BrandKey key)
{
  std::vector<Brand*>* ret = new std::vector<Brand*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetBrands ds(dbAdapter->getAdapter());
    ds.findBrands(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in BrandDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
BrandDAO::destroy(BrandKey key, std::vector<Brand*>* recs)
{
  destroyContainer(recs);
}

void
BrandDAO::load()
{
  // Pre-load this table only if Brand Grouping is enabled
  std::string configVal("N");
  (Global::config()).getValue("BRAND_GROUPING", configVal, "FAREDISPLAY_SVC");

  if (configVal != "Y" && configVal != "y" && configVal != "1")
    return;

  StartupLoader<QueryGetAllBrands, Brand, BrandDAO>();
}

std::string
BrandDAO::_name("Brand");
std::string
BrandDAO::_cacheClass("FareDisplay");

DAOHelper<BrandDAO>
BrandDAO::_helper(_name);

BrandDAO* BrandDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: BrandHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
BrandHistoricalDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.BrandHistoricalDAO"));
BrandHistoricalDAO&
BrandHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<Brand*>&
BrandHistoricalDAO::get(DeleteList& del,
                        const Indicator& userApplType,
                        const UserApplCode& userAppl,
                        const CarrierCode& carrier,
                        const DateTime& travelDate,
                        const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  BrandKey key(userApplType, userAppl, carrier);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<Brand*>* ret = new std::vector<Brand*>;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveHist<Brand>(travelDate, ticketDate));
  while (i != ptr->end())
  {
    ret->push_back(*i);
    i = find_if(++i, ptr->end(), IsEffectiveHist<Brand>(travelDate, ticketDate));
  }
  del.adopt(ret);
  return *ret;
}

std::vector<Brand*>*
BrandHistoricalDAO::create(BrandKey key)
{
  std::vector<Brand*>* ret = new std::vector<Brand*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetBrandsHistorical ds(dbAdapter->getAdapter());
    ds.findBrands(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in BrandHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
BrandHistoricalDAO::destroy(BrandKey key, std::vector<Brand*>* recs)
{
  destroyContainer(recs);
}

struct BrandHistoricalDAO::groupByKey
{
public:
  BrandKey prevKey;
  DAOCache& cache;

  groupByKey() : cache(BrandHistoricalDAO::instance().cache()), ptr(nullptr) {}

  std::vector<Brand*>* ptr;

  void operator()(Brand* info)
  {
    BrandKey key(info->userApplType(), info->userAppl(), info->carrier());
    if (!(key == prevKey))
    {
      ptr = new std::vector<Brand*>;
      cache.put(key, ptr);
      prevKey = key;
    }
    ptr->push_back(info);
  }
};

void
BrandHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  // Pre-load this table only if Brand Grouping is enabled
  std::string configVal("N");
  (Global::config()).getValue("BRAND_GROUPING", configVal, "FAREDISPLAY_SVC");

  if (configVal != "Y" && configVal != "y" && configVal != "1")
    return;

  std::vector<Brand*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllBrandsHistorical ds(dbAdapter->getAdapter());
    ds.findAllBrands(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in BrandHistoricalDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }
  std::for_each(recs.begin(), recs.end(), groupByKey());
}

std::string
BrandHistoricalDAO::_name("BrandHistorical");
std::string
BrandHistoricalDAO::_cacheClass("FareDisplay");
DAOHelper<BrandHistoricalDAO>
BrandHistoricalDAO::_helper(_name);

BrandHistoricalDAO* BrandHistoricalDAO::_instance = nullptr;

} // namespace tse
