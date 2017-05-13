//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/PfcEssAirSvcDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/PfcEssAirSvc.h"
#include "DBAccess/Queries/QueryGetPfcEssAirSvc.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
PfcEssAirSvcDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.PfcEssAirSvcDAO"));

// dao fix
size_t
PfcEssAirSvcDAO::clear()
{
  size_t result(cache().clear());
  //DISKCACHE.remove(time(NULL), true, _name, DISKCACHE_ALL);
  load();
  return result;
}

size_t
PfcEssAirSvcDAO::invalidate(const ObjectKey& objectKey)
{
  return clear();
}

PfcEssAirSvcDAO&
PfcEssAirSvcDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<PfcEssAirSvc*>&
getPfcEssAirSvcData(const LocCode& easHubArpt,
                    const LocCode& easArpt,
                    const DateTime& date,
                    DeleteList& deleteList,
                    const DateTime& ticketDate,
                    bool isHistorical)
{
  if (isHistorical)
  {
    PfcEssAirSvcHistoricalDAO& dao = PfcEssAirSvcHistoricalDAO::instance();
    return dao.get(deleteList, easHubArpt, easArpt, date, ticketDate);
  }
  else
  {
    PfcEssAirSvcDAO& dao = PfcEssAirSvcDAO::instance();
    return dao.get(deleteList, easHubArpt, easArpt, date, ticketDate);
  }
}

const std::vector<PfcEssAirSvc*>&
PfcEssAirSvcDAO::get(DeleteList& del,
                     const LocCode& easHubArpt,
                     const LocCode& easArpt,
                     const DateTime& date,
                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  PfcEssAirSvcKey key(easHubArpt, easArpt);
  DAOCache::pointer_type ptr = cache().get(key);
  return *(applyFilter(del, ptr, IsNotEffectiveG<PfcEssAirSvc>(date, ticketDate)));
}

const std::vector<PfcEssAirSvc*>&
getAllPfcEssAirSvcData(DeleteList& del)
{
  PfcEssAirSvcDAO& dao = PfcEssAirSvcDAO::instance();
  return dao.getAll(del);
}

const std::vector<PfcEssAirSvc*>&
PfcEssAirSvcDAO::getAll(DeleteList& del)
{
  // Track calls for code coverage
  _codeCoverageGetAllCallCount++;

  std::shared_ptr<std::vector<PfcEssAirSvcKey>> keys = cache().keys();

  std::vector<PfcEssAirSvc*>* ret = new std::vector<PfcEssAirSvc*>;

  for (auto& elem : *keys)
  {
    DAOCache::pointer_type ptr = cache().get(elem);
    del.copy(ptr);
    std::copy(ptr->begin(), ptr->end(), std::back_inserter(*ret));
  }

  del.adopt(ret);

  return *ret;
}

void
PfcEssAirSvcDAO::load()
{
  StartupLoader<QueryGetAllPfcEssAirSvc, PfcEssAirSvc, PfcEssAirSvcDAO>();
}

PfcEssAirSvcKey
PfcEssAirSvcDAO::createKey(PfcEssAirSvc* info)
{
  return PfcEssAirSvcKey(info->easHubArpt(), info->easArpt());
}

std::vector<PfcEssAirSvc*>*
PfcEssAirSvcDAO::create(PfcEssAirSvcKey key)
{
  std::vector<PfcEssAirSvc*>* ret = new std::vector<PfcEssAirSvc*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetPfcEssAirSvc eas(dbAdapter->getAdapter());
    eas.findPfcEssAirSvc(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in PfcEssAirSvcDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
PfcEssAirSvcDAO::destroy(PfcEssAirSvcKey key, std::vector<PfcEssAirSvc*>* recs)
{
  destroyContainer(recs);
}

sfc::CompressedData*
PfcEssAirSvcDAO::compress(const std::vector<PfcEssAirSvc*>* vect) const
{
  return compressVector(vect);
}

std::vector<PfcEssAirSvc*>*
PfcEssAirSvcDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<PfcEssAirSvc>(compressed);
}

std::string
PfcEssAirSvcDAO::_name("PfcEssAirSvc");
std::string
PfcEssAirSvcDAO::_cacheClass("Taxes");

DAOHelper<PfcEssAirSvcDAO>
PfcEssAirSvcDAO::_helper(_name);

PfcEssAirSvcDAO* PfcEssAirSvcDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: PfcEssAirSvcHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
PfcEssAirSvcHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.PfcEssAirSvcHistoricalDAO"));
PfcEssAirSvcHistoricalDAO&
PfcEssAirSvcHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<PfcEssAirSvc*>&
PfcEssAirSvcHistoricalDAO::get(DeleteList& del,
                               const LocCode& easHubArpt,
                               const LocCode& easArpt,
                               const DateTime& date,
                               const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  PfcEssAirSvcHistoricalKey key(easHubArpt, easArpt);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<PfcEssAirSvc*>* ret = new std::vector<PfcEssAirSvc*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveHist<PfcEssAirSvc>(date, ticketDate));
  ret->erase(std::remove_if(ret->begin(), ret->end(), Inhibit<PfcEssAirSvc>), ret->end());

  return *ret;
}

std::vector<PfcEssAirSvc*>*
PfcEssAirSvcHistoricalDAO::create(PfcEssAirSvcHistoricalKey key)
{
  std::vector<PfcEssAirSvc*>* ret = new std::vector<PfcEssAirSvc*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetPfcEssAirSvcHistorical eas(dbAdapter->getAdapter());
    eas.findPfcEssAirSvc(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in PfcEssAirSvcHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
PfcEssAirSvcHistoricalDAO::destroy(PfcEssAirSvcHistoricalKey key, std::vector<PfcEssAirSvc*>* recs)
{
  destroyContainer(recs);
}

sfc::CompressedData*
PfcEssAirSvcHistoricalDAO::compress(const std::vector<PfcEssAirSvc*>* vect) const
{
  return compressVector(vect);
}

std::vector<PfcEssAirSvc*>*
PfcEssAirSvcHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<PfcEssAirSvc>(compressed);
}

std::string
PfcEssAirSvcHistoricalDAO::_name("PfcEssAirSvcHistorical");
std::string
PfcEssAirSvcHistoricalDAO::_cacheClass("Taxes");
DAOHelper<PfcEssAirSvcHistoricalDAO>
PfcEssAirSvcHistoricalDAO::_helper(_name);
PfcEssAirSvcHistoricalDAO* PfcEssAirSvcHistoricalDAO::_instance = nullptr;

} // namespace tse
