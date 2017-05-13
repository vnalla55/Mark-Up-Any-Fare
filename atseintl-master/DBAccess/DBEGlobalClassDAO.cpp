//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/DBEGlobalClassDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DBEGlobalClass.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetDBEGlobalClass.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
DBEGlobalClassDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.DBEGlobalClassDAO"));

DBEGlobalClassDAO&
DBEGlobalClassDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<DBEGlobalClass*>&
getDBEGlobalClassData(const DBEClass& key,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    DBEGlobalClassHistoricalDAO& dao = DBEGlobalClassHistoricalDAO::instance();
    return dao.get(deleteList, key, ticketDate);
  }
  else
  {
    DBEGlobalClassDAO& dao = DBEGlobalClassDAO::instance();
    return dao.get(deleteList, key, ticketDate);
  }
}

const std::vector<DBEGlobalClass*>&
DBEGlobalClassDAO::get(DeleteList& del, const DBEClass& key, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(DBEClassKey(key));
  del.copy(ptr);
  return *ptr;
}

DBEClassKey
DBEGlobalClassDAO::createKey(DBEGlobalClass* info)
{
  return DBEClassKey(info->dbeGlobalClass());
}

std::vector<DBEGlobalClass*>*
DBEGlobalClassDAO::create(DBEClassKey key)
{
  std::vector<DBEGlobalClass*>* ret = new std::vector<DBEGlobalClass*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetDBEGlobalClass dbe(dbAdapter->getAdapter());
    dbe.findDBEGlobalClass(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in DBEGlobalClassDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
DBEGlobalClassDAO::destroy(DBEClassKey key, std::vector<DBEGlobalClass*>* recs)
{
  destroyContainer(recs);
}

void
DBEGlobalClassDAO::load()
{
  StartupLoader<QueryGetAllDBEGlobalClass, DBEGlobalClass, DBEGlobalClassDAO>();
}

std::string
DBEGlobalClassDAO::_name("DBEGlobalClass");
std::string
DBEGlobalClassDAO::_cacheClass("Fares");

DAOHelper<DBEGlobalClassDAO>
DBEGlobalClassDAO::_helper(_name);

DBEGlobalClassDAO* DBEGlobalClassDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: DBEGlobalClassHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
DBEGlobalClassHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.DBEGlobalClassHistoricalDAO"));
DBEGlobalClassHistoricalDAO&
DBEGlobalClassHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<DBEGlobalClass*>&
DBEGlobalClassHistoricalDAO::get(DeleteList& del, const DBEClass& key, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  return *ptr;
}

std::vector<DBEGlobalClass*>*
DBEGlobalClassHistoricalDAO::create(DBEClass key)
{
  std::vector<DBEGlobalClass*>* ret = new std::vector<DBEGlobalClass*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetDBEGlobalClass dbe(dbAdapter->getAdapter());
    dbe.findDBEGlobalClass(*ret, key);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in DBEGlobalClassHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
DBEGlobalClassHistoricalDAO::destroy(DBEClass key, std::vector<DBEGlobalClass*>* recs)
{
  destroyContainer(recs);
}

std::string
DBEGlobalClassHistoricalDAO::_name("DBEGlobalClassHistorical");
std::string
DBEGlobalClassHistoricalDAO::_cacheClass("Fares");
DAOHelper<DBEGlobalClassHistoricalDAO>
DBEGlobalClassHistoricalDAO::_helper(_name);
DBEGlobalClassHistoricalDAO* DBEGlobalClassHistoricalDAO::_instance = nullptr;

} // namespace tse
