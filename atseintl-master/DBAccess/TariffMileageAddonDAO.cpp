//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/TariffMileageAddonDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetTariffMileageAddon.h"
#include "DBAccess/TariffMileageAddon.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
TariffMileageAddonDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.TariffMileageAddonDAO"));

TariffMileageAddonDAO&
TariffMileageAddonDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const TariffMileageAddon*
getTariffMileageAddonData(const CarrierCode& carrier,
                          const LocCode& unpublishedAddonLoc,
                          const GlobalDirection& globalDir,
                          const DateTime& date,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical)
{
  if (isHistorical)
  {
    TariffMileageAddonHistoricalDAO& dao = TariffMileageAddonHistoricalDAO::instance();
    return dao.get(deleteList, carrier, unpublishedAddonLoc, globalDir, date, ticketDate);
  }
  else
  {
    TariffMileageAddonDAO& dao = TariffMileageAddonDAO::instance();
    return dao.get(deleteList, carrier, unpublishedAddonLoc, globalDir, date, ticketDate);
  }
}

const TariffMileageAddon*
TariffMileageAddonDAO::get(DeleteList& del,
                           const CarrierCode& carrier,
                           const LocCode& unpublishedAddonLoc,
                           const GlobalDirection& globalDir,
                           const DateTime& date,
                           const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  TariffMileageAddonKey key(carrier, unpublishedAddonLoc, globalDir);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  TariffMileageAddon* ret = nullptr;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveG<TariffMileageAddon>(date, ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

void
TariffMileageAddonDAO::load()
{
  StartupLoader<QueryGetAllTariffMileageAddon, TariffMileageAddon, TariffMileageAddonDAO>();
}

TariffMileageAddonKey
TariffMileageAddonDAO::createKey(TariffMileageAddon* info)
{
  return TariffMileageAddonKey(info->carrier(), info->unpublishedAddonLoc(), info->globalDir());
}

std::vector<TariffMileageAddon*>*
TariffMileageAddonDAO::create(TariffMileageAddonKey key)
{
  std::vector<TariffMileageAddon*>* ret = new std::vector<TariffMileageAddon*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTariffMileageAddon tma(dbAdapter->getAdapter());
    tma.findTariffMileageAddon(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TariffMileageAddonDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TariffMileageAddonDAO::destroy(TariffMileageAddonKey key, std::vector<TariffMileageAddon*>* recs)
{
  std::vector<TariffMileageAddon*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
TariffMileageAddonDAO::_name("TariffMileageAddon");
std::string
TariffMileageAddonDAO::_cacheClass("Routing");

DAOHelper<TariffMileageAddonDAO>
TariffMileageAddonDAO::_helper(_name);

TariffMileageAddonDAO* TariffMileageAddonDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: TariffMileageAddonHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
TariffMileageAddonHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.TariffMileageAddonHistoricalDAO"));
TariffMileageAddonHistoricalDAO&
TariffMileageAddonHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const TariffMileageAddon*
TariffMileageAddonHistoricalDAO::get(DeleteList& del,
                                     const CarrierCode& carrier,
                                     const LocCode& unpublishedAddonLoc,
                                     const GlobalDirection& globalDir,
                                     const DateTime& date,
                                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  TariffMileageAddonKey key(carrier, unpublishedAddonLoc, globalDir);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  TariffMileageAddon* ret = nullptr;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveHist<TariffMileageAddon>(date, ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<TariffMileageAddon*>*
TariffMileageAddonHistoricalDAO::create(TariffMileageAddonKey key)
{
  std::vector<TariffMileageAddon*>* ret = new std::vector<TariffMileageAddon*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTariffMileageAddonHistorical tma(dbAdapter->getAdapter());
    tma.findTariffMileageAddon(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TariffMileageAddonHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TariffMileageAddonHistoricalDAO::destroy(TariffMileageAddonKey key,
                                         std::vector<TariffMileageAddon*>* recs)
{
  std::vector<TariffMileageAddon*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

struct TariffMileageAddonHistoricalDAO::groupByKey
{
public:
  TariffMileageAddonKey prevKey;

  DAOCache& cache;

  groupByKey() : cache(TariffMileageAddonHistoricalDAO::instance().cache()), ptr(nullptr) {}

  std::vector<TariffMileageAddon*>* ptr;

  void operator()(TariffMileageAddon* info)
  {
    TariffMileageAddonKey key(info->carrier(), info->unpublishedAddonLoc(), info->globalDir());
    if (!(key == prevKey))
    {
      ptr = new std::vector<TariffMileageAddon*>;
      cache.put(key, ptr);
      prevKey = key;
    }
    ptr->push_back(info);
  }
};

void
TariffMileageAddonHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<TariffMileageAddon*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllTariffMileageAddonHistorical tma(dbAdapter->getAdapter());
    tma.findAllTariffMileageAddon(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TariffMileageAddonHistoricalDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::for_each(recs.begin(), recs.end(), groupByKey());
}

std::string
TariffMileageAddonHistoricalDAO::_name("TariffMileageAddonHistorical");
std::string
TariffMileageAddonHistoricalDAO::_cacheClass("Routing");
DAOHelper<TariffMileageAddonHistoricalDAO>
TariffMileageAddonHistoricalDAO::_helper(_name);

TariffMileageAddonHistoricalDAO* TariffMileageAddonHistoricalDAO::_instance = nullptr;

} // namespace tse
