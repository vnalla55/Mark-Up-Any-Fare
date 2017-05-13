//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/NoPNRFareTypeGroupDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/NoPNRFareTypeGroup.h"
#include "DBAccess/Queries/QueryGetNoPNRFareTypeGroup.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
NoPNRFareTypeGroupDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.NoPNRFareTypeGroupDAO"));

NoPNRFareTypeGroupDAO&
NoPNRFareTypeGroupDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const NoPNRFareTypeGroup*
getNoPNRFareTypeGroupData(const int fareTypeGroup,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical)
{
  if (isHistorical)
  {
    NoPNRFareTypeGroupHistoricalDAO& dao = NoPNRFareTypeGroupHistoricalDAO::instance();
    return dao.get(deleteList, fareTypeGroup, ticketDate);
  }
  else
  {
    NoPNRFareTypeGroupDAO& dao = NoPNRFareTypeGroupDAO::instance();
    return dao.get(deleteList, fareTypeGroup, ticketDate);
  }
}

const NoPNRFareTypeGroup*
NoPNRFareTypeGroupDAO::get(DeleteList& del, const int fareTypeGroup, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  IntKey key(fareTypeGroup);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;
  else
  {
    del.copy(ptr);
    return ptr->front();
  }
}

std::vector<NoPNRFareTypeGroup*>*
NoPNRFareTypeGroupDAO::create(IntKey key)
{
  std::vector<NoPNRFareTypeGroup*>* ret = new std::vector<NoPNRFareTypeGroup*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetNoPNRFareTypeGroup npftg(dbAdapter->getAdapter());
    npftg.findNoPNRFareTypeGroup(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in NoPNRFareTypeGroupDAO::create");
    throw;
  }

  return ret;
}

void
NoPNRFareTypeGroupDAO::destroy(IntKey key, std::vector<NoPNRFareTypeGroup*>* recs)
{
  std::vector<NoPNRFareTypeGroup*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
NoPNRFareTypeGroupDAO::_name("NoPNRFareTypeGroup");
std::string
NoPNRFareTypeGroupDAO::_cacheClass("Common");

DAOHelper<NoPNRFareTypeGroupDAO>
NoPNRFareTypeGroupDAO::_helper(_name);

NoPNRFareTypeGroupDAO* NoPNRFareTypeGroupDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: NoPNRFareTypeGroupHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
NoPNRFareTypeGroupHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.NoPNRFareTypeGroupHistoricalDAO"));
NoPNRFareTypeGroupHistoricalDAO&
NoPNRFareTypeGroupHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const NoPNRFareTypeGroup*
NoPNRFareTypeGroupHistoricalDAO::get(DeleteList& del,
                                     const int fareTypeGroup,
                                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  NoPNRFareTypeGroupKey key(fareTypeGroup);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;
  else
  {
    del.copy(ptr);
    return ptr->front();
  }
}

std::vector<NoPNRFareTypeGroup*>*
NoPNRFareTypeGroupHistoricalDAO::create(NoPNRFareTypeGroupKey key)
{
  std::vector<NoPNRFareTypeGroup*>* ret = new std::vector<NoPNRFareTypeGroup*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetNoPNRFareTypeGroup npftg(dbAdapter->getAdapter());
    npftg.findNoPNRFareTypeGroup(*ret, key);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in NoPNRFareTypeGroupHistoricalDAO::create");
    throw;
  }

  return ret;
}

void
NoPNRFareTypeGroupHistoricalDAO::destroy(NoPNRFareTypeGroupKey key,
                                         std::vector<NoPNRFareTypeGroup*>* recs)
{
  std::vector<NoPNRFareTypeGroup*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
NoPNRFareTypeGroupHistoricalDAO::_name("NoPNRFareTypeGroupHistorical");
std::string
NoPNRFareTypeGroupHistoricalDAO::_cacheClass("Common");
DAOHelper<NoPNRFareTypeGroupHistoricalDAO>
NoPNRFareTypeGroupHistoricalDAO::_helper(_name);

NoPNRFareTypeGroupHistoricalDAO* NoPNRFareTypeGroupHistoricalDAO::_instance = nullptr;

} // namespace tse
