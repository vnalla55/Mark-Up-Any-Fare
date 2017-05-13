//------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
#include "DBAccess/RuleCatAlphaCodeDAO.h"

#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetRuleCatAlphaCode.h"
#include "DBAccess/RuleCatAlphaCode.h"

namespace tse
{
log4cxx::LoggerPtr
RuleCatAlphaCodeDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.RuleCatAlphaCodeDAO"));
RuleCatAlphaCodeDAO&
RuleCatAlphaCodeDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<RuleCatAlphaCode*>&
getRuleCatAlphaCodeData(const AlphaCode& key,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical)
{
  if (isHistorical)
  {
    RuleCatAlphaCodeHistoricalDAO& dao = RuleCatAlphaCodeHistoricalDAO::instance();
    return dao.get(deleteList, key, ticketDate);
  }
  else
  {
    RuleCatAlphaCodeDAO& dao = RuleCatAlphaCodeDAO::instance();
    return dao.get(deleteList, key, ticketDate);
  }
}

const std::vector<RuleCatAlphaCode*>&
RuleCatAlphaCodeDAO::get(DeleteList& del, const AlphaCode& key, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(AlphaKey(key));
  del.copy(ptr);
  return *ptr;
}

AlphaKey
RuleCatAlphaCodeDAO::createKey(RuleCatAlphaCode* info)
{
  return AlphaKey(info->alphaRepresentation());
}

std::vector<RuleCatAlphaCode*>*
RuleCatAlphaCodeDAO::create(AlphaKey key)
{
  std::vector<RuleCatAlphaCode*>* ret = new std::vector<RuleCatAlphaCode*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetRuleCatAlphaCode rcac(dbAdapter->getAdapter());
    rcac.findRuleCatAlphaCode(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in RuleCatAlphaCodeDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
RuleCatAlphaCodeDAO::destroy(AlphaKey key, std::vector<RuleCatAlphaCode*>* recs)
{
  destroyContainer(recs);
}

void
RuleCatAlphaCodeDAO::load()
{
  StartupLoader<QueryGetAllRuleCatAlphaCode, RuleCatAlphaCode, RuleCatAlphaCodeDAO>();
}

std::string
RuleCatAlphaCodeDAO::_name("RuleCatAlphaCode");
std::string
RuleCatAlphaCodeDAO::_cacheClass("Rules");
DAOHelper<RuleCatAlphaCodeDAO>
RuleCatAlphaCodeDAO::_helper(_name);
RuleCatAlphaCodeDAO* RuleCatAlphaCodeDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: RuleCatAlphaCodeHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
RuleCatAlphaCodeHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.RuleCatAlphaCodeHistoricalDAO"));
RuleCatAlphaCodeHistoricalDAO&
RuleCatAlphaCodeHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<RuleCatAlphaCode*>&
RuleCatAlphaCodeHistoricalDAO::get(DeleteList& del,
                                   const AlphaCode& key,
                                   const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  // some of the objects aren't current, so we have to create a new vector to
  // return and copy all the current objects into it and return them.
  std::vector<RuleCatAlphaCode*>* ret =
      new std::vector<RuleCatAlphaCode*>(ptr->begin(), ptr->end());
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotCurrentCreateDateOnly<RuleCatAlphaCode>(ticketDate));

  return *ret;
}

std::vector<RuleCatAlphaCode*>*
RuleCatAlphaCodeHistoricalDAO::create(AlphaCode key)
{
  std::vector<RuleCatAlphaCode*>* ret = new std::vector<RuleCatAlphaCode*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetRuleCatAlphaCode rcac(dbAdapter->getAdapter());
    rcac.findRuleCatAlphaCode(*ret, key);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in RuleCatAlphaCodeHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
RuleCatAlphaCodeHistoricalDAO::destroy(AlphaCode key, std::vector<RuleCatAlphaCode*>* recs)
{
  destroyContainer(recs);
}

struct RuleCatAlphaCodeHistoricalDAO::groupByKey
{
public:
  AlphaCode prevKey;
  DAOCache& cache;

  groupByKey() : cache(RuleCatAlphaCodeHistoricalDAO::instance().cache()), ptr(nullptr) {}

  std::vector<RuleCatAlphaCode*>* ptr;

  void operator()(RuleCatAlphaCode* info)
  {
    if (!(info->alphaRepresentation() == prevKey))
    {
      ptr = new std::vector<RuleCatAlphaCode*>;
      cache.put(info->alphaRepresentation(), ptr);
      prevKey = info->alphaRepresentation();
    }
    ptr->push_back(info);
  }
};

void
RuleCatAlphaCodeHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<RuleCatAlphaCode*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllRuleCatAlphaCode rcac(dbAdapter->getAdapter());
    rcac.findAllRuleCatAlphaCode(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in RuleCatAlphaCodeHistoricalDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }
  std::for_each(recs.begin(), recs.end(), groupByKey());
}

std::string
RuleCatAlphaCodeHistoricalDAO::_name("RuleCatAlphaCodeHistorical");
std::string
RuleCatAlphaCodeHistoricalDAO::_cacheClass("Rules");
DAOHelper<RuleCatAlphaCodeHistoricalDAO>
RuleCatAlphaCodeHistoricalDAO::_helper(_name);
RuleCatAlphaCodeHistoricalDAO* RuleCatAlphaCodeHistoricalDAO::_instance = nullptr;

} // namespace tse
