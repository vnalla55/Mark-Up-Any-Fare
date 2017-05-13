//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/CopMinimumDAO.h"

#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DBAccess/CopMinimum.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetCopMinimum.h"

namespace tse
{
log4cxx::LoggerPtr
CopMinimumDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.CopMinimumDAO"));
CopMinimumDAO&
CopMinimumDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<CopMinimum*>&
getCopMinimumData(const NationCode& key,
                  const DateTime& date,
                  DeleteList& deleteList,
                  const DateTime& ticketDate,
                  bool isHistorical)
{
  if (isHistorical)
  {
    CopMinimumHistoricalDAO& dao = CopMinimumHistoricalDAO::instance();
    return dao.get(deleteList, key, date, ticketDate);
  }
  else
  {
    CopMinimumDAO& dao = CopMinimumDAO::instance();
    return dao.get(deleteList, key, date, ticketDate);
  }
}

const std::vector<CopMinimum*>&
CopMinimumDAO::get(DeleteList& del,
                   const NationCode& key,
                   const DateTime& date,
                   const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(NationKey(key));
  return *(applyFilter(del, ptr, IsNotEffectiveG<CopMinimum>(date, ticketDate)));
}

void
CopMinimumDAO::load()
{
  StartupLoader<QueryGetAllCopMinBase, CopMinimum, CopMinimumDAO>();
}

NationKey
CopMinimumDAO::createKey(CopMinimum* info)
{
  return NationKey(info->copNation());
}

std::vector<CopMinimum*>*
CopMinimumDAO::create(NationKey key)
{
  std::vector<CopMinimum*>* ret = new std::vector<CopMinimum*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCopMinBase cm(dbAdapter->getAdapter());
    cm.findCopMinimum(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CopMinimumDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CopMinimumDAO::destroy(NationKey key, std::vector<CopMinimum*>* recs)
{
  std::vector<CopMinimum*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
CopMinimumDAO::_name("CopMinimum");
std::string
CopMinimumDAO::_cacheClass("MinFares");
DAOHelper<CopMinimumDAO>
CopMinimumDAO::_helper(_name);
CopMinimumDAO* CopMinimumDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: CopMinimumHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
CopMinimumHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.CopMinimumHistoricalDAO"));
CopMinimumHistoricalDAO&
CopMinimumHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<CopMinimum*>&
CopMinimumHistoricalDAO::get(DeleteList& del,
                             const NationCode& key,
                             const DateTime& date,
                             const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<CopMinimum*>* ret = new std::vector<CopMinimum*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveHist<CopMinimum>(date, ticketDate));

  return *ret;
}

struct CopMinimumHistoricalDAO::groupByKey
{
public:
  NationCode prevKey;

  DAOCache& cache;

  groupByKey()
    : prevKey(INVALID_NATIONCODE), cache(CopMinimumHistoricalDAO::instance().cache()), ptr(nullptr)
  {
  }

  std::vector<CopMinimum*>* ptr;

  void operator()(CopMinimum* info)
  {
    if (info->copNation() != prevKey)
    {
      ptr = new std::vector<CopMinimum*>;
      cache.put(info->copNation(), ptr);
      prevKey = info->copNation();
    }
    ptr->push_back(info);
  }
};

void
CopMinimumHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<CopMinimum*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllCopMinBaseHistorical cm(dbAdapter->getAdapter());
    cm.findAllCopMinimum(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CopMinimumDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::for_each(recs.begin(), recs.end(), groupByKey());
}

std::vector<CopMinimum*>*
CopMinimumHistoricalDAO::create(NationCode key)
{
  std::vector<CopMinimum*>* ret = new std::vector<CopMinimum*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCopMinBaseHistorical cm(dbAdapter->getAdapter());
    cm.findCopMinimum(*ret, key);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CopMinimumHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CopMinimumHistoricalDAO::destroy(NationCode key, std::vector<CopMinimum*>* recs)
{
  std::vector<CopMinimum*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
CopMinimumHistoricalDAO::_name("CopMinimumHistorical");
std::string
CopMinimumHistoricalDAO::_cacheClass("MinFares");
DAOHelper<CopMinimumHistoricalDAO>
CopMinimumHistoricalDAO::_helper(_name);
CopMinimumHistoricalDAO* CopMinimumHistoricalDAO::_instance = nullptr;

} // namespace tse
