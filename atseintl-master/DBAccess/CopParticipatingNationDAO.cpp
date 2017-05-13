//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/CopParticipatingNationDAO.h"

#include "Common/Logger.h"
#include "DBAccess/CopParticipatingNation.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetCopParticipatingNation.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
CopParticipatingNationDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.CopParticipatingNationDAO"));

CopParticipatingNationDAO&
CopParticipatingNationDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const CopParticipatingNation*
getCopParticipatingNationData(const NationCode& nation,
                              const NationCode& copNation,
                              const DateTime& date,
                              DeleteList& deleteList,
                              const DateTime& ticketDate,
                              bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    CopParticipatingNationHistoricalDAO& dao = CopParticipatingNationHistoricalDAO::instance();
    return dao.get(deleteList, nation, copNation, date, ticketDate);
  }
  else
  {
    CopParticipatingNationDAO& dao = CopParticipatingNationDAO::instance();
    return dao.get(deleteList, nation, copNation, date, ticketDate);
  }
}

const CopParticipatingNation*
CopParticipatingNationDAO::get(DeleteList& del,
                               const NationCode& nation,
                               const NationCode& copNation,
                               const DateTime& date,
                               const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  CopParticipatingNationKey key(nation, copNation);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  CopParticipatingNation* ret = nullptr;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveG<CopParticipatingNation>(date, ticketDate));
  if (i != ptr->end())
    ret = *i;

  return ret;
}

void
CopParticipatingNationDAO::load()
{
  StartupLoader<QueryGetAllCopParticipatingNation,
                CopParticipatingNation,
                CopParticipatingNationDAO>();
}

CopParticipatingNationKey
CopParticipatingNationDAO::createKey(CopParticipatingNation* info)
{
  return CopParticipatingNationKey(info->nation(), info->copNation());
}

std::vector<CopParticipatingNation*>*
CopParticipatingNationDAO::create(CopParticipatingNationKey key)
{
  std::vector<CopParticipatingNation*>* ret = new std::vector<CopParticipatingNation*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCopParticipatingNation cpn(dbAdapter->getAdapter());
    cpn.findCopParticipatingNation(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CopParticipatingNationDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CopParticipatingNationDAO::destroy(CopParticipatingNationKey key,
                                   std::vector<CopParticipatingNation*>* recs)
{
  std::vector<CopParticipatingNation*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
CopParticipatingNationDAO::_name("CopParticipatingNation");
std::string
CopParticipatingNationDAO::_cacheClass("MinFares");

DAOHelper<CopParticipatingNationDAO>
CopParticipatingNationDAO::_helper(_name);

CopParticipatingNationDAO* CopParticipatingNationDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: CopParticipatingNationHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
CopParticipatingNationHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.CopParticipatingNationHistoricalDAO"));
CopParticipatingNationHistoricalDAO&
CopParticipatingNationHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const CopParticipatingNation*
CopParticipatingNationHistoricalDAO::get(DeleteList& del,
                                         const NationCode& nation,
                                         const NationCode& copNation,
                                         const DateTime& date,
                                         const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  CopParticipatingNationKey key(nation, copNation);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  CopParticipatingNation* ret = nullptr;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveHist<CopParticipatingNation>(date, ticketDate));
  if (i != ptr->end())
    ret = *i;

  return ret;
}

std::vector<CopParticipatingNation*>*
CopParticipatingNationHistoricalDAO::create(CopParticipatingNationKey key)
{
  std::vector<CopParticipatingNation*>* ret = new std::vector<CopParticipatingNation*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCopParticipatingNationHistorical cpn(dbAdapter->getAdapter());
    cpn.findCopParticipatingNation(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CopParticipatingNationHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CopParticipatingNationHistoricalDAO::destroy(CopParticipatingNationKey key,
                                             std::vector<CopParticipatingNation*>* recs)
{
  std::vector<CopParticipatingNation*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

struct CopParticipatingNationHistoricalDAO::groupByKey
{
public:
  CopParticipatingNationKey prevKey;

  DAOCache& cache;

  groupByKey() : cache(CopParticipatingNationHistoricalDAO::instance().cache()), ptr(nullptr) {}

  std::vector<CopParticipatingNation*>* ptr;

  void operator()(CopParticipatingNation* info)
  {
    CopParticipatingNationKey key(info->nation(), info->copNation());
    if (!(key == prevKey))
    {
      ptr = new std::vector<CopParticipatingNation*>;
      cache.put(key, ptr);
      prevKey = key;
    }
    ptr->push_back(info);
  }
};

void
CopParticipatingNationHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<CopParticipatingNation*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllCopParticipatingNationHistorical cpn(dbAdapter->getAdapter());
    cpn.findAllCopParticipatingNation(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in QueryGetAllCopParticipatingNationHistorical::load");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::for_each(recs.begin(), recs.end(), groupByKey());
}

std::string
CopParticipatingNationHistoricalDAO::_name("CopParticipatingNationHistorical");
std::string
CopParticipatingNationHistoricalDAO::_cacheClass("MinFares");
DAOHelper<CopParticipatingNationHistoricalDAO>
CopParticipatingNationHistoricalDAO::_helper(_name);
CopParticipatingNationHistoricalDAO* CopParticipatingNationHistoricalDAO::_instance = nullptr;

} // namespace tse
