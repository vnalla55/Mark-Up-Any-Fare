//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/SalesNationRestrDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetSalesNationRestr.h"
#include "DBAccess/SalesNationRestr.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
SalesNationRestrDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SalesNationRestrDAO"));

SalesNationRestrDAO&
SalesNationRestrDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<SalesNationRestr*>&
getSalesNationRestrData(const NationCode& key,
                        const DateTime& date,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    SalesNationRestrHistoricalDAO& dao = SalesNationRestrHistoricalDAO::instance();
    return dao.get(deleteList, key, date, ticketDate);
  }
  else
  {
    SalesNationRestrDAO& dao = SalesNationRestrDAO::instance();
    return dao.get(deleteList, key, date, ticketDate);
  }
}

const std::vector<SalesNationRestr*>&
SalesNationRestrDAO::get(DeleteList& del,
                         const NationCode& key,
                         const DateTime& date,
                         const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(key);
  /*
  del.copy(ptr);
  std::vector<SalesNationRestr*>* ret = new std::vector<SalesNationRestr*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret),
  IsNotEffectiveG<SalesNationRestr>(date, ticketDate));
  return *ret;
  */
  return *(applyFilter(del, ptr, IsNotEffectiveG<SalesNationRestr>(date, ticketDate)));
}

std::vector<SalesNationRestr*>*
SalesNationRestrDAO::create(NationCode key)
{
  std::vector<SalesNationRestr*>* ret = new std::vector<SalesNationRestr*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSalesNationRestrBase snr(dbAdapter->getAdapter());
    snr.findSalesNationRestr(*ret, key);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SalesNationRestrDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SalesNationRestrDAO::destroy(NationCode key, std::vector<SalesNationRestr*>* recs)
{
  std::vector<SalesNationRestr*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
SalesNationRestrDAO::_name("SalesNationRestr");
std::string
SalesNationRestrDAO::_cacheClass("Rules");
DAOHelper<SalesNationRestrDAO>
SalesNationRestrDAO::_helper(_name);
SalesNationRestrDAO* SalesNationRestrDAO::_instance = nullptr;
// --------------------------------------------------
// Historical DAO: SalesNationRestrHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
SalesNationRestrHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SalesNationRestrHistoricalDAO"));
SalesNationRestrHistoricalDAO&
SalesNationRestrHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<SalesNationRestr*>&
SalesNationRestrHistoricalDAO::get(DeleteList& del,
                                   const NationCode& key,
                                   const DateTime& date,
                                   const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<SalesNationRestr*>* ret = new std::vector<SalesNationRestr*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveHist<SalesNationRestr>(date, ticketDate));
  return *ret;
}

struct SalesNationRestrHistoricalDAO::groupByKey
{
public:
  NationCode prevKey;

  DAOCache& cache;

  groupByKey() : cache(SalesNationRestrHistoricalDAO::instance().cache()), ptr(nullptr) {}

  std::vector<SalesNationRestr*>* ptr;

  void operator()(SalesNationRestr* info)
  {
    NationCode key(info->nation());
    if (!(key == prevKey) || ptr == nullptr)
    {
      ptr = new std::vector<SalesNationRestr*>;
      cache.put(key, ptr);
      prevKey = key;
    }

    ptr->push_back(info);
  }
};

void
SalesNationRestrHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<SalesNationRestr*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllSalesNationRestrBaseHistorical ssb(dbAdapter->getAdapter());
    ssb.findAllSalesNationRestr(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SalesNationRestrHistoricalDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::for_each(recs.begin(), recs.end(), groupByKey());
}

std::vector<SalesNationRestr*>*
SalesNationRestrHistoricalDAO::create(NationCode key)
{
  std::vector<SalesNationRestr*>* ret = new std::vector<SalesNationRestr*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSalesNationRestrBaseHistorical snr(dbAdapter->getAdapter());
    snr.findSalesNationRestr(*ret, key);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SalesNationRestrHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SalesNationRestrHistoricalDAO::destroy(NationCode key, std::vector<SalesNationRestr*>* recs)
{
  std::vector<SalesNationRestr*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
SalesNationRestrHistoricalDAO::_name("SalesNationRestrHistorical");
std::string
SalesNationRestrHistoricalDAO::_cacheClass("Rules");
DAOHelper<SalesNationRestrHistoricalDAO>
SalesNationRestrHistoricalDAO::_helper(_name);
SalesNationRestrHistoricalDAO* SalesNationRestrHistoricalDAO::_instance = nullptr;

} // namespace tse
