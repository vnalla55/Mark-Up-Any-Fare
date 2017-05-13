//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/GlobalDirDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/GlobalDir.h"
#include "DBAccess/Queries/QueryGetGlobalDirs.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
GlobalDirDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.GlobalDirDAO"));

GlobalDirDAO&
GlobalDirDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const GlobalDir*
getGlobalDirData(const GlobalDirection& key,
                 const DateTime& date,
                 DeleteList& deleteList,
                 const DateTime& ticketDate,
                 bool isHistorical)
{
  if (isHistorical)
  {
    GlobalDirHistoricalDAO& dao = GlobalDirHistoricalDAO::instance();
    return dao.get(deleteList, key, date, ticketDate);
  }
  else
  {
    GlobalDirDAO& dao = GlobalDirDAO::instance();
    return dao.get(deleteList, key, date, ticketDate);
  }
}

const GlobalDir*
GlobalDirDAO::get(DeleteList& del,
                  const GlobalDirection& key,
                  const DateTime& date,
                  const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  IsEffectiveG<GlobalDir> isEffective(date, date, ticketDate);
  DAOCache::pointer_type ptr = cache().get(IntKey(0));
  del.copy(ptr);
  DAOCache::value_type::iterator i = find_if(ptr->begin(), ptr->end(), isEffective);
  while (i != ptr->end())
  {
    GlobalDir* globalDir = *i;
    if (globalDir->globalDir() == key)
    {
      return globalDir;
    }
    i = find_if(++i, ptr->end(), isEffective);
  }
  return nullptr;
}

const std::vector<GlobalDirSeg*>&
getGlobalDirSegData(const DateTime& date,
                    DeleteList& deleteList,
                    const DateTime& ticketDate,
                    bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    GlobalDirHistoricalDAO& dao = GlobalDirHistoricalDAO::instance();
    return dao.getAll(deleteList, date, ticketDate);
  }
  else
  {
    GlobalDirDAO& dao = GlobalDirDAO::instance();
    return dao.getAll(deleteList, date, ticketDate);
  }
}

const std::vector<GlobalDirSeg*>&
GlobalDirDAO::getAll(DeleteList& del, const DateTime& date, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetAllCallCount++;

  std::vector<GlobalDirSeg*>* ret = new std::vector<GlobalDirSeg*>;

  IsEffectiveG<GlobalDir> isEffective(date, date, ticketDate);
  DAOCache::pointer_type ptr = cache().get(IntKey(0));
  del.copy(ptr);
  DAOCache::value_type::iterator i = find_if(ptr->begin(), ptr->end(), isEffective);
  while (i != ptr->end())
  {
    GlobalDir& globalDir = **i;
    if (globalDir.displayOnlyInd() != 'Y')
    {
      std::vector<GlobalDirSeg>::iterator j;
      for (j = globalDir.segs().begin(); j != globalDir.segs().end(); j++)
      {
        ret->push_back(&(*j));
      }
    }
    i = find_if(++i, ptr->end(), isEffective);
  }

  del.adopt(ret);

  return *ret;
}

IntKey
GlobalDirDAO::createKey(GlobalDir* info)
{
  return IntKey(0);
}

void
GlobalDirDAO::load()
{
  StartupLoader<QueryGetGlobalDirs, GlobalDir, GlobalDirDAO>();
}

bool
GlobalDirDAO::insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
{
  GlobalDir* info(new GlobalDir);
  GlobalDir::dummyData(*info);
  DAOCache::pointer_type ptr = cache().get(IntKey(0));
  bool alreadyExists(false);

  for (std::vector<GlobalDir*>::const_iterator bit = ptr->begin(); bit != ptr->end(); ++bit)
  {
    const GlobalDir* thisDir((*bit));
    if (thisDir->globalDir() == info->globalDir())
    {
      alreadyExists = true;
      break;
    }
  }

  if (alreadyExists)
  {
    delete info;
  }
  else
  {
    cache().getCacheImpl()->queueDiskPut(IntKey(0), true);
    ptr->push_back(info);
  }

  return true;
}

std::vector<GlobalDir*>*
GlobalDirDAO::create(IntKey key)
{
  std::vector<GlobalDir*>* ret = new std::vector<GlobalDir*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetGlobalDirs gd(dbAdapter->getAdapter());
    gd.findAllGlobalDir(*ret);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in GlobalDirDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
GlobalDirDAO::destroy(IntKey key, std::vector<GlobalDir*>* recs)
{
  destroyContainer(recs);
}

std::string
GlobalDirDAO::_name("GlobalDir");
std::string
GlobalDirDAO::_cacheClass("Common");
DAOHelper<GlobalDirDAO>
GlobalDirDAO::_helper(_name);
GlobalDirDAO* GlobalDirDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: GlobalDirHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
GlobalDirHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.GlobalDirHistoricalDAO"));
GlobalDirHistoricalDAO&
GlobalDirHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const GlobalDir*
GlobalDirHistoricalDAO::get(DeleteList& del,
                            const GlobalDirection& key,
                            const DateTime& date,
                            const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  IsEffectiveHist<GlobalDir> isEffective(date, date, ticketDate);
  DAOCache::pointer_type ptr = cache().get(dummy);
  del.copy(ptr);
  DAOCache::value_type::iterator i = find_if(ptr->begin(), ptr->end(), isEffective);
  while (i != ptr->end())
  {
    GlobalDir* globalDir = *i;
    if (globalDir->globalDir() == key)
    {
      return globalDir;
    }
    i = find_if(++i, ptr->end(), isEffective);
  }
  return nullptr;
}

const std::vector<GlobalDirSeg*>&
GlobalDirHistoricalDAO::getAll(DeleteList& del, const DateTime& date, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetAllCallCount++;

  std::vector<GlobalDirSeg*>* ret = new std::vector<GlobalDirSeg*>;

  IsEffectiveHist<GlobalDir> isEffective(date, date, ticketDate);
  DAOCache::pointer_type ptr = cache().get(dummy);
  DAOCache::value_type::iterator i = find_if(ptr->begin(), ptr->end(), isEffective);
  while (i != ptr->end())
  {
    GlobalDir& globalDir = **i;
    if (globalDir.displayOnlyInd() != 'Y')
    {
      std::vector<GlobalDirSeg>::iterator j;
      for (j = globalDir.segs().begin(); j != globalDir.segs().end(); j++)
      {
        ret->push_back(&(*j));
      }
    }
    i = find_if(++i, ptr->end(), isEffective);
  }

  del.adopt(ret);

  return *ret;
}

std::vector<GlobalDir*>*
GlobalDirHistoricalDAO::create(int key)
{
  std::vector<GlobalDir*>* ret = new std::vector<GlobalDir*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetGlobalDirsHistorical gd(dbAdapter->getAdapter());
    gd.findAllGlobalDir(*ret);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in GlobalDirHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
GlobalDirHistoricalDAO::destroy(int key, std::vector<GlobalDir*>* recs)
{
  std::vector<GlobalDir*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

struct GlobalDirHistoricalDAO::groupByKey
{
public:
  int prevKey;

  DAOCache& cache;

  groupByKey() : prevKey(-1), cache(GlobalDirHistoricalDAO::instance().cache()), ptr(nullptr) {}

  std::vector<GlobalDir*>* ptr;

  void operator()(GlobalDir* info)
  {
    int key(info->globalDir());
    if (!(key == prevKey))
    {
      ptr = new std::vector<GlobalDir*>;
      cache.put(key, ptr);
      prevKey = key;
    }

    ptr->push_back(info);
  }
};

void
GlobalDirHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<GlobalDir*>* ret = new std::vector<GlobalDir*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllGlobalDirsHistorical gd(dbAdapter->getAdapter());
    gd.findAllGlobalDir(*ret);
    cache().put(dummy, ret);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in GlobalDirHistoricalDAO::load");
    destroyContainer(ret);
    throw;
  }
}

std::string
GlobalDirHistoricalDAO::_name("GlobalDirHistorical");
std::string
GlobalDirHistoricalDAO::_cacheClass("Common");
const int GlobalDirHistoricalDAO::dummy;
DAOHelper<GlobalDirHistoricalDAO>
GlobalDirHistoricalDAO::_helper(_name);
GlobalDirHistoricalDAO* GlobalDirHistoricalDAO::_instance = nullptr;

} // namespace tse
