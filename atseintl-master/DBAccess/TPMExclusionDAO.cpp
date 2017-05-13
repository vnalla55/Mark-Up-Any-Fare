//------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
#include "DBAccess/TPMExclusionDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetTPMExclusion.h"
#include "DBAccess/TPMExclusion.h"

namespace tse
{
log4cxx::LoggerPtr
TPMExclusionDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.TPMExclusionDAO"));

TPMExclusionDAO&
TPMExclusionDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<TPMExclusion*>&
getTPMExclusionsData(const CarrierCode& carrier,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    TPMExclusionHistoricalDAO& dao = TPMExclusionHistoricalDAO::instance();
    return dao.get(deleteList, carrier, ticketDate);
  }
  else
  {
    TPMExclusionDAO& dao = TPMExclusionDAO::instance();
    return dao.get(deleteList, carrier, ticketDate);
  }
}

const std::vector<TPMExclusion*>&
TPMExclusionDAO::get(DeleteList& del, const CarrierCode& carrier, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  TPMExclKey key(carrier);
  TPMExclKey keyBlank("");
  DAOCache::pointer_type ptr = cache().get(key);
  DAOCache::pointer_type ptrBlank = cache().get(keyBlank);
  del.copy(ptr);
  del.copy(ptrBlank);
  std::vector<TPMExclusion*>* ret = new std::vector<TPMExclusion*>;
  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotEffectiveG<TPMExclusion>(ticketDate));
  remove_copy_if(ptrBlank->begin(),
                 ptrBlank->end(),
                 back_inserter(*ret),
                 IsNotEffectiveG<TPMExclusion>(ticketDate));
  del.adopt(ret);
  return *ret;
}

TPMExclKey
TPMExclusionDAO::createKey(TPMExclusion* info)
{
  return TPMExclKey(info->carrier());
}

std::vector<TPMExclusion*>*
TPMExclusionDAO::create(TPMExclKey key)
{
  std::vector<TPMExclusion*>* ret = new std::vector<TPMExclusion*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTPMExclusion ds(dbAdapter->getAdapter());
    ds.findTPMExcl(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TPMExclusionDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TPMExclusionDAO::destroy(TPMExclKey key, std::vector<TPMExclusion*>* recs)
{
  destroyContainer(recs);
}

void
TPMExclusionDAO::load()
{
  StartupLoader<QueryGetAllTPMExclusion, TPMExclusion, TPMExclusionDAO>();
}

std::string
TPMExclusionDAO::_name("TpmExclusion");
std::string
TPMExclusionDAO::_cacheClass("Routing");

DAOHelper<TPMExclusionDAO>
TPMExclusionDAO::_helper(_name);

TPMExclusionDAO* TPMExclusionDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: TPMExclusionHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
TPMExclusionHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.TPMExclusionHistoricalDAO"));
TPMExclusionHistoricalDAO&
TPMExclusionHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<TPMExclusion*>&
TPMExclusionHistoricalDAO::get(DeleteList& del,
                               const CarrierCode& carrier,
                               const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  TPMExclKey key(carrier);
  TPMExclKey keyBlank("");
  DAOCache::pointer_type ptr = cache().get(key);
  DAOCache::pointer_type ptrBlank = cache().get(keyBlank);
  del.copy(ptr);
  del.copy(ptrBlank);
  std::vector<TPMExclusion*>* ret = new std::vector<TPMExclusion*>;
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveH<TPMExclusion>(ticketDate, ticketDate));
  remove_copy_if(ptrBlank->begin(),
                 ptrBlank->end(),
                 back_inserter(*ret),
                 IsNotEffectiveH<TPMExclusion>(ticketDate, ticketDate));
  del.adopt(ret);
  return *ret;
}

std::vector<TPMExclusion*>*
TPMExclusionHistoricalDAO::create(TPMExclKey key)
{
  std::vector<TPMExclusion*>* ret = new std::vector<TPMExclusion*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTPMExclusionHistorical ds(dbAdapter->getAdapter());
    ds.findTPMExcl(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TPMExclusionHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TPMExclusionHistoricalDAO::destroy(TPMExclKey key, std::vector<TPMExclusion*>* recs)
{
  destroyContainer(recs);
}

struct TPMExclusionHistoricalDAO::groupByKey
{
public:
  TPMExclKey prevKey;
  DAOCache& cache;

  groupByKey() : cache(TPMExclusionHistoricalDAO::instance().cache()), ptr(nullptr) {}

  std::vector<TPMExclusion*>* ptr;

  void operator()(TPMExclusion* info)
  {
    TPMExclKey key(info->carrier());
    if (!(key == prevKey))
    {
      ptr = new std::vector<TPMExclusion*>;
      cache.put(key, ptr);
      prevKey = key;
    }
    ptr->push_back(info);
  }
};

void
TPMExclusionHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<TPMExclusion*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllTPMExclusionHistorical ds(dbAdapter->getAdapter());
    ds.findAllTPMExcl(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TPMExclusionHistoricalDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }
  std::for_each(recs.begin(), recs.end(), groupByKey());
}

std::string
TPMExclusionHistoricalDAO::_name("TpmExclusionHistorical");
std::string
TPMExclusionHistoricalDAO::_cacheClass("Routing");
DAOHelper<TPMExclusionHistoricalDAO>
TPMExclusionHistoricalDAO::_helper(_name);

TPMExclusionHistoricalDAO* TPMExclusionHistoricalDAO::_instance = nullptr;

} // namespace tse
