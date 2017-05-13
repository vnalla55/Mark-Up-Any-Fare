//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/NoPNROptionsDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/NoPNROptions.h"
#include "DBAccess/Queries/QueryGetNoPNROptions.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
NoPNROptionsDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.NoPNROptionsDAO"));

NoPNROptionsDAO&
NoPNROptionsDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<NoPNROptions*>&
getNoPNROptionsData(const Indicator& userApplType,
                    const UserApplCode& userAppl,
                    DeleteList& deleteList,
                    const DateTime& ticketDate,
                    bool isHistorical)
{
  if (isHistorical)
  {
    NoPNROptionsHistoricalDAO& dao = NoPNROptionsHistoricalDAO::instance();
    return dao.get(deleteList, userApplType, userAppl, ticketDate);
  }
  else
  {
    NoPNROptionsDAO& dao = NoPNROptionsDAO::instance();
    return dao.get(deleteList, userApplType, userAppl, ticketDate);
  }
}

const std::vector<NoPNROptions*>&
NoPNROptionsDAO::get(DeleteList& del,
                     const Indicator userApplType,
                     const UserApplCode& userAppl,
                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  std::vector<NoPNROptions*>* recs = new std::vector<NoPNROptions*>;
  del.adopt(recs);

  NoPNROptionsKey key(userApplType, userAppl);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  copy(ptr->begin(), ptr->end(), back_inserter(*recs));

  if (userApplType != ' ' || userAppl != "")
  {
    NoPNROptionsKey key3(' ', "");
    ptr = cache().get(key3);
    del.copy(ptr);
    copy(ptr->begin(), ptr->end(), back_inserter(*recs));
  }

  return *recs;
}

std::vector<NoPNROptions*>*
NoPNROptionsDAO::create(NoPNROptionsKey key)
{
  std::vector<NoPNROptions*>* ret = new std::vector<NoPNROptions*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetNoPNROptions npo(dbAdapter->getAdapter());
    npo.findNoPNROptions(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in NoPNROptionsDAO::create");
    throw;
  }

  return ret;
}

void
NoPNROptionsDAO::destroy(NoPNROptionsKey key, std::vector<NoPNROptions*>* recs)
{
  std::vector<NoPNROptions*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
NoPNROptionsDAO::_name("NoPNROptions");
std::string
NoPNROptionsDAO::_cacheClass("Common");

DAOHelper<NoPNROptionsDAO>
NoPNROptionsDAO::_helper(_name);

NoPNROptionsDAO* NoPNROptionsDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: NoPNROptionsHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
NoPNROptionsHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.NoPNROptionsHistoricalDAO"));
NoPNROptionsHistoricalDAO&
NoPNROptionsHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<NoPNROptions*>&
NoPNROptionsHistoricalDAO::get(DeleteList& del,
                               const Indicator userApplType,
                               const UserApplCode& userAppl,
                               const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  std::vector<NoPNROptions*>* recs = new std::vector<NoPNROptions*>;
  del.adopt(recs);

  NoPNROptionsKey key(userApplType, userAppl);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  copy(ptr->begin(), ptr->end(), back_inserter(*recs));

  if (userApplType != ' ' || userAppl != "")
  {
    NoPNROptionsKey key3(' ', "");
    ptr = cache().get(key3);
    del.copy(ptr);
    copy(ptr->begin(), ptr->end(), back_inserter(*recs));
  }

  return *recs;
}

std::vector<NoPNROptions*>*
NoPNROptionsHistoricalDAO::create(NoPNROptionsKey key)
{
  std::vector<NoPNROptions*>* ret = new std::vector<NoPNROptions*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetNoPNROptions npo(dbAdapter->getAdapter());
    npo.findNoPNROptions(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in NoPNROptionsHistoricalDAO::create");
    throw;
  }

  return ret;
}

void
NoPNROptionsHistoricalDAO::destroy(NoPNROptionsKey key, std::vector<NoPNROptions*>* recs)
{
  std::vector<NoPNROptions*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
NoPNROptionsHistoricalDAO::_name("NoPNROptionsHistorical");
std::string
NoPNROptionsHistoricalDAO::_cacheClass("Common");
DAOHelper<NoPNROptionsHistoricalDAO>
NoPNROptionsHistoricalDAO::_helper(_name);

NoPNROptionsHistoricalDAO* NoPNROptionsHistoricalDAO::_instance = nullptr;

} // namespace tse
