//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/MinFareFareTypeGrpDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/MinFareFareTypeGrp.h"
#include "DBAccess/Queries/QueryGetMinFareFareTypeGrp.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
MinFareFareTypeGrpDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.MinFareFareTypeGrpDAO"));

MinFareFareTypeGrpDAO&
MinFareFareTypeGrpDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const MinFareFareTypeGrp*
getMinFareFareTypeGrpData(const std::string& key,
                          const DateTime& date,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical)
{
  if (isHistorical)
  {
    MinFareFareTypeGrpHistoricalDAO& dao = MinFareFareTypeGrpHistoricalDAO::instance();
    return dao.get(deleteList, key, date, ticketDate);
  }
  else
  {
    MinFareFareTypeGrpDAO& dao = MinFareFareTypeGrpDAO::instance();
    return dao.get(deleteList, key, date, ticketDate);
  }
}

const MinFareFareTypeGrp*
MinFareFareTypeGrpDAO::get(DeleteList& del,
                           const std::string& key,
                           const DateTime& date,
                           const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(StringKey(key));
  del.copy(ptr);
  MinFareFareTypeGrp* ret = nullptr;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveG<MinFareFareTypeGrp>(date, ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

StringKey
MinFareFareTypeGrpDAO::createKey(MinFareFareTypeGrp* info)
{
  return StringKey(info->specialProcessName());
}

void
MinFareFareTypeGrpDAO::load()
{
  StartupLoader<QueryGetAllMinFareFareTypeGrp, MinFareFareTypeGrp, MinFareFareTypeGrpDAO>();
}

std::vector<MinFareFareTypeGrp*>*
MinFareFareTypeGrpDAO::create(StringKey key)
{
  std::vector<MinFareFareTypeGrp*>* ret = new std::vector<MinFareFareTypeGrp*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMinFareFareTypeGrp mfftg(dbAdapter->getAdapter());
    mfftg.findMinFareFareTypeGrp(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MinFareFareTypeGrpDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MinFareFareTypeGrpDAO::destroy(StringKey key, std::vector<MinFareFareTypeGrp*>* recs)
{
  destroyContainer(recs);
}

sfc::CompressedData*
MinFareFareTypeGrpDAO::compress(const std::vector<MinFareFareTypeGrp*>* vect) const
{
  return compressVector(vect);
}

std::vector<MinFareFareTypeGrp*>*
MinFareFareTypeGrpDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MinFareFareTypeGrp>(compressed);
}

std::string
MinFareFareTypeGrpDAO::_name("MinFareFareTypeGrp");
std::string
MinFareFareTypeGrpDAO::_cacheClass("MinFares");

DAOHelper<MinFareFareTypeGrpDAO>
MinFareFareTypeGrpDAO::_helper(_name);

MinFareFareTypeGrpDAO* MinFareFareTypeGrpDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: MinFareFareTypeGrpHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
MinFareFareTypeGrpHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.MinFareFareTypeGrpHistoricalDAO"));
MinFareFareTypeGrpHistoricalDAO&
MinFareFareTypeGrpHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const MinFareFareTypeGrp*
MinFareFareTypeGrpHistoricalDAO::get(DeleteList& del,
                                     const std::string& key,
                                     const DateTime& date,
                                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  MinFareFareTypeGrp* ret = nullptr;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveHist<MinFareFareTypeGrp>(date, ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<MinFareFareTypeGrp*>*
MinFareFareTypeGrpHistoricalDAO::create(std::string key)
{
  std::vector<MinFareFareTypeGrp*>* ret = new std::vector<MinFareFareTypeGrp*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMinFareFareTypeGrpHistorical mfftg(dbAdapter->getAdapter());
    mfftg.findMinFareFareTypeGrp(*ret, key);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MinFareFareTypeGrpHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MinFareFareTypeGrpHistoricalDAO::destroy(std::string key, std::vector<MinFareFareTypeGrp*>* recs)
{
  std::vector<MinFareFareTypeGrp*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

struct MinFareFareTypeGrpHistoricalDAO::groupByKey
{
public:
  std::string prevKey;

  DAOCache& cache;

  groupByKey() : cache(MinFareFareTypeGrpHistoricalDAO::instance().cache()), ptr(nullptr) {}

  std::vector<MinFareFareTypeGrp*>* ptr;

  void operator()(MinFareFareTypeGrp* info)
  {
    if (info->specialProcessName() != prevKey)
    {
      ptr = new std::vector<MinFareFareTypeGrp*>;
      cache.put(info->specialProcessName(), ptr);
      prevKey = info->specialProcessName();
    }
    ptr->push_back(info);
  }
};

void
MinFareFareTypeGrpHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<MinFareFareTypeGrp*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllMinFareFareTypeGrpHistorical mfftg(dbAdapter->getAdapter());
    mfftg.findAllMinFareFareTypeGrp(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MinFareFareTypeGrpHistoricalDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::for_each(recs.begin(), recs.end(), groupByKey());
}

sfc::CompressedData*
MinFareFareTypeGrpHistoricalDAO::compress(const std::vector<MinFareFareTypeGrp*>* vect) const
{
  return compressVector(vect);
}

std::vector<MinFareFareTypeGrp*>*
MinFareFareTypeGrpHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MinFareFareTypeGrp>(compressed);
}

std::string
MinFareFareTypeGrpHistoricalDAO::_name("MinFareFareTypeGrpHistorical");
std::string
MinFareFareTypeGrpHistoricalDAO::_cacheClass("MinFares");
DAOHelper<MinFareFareTypeGrpHistoricalDAO>
MinFareFareTypeGrpHistoricalDAO::_helper(_name);

MinFareFareTypeGrpHistoricalDAO* MinFareFareTypeGrpHistoricalDAO::_instance = nullptr;

} // namespace tse
