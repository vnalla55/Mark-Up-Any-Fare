//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/SurfaceSectorExemptDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetSurfaceSectorExempt.h"
#include "DBAccess/SurfaceSectorExempt.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
SurfaceSectorExemptDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SurfaceSectorExemptDAO"));

SurfaceSectorExemptDAO&
SurfaceSectorExemptDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const SurfaceSectorExempt*
getSurfaceSectorExemptData(const LocCode& origLoc,
                           const LocCode& destLoc,
                           const DateTime& date,
                           DeleteList& deleteList,
                           const DateTime& ticketDate,
                           bool isHistorical)
{
  if (isHistorical)
  {
    SurfaceSectorExemptHistoricalDAO& dao = SurfaceSectorExemptHistoricalDAO::instance();
    return dao.get(deleteList, origLoc, destLoc, date, ticketDate);
  }
  else
  {
    SurfaceSectorExemptDAO& dao = SurfaceSectorExemptDAO::instance();
    return dao.get(deleteList, origLoc, destLoc, date, ticketDate);
  }
}

const SurfaceSectorExempt*
SurfaceSectorExemptDAO::get(DeleteList& del,
                            const LocCode& origLoc,
                            const LocCode& destLoc,
                            const DateTime& date,
                            const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  SurfaceSectorExemptKey key(origLoc, destLoc);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  SurfaceSectorExempt* ret = nullptr;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveG<SurfaceSectorExempt>(date, ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

SurfaceSectorExemptKey
SurfaceSectorExemptDAO::createKey(SurfaceSectorExempt* info)
{
  return SurfaceSectorExemptKey(info->origLoc(), info->destLoc());
}

std::vector<SurfaceSectorExempt*>*
SurfaceSectorExemptDAO::create(SurfaceSectorExemptKey key)
{
  std::vector<SurfaceSectorExempt*>* ret = new std::vector<SurfaceSectorExempt*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSurfaceSectorExempt sse(dbAdapter->getAdapter());
    sse.findSurfaceSectorExempt(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SurfaceSectorExemptDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SurfaceSectorExemptDAO::load()
{
  StartupLoader<QueryGetAllSurfaceSectorExempt, SurfaceSectorExempt, SurfaceSectorExemptDAO>();
}

void
SurfaceSectorExemptDAO::destroy(SurfaceSectorExemptKey key, std::vector<SurfaceSectorExempt*>* recs)
{
  std::vector<SurfaceSectorExempt*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
SurfaceSectorExemptDAO::compress(const std::vector<SurfaceSectorExempt*>* vect) const
{
  return compressVector(vect);
}

std::vector<SurfaceSectorExempt*>*
SurfaceSectorExemptDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<SurfaceSectorExempt>(compressed);
}

std::string
SurfaceSectorExemptDAO::_name("SurfaceSectorExempt");
std::string
SurfaceSectorExemptDAO::_cacheClass("BookingCode");

DAOHelper<SurfaceSectorExemptDAO>
SurfaceSectorExemptDAO::_helper(_name);

SurfaceSectorExemptDAO* SurfaceSectorExemptDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: SurfaceSectorExemptHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
SurfaceSectorExemptHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SurfaceSectorExemptHistoricalDAO"));
SurfaceSectorExemptHistoricalDAO&
SurfaceSectorExemptHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const SurfaceSectorExempt*
SurfaceSectorExemptHistoricalDAO::get(DeleteList& del,
                                      const LocCode& origLoc,
                                      const LocCode& destLoc,
                                      const DateTime& date,
                                      const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  SurfaceSectorExemptKey key(origLoc, destLoc);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  SurfaceSectorExempt* ret = nullptr;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveHist<SurfaceSectorExempt>(date, ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<SurfaceSectorExempt*>*
SurfaceSectorExemptHistoricalDAO::create(SurfaceSectorExemptKey key)
{
  std::vector<SurfaceSectorExempt*>* ret = new std::vector<SurfaceSectorExempt*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSurfaceSectorExemptHistorical sse(dbAdapter->getAdapter());
    sse.findSurfaceSectorExempt(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SurfaceSectorExemptHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

struct SurfaceSectorExemptHistoricalDAO::groupByKey
{
public:
  SurfaceSectorExemptKey prevKey;

  DAOCache& cache;

  groupByKey() : cache(SurfaceSectorExemptHistoricalDAO::instance().cache()), ptr(nullptr) {}

  std::vector<SurfaceSectorExempt*>* ptr;

  void operator()(SurfaceSectorExempt* info)
  {
    SurfaceSectorExemptKey key(info->origLoc(), info->destLoc());
    if (!(key == prevKey))
    {
      ptr = new std::vector<SurfaceSectorExempt*>;
      cache.put(key, ptr);
      prevKey = key;
    }
    ptr->push_back(info);
  }
};

void
SurfaceSectorExemptHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<SurfaceSectorExempt*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllSurfaceSectorExemptHistorical sse(dbAdapter->getAdapter());
    sse.findAllSurfaceSectorExempt(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SurfaceSectorExemptHistoricalDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::for_each(recs.begin(), recs.end(), groupByKey());
}

void
SurfaceSectorExemptHistoricalDAO::destroy(SurfaceSectorExemptKey key,
                                          std::vector<SurfaceSectorExempt*>* recs)
{
  std::vector<SurfaceSectorExempt*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
SurfaceSectorExemptHistoricalDAO::compress(const std::vector<SurfaceSectorExempt*>* vect) const
{
  return compressVector(vect);
}

std::vector<SurfaceSectorExempt*>*
SurfaceSectorExemptHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<SurfaceSectorExempt>(compressed);
}

std::string
SurfaceSectorExemptHistoricalDAO::_name("SurfaceSectorExemptHistorical");
std::string
SurfaceSectorExemptHistoricalDAO::_cacheClass("BookingCode");
DAOHelper<SurfaceSectorExemptHistoricalDAO>
SurfaceSectorExemptHistoricalDAO::_helper(_name);

SurfaceSectorExemptHistoricalDAO* SurfaceSectorExemptHistoricalDAO::_instance = nullptr;

} // namespace tse
