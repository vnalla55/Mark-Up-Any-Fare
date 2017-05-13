//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/NationDAO.h"

#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Nation.h"
#include "DBAccess/Queries/QueryGetNation.h"

#include <algorithm>
#include <functional>
#include <map>

namespace tse
{
log4cxx::LoggerPtr
NationDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.NationDAO"));

NationDAO&
NationDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}
const Nation*
getNationData(const NationCode& nationCode,
              const DateTime& date,
              DeleteList& deleteList,
              const DateTime& ticketDate,
              bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    NationHistoricalDAO& dao = NationHistoricalDAO::instance();
    return dao.get(deleteList, nationCode, date, ticketDate);
  }
  else
  {
    NationDAO& dao = NationDAO::instance();
    return dao.get(deleteList, nationCode, date, ticketDate);
  }
}

const Nation*
NationDAO::get(DeleteList& del,
               const NationCode& key,
               const DateTime& date,
               const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(NationKey(key));
  del.copy(ptr);
  Nation* ret = nullptr;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveG<Nation>(date, ticketDate));
  if (LIKELY(i != ptr->end()))
    ret = *i;
  return ret;
}

const std::vector<Nation*>&
getAllNationData(const DateTime& ticketDate, DeleteList& deleteList, bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    NationHistoricalDAO& hdao = NationHistoricalDAO::instance();
    return hdao.getAll(deleteList, ticketDate);
  }
  else
  {
    NationDAO& dao = NationDAO::instance();
    return dao.getAll(deleteList, ticketDate);
  }
}

const std::vector<Nation*>&
NationDAO::getAll(DeleteList& del, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetAllCallCount++;

  std::vector<Nation*>* recs = new std::vector<Nation*>;
  del.adopt(recs);
  TSEReadGuard<> l(_loadListMutex);
  remove_copy_if(loadList.begin(),
                 loadList.end(),
                 back_inserter(*recs),
                 IsNotEffectiveG<Nation>(ticketDate, ticketDate, ticketDate));
  return *recs;
}

void
NationDAO::load()
{
  StartupLoader<QueryGetNations, Nation, NationDAO>();

  // Deep copy the cache to create our own sorted list
  typedef std::map<std::string, Nation*, std::less<std::string> > NATIONMAP;
  NATIONMAP nm;
  TSEWriteGuard<> l(_loadListMutex);
  std::shared_ptr<std::vector<NationKey>> keys = cache().keys();
  for (std::vector<NationKey>::const_iterator kit = keys->begin(); kit != keys->end(); ++kit)
  {
    const NationKey& key = (*kit);
    DAOCache::pointer_type ptr = cache().getIfResident(key);
    if (ptr)
    {
      for (std::vector<Nation*>::const_iterator cit = ptr->begin(); cit != ptr->end(); ++cit)
      {
        Nation* duplicate = (*cit)->newDuplicate();
        nm[duplicate->description()] = duplicate;
      }
    }
  }
  for (NATIONMAP::const_iterator nmit = nm.begin(); nmit != nm.end(); ++nmit)
  {
    loadList.push_back((*nmit).second);
  }
}

size_t
NationDAO::clear()
{
  size_t result(cache().clear());
  {
    TSEWriteGuard<> l(_loadListMutex);
    deleteVectorOfPointers(loadList);
  }
  load();
  return result;
}

size_t
NationDAO::invalidate(const ObjectKey& objectKey)
{
  // Remove the key from the "normal" cache
  size_t result(DataAccessObject<NationKey, std::vector<Nation*>, false>::invalidate(objectKey));

  // Go ahead and remove from LDC right now (don't wait for the next iteration
  // of the LDC action thread) to be sure it's gone
  NationKey key;
  translateKey(objectKey, key);
  //DISKCACHE.remove(time(NULL), true, _name, key.toString());

  // Re-populate the entire sorted list directly from the database to make sure we have
  // the latest and greatest; the item that was removed from the "normal" cache and LDC
  // will eventually be replaced via the standard caching mechanisms.
  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(cacheClass());
  {
    TSEWriteGuard<> l(_loadListMutex);
    deleteVectorOfPointers(loadList);
    try { QueryGetNations(dbAdapter->getAdapter()).execute(loadList); }
    catch (...)
    {
      LOG4CXX_WARN(_logger, "DB exception in object load for cache [NATION].");
      deleteVectorOfPointers(loadList);
      throw;
    }
  }
  return result;
}

NationKey
NationDAO::createKey(Nation* info)
{
  return NationKey(info->nation());
}

std::vector<Nation*>*
NationDAO::create(NationKey key)
{
  std::vector<Nation*>* ret = new std::vector<Nation*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetNation nat(dbAdapter->getAdapter());
    nat.findNation(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in NationDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
NationDAO::destroy(NationKey key, std::vector<Nation*>* recs)
{
  destroyContainer(recs);
}

std::string
NationDAO::_name("Nation");
std::string
NationDAO::_cacheClass("Common");

DAOHelper<NationDAO>
NationDAO::_helper(_name);

NationDAO* NationDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: NationHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
NationHistoricalDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.NationHistoricalDAO"));
NationHistoricalDAO&
NationHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const Nation*
NationHistoricalDAO::get(DeleteList& del,
                         const NationCode& key,
                         const DateTime& date,
                         const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  Nation* ret = nullptr;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveHist<Nation>(date, ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

const std::vector<Nation*>&
NationHistoricalDAO::getAll(DeleteList& del, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetAllCallCount++;

  std::vector<Nation*>* recs = new std::vector<Nation*>;
  del.adopt(recs);
  TSEReadGuard<> l(_loadListMutex);
  remove_copy_if(loadList.begin(),
                 loadList.end(),
                 back_inserter(*recs),
                 IsNotEffectiveHist<Nation>(ticketDate, ticketDate, ticketDate));
  return *recs;
}

std::vector<Nation*>*
NationHistoricalDAO::create(NationCode key)
{
  std::vector<Nation*>* ret = new std::vector<Nation*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetNationHistorical nat(dbAdapter->getAdapter());
    nat.findNation(*ret, key);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in NationHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

struct NationHistoricalDAO::groupByKey
{
public:
  DAOCache& cache;

  groupByKey() : cache(NationHistoricalDAO::instance().cache()) {}

  void operator()(Nation* info) { cache.get(info->nation()); }
};

void
NationHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;
  loadNationList();
  std::for_each(loadList.begin(), loadList.end(), groupByKey());
}

void
NationHistoricalDAO::loadNationList()
{
  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetNationsHistorical nat(dbAdapter->getAdapter());
    nat.findAllNations(loadList);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in NationHistoricalDAO::load");
    throw;
  }
}

size_t
NationHistoricalDAO::clear()
{
  size_t result(cache().clear());
  TSEWriteGuard<> l(_loadListMutex);
  deleteVectorOfPointers(loadList);
  load();
  return result;
}

size_t
NationHistoricalDAO::invalidate(const ObjectKey& objectKey)
{
  size_t result(HistoricalDataAccessObject<NationCode, std::vector<Nation*>, false>::invalidate(objectKey));
  TSEWriteGuard<> l(_loadListMutex);
  deleteVectorOfPointers(loadList);
  loadNationList();
  return result;
}

void
NationHistoricalDAO::destroy(NationCode key, std::vector<Nation*>* recs)
{
  std::vector<Nation*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
NationHistoricalDAO::_name("NationHistorical");
std::string
NationHistoricalDAO::_cacheClass("Common");
DAOHelper<NationHistoricalDAO>
NationHistoricalDAO::_helper(_name);

NationHistoricalDAO* NationHistoricalDAO::_instance = nullptr;

} // namespace tse
