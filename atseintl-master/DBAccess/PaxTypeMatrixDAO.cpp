//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/PaxTypeMatrixDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/PaxTypeMatrix.h"
#include "DBAccess/Queries/QueryGetPaxTypeMatrix.h"

namespace tse
{
log4cxx::LoggerPtr
PaxTypeMatrixDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.PaxTypeMatrixDAO"));
PaxTypeMatrixDAO&
PaxTypeMatrixDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<const PaxTypeMatrix*>&
getPaxTypeMatrixData(const PaxTypeCode& key,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical)
{
  if (isHistorical)
  {
    PaxTypeMatrixHistoricalDAO& dao = PaxTypeMatrixHistoricalDAO::instance();
    return dao.get(deleteList, key, ticketDate);
  }
  else
  {
    PaxTypeMatrixDAO& dao = PaxTypeMatrixDAO::instance();
    return dao.get(deleteList, key, ticketDate);
  }
}

const std::vector<const PaxTypeMatrix*>&
PaxTypeMatrixDAO::get(DeleteList& del, const PaxTypeCode& key, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(PaxCodeKey(key));
  del.copy(ptr);
  return *ptr;
}

void
PaxTypeMatrixDAO::load()
{
  StartupLoader<QueryGetPaxTypeMatrixs, PaxTypeMatrix, PaxTypeMatrixDAO>();
}

PaxCodeKey
PaxTypeMatrixDAO::createKey(const PaxTypeMatrix* info)
{
  return PaxCodeKey(info->sabrePaxType());
}

std::vector<const PaxTypeMatrix*>*
PaxTypeMatrixDAO::create(PaxCodeKey key)
{
  std::vector<const PaxTypeMatrix*>* ret = new std::vector<const PaxTypeMatrix*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetPaxTypeMatrix ptm(dbAdapter->getAdapter());
    ptm.findPaxTypeMatrix(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in PaxTypeMatrixDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
PaxTypeMatrixDAO::destroy(PaxCodeKey key, std::vector<const PaxTypeMatrix*>* recs)
{
  destroyContainer(recs);
}

std::string
PaxTypeMatrixDAO::_name("PaxTypeMatrix");
std::string
PaxTypeMatrixDAO::_cacheClass("Common");
DAOHelper<PaxTypeMatrixDAO>
PaxTypeMatrixDAO::_helper(_name);
PaxTypeMatrixDAO* PaxTypeMatrixDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: PaxTypeMatrixHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
PaxTypeMatrixHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.PaxTypeMatrixHistoricalDAO"));
PaxTypeMatrixHistoricalDAO&
PaxTypeMatrixHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<const PaxTypeMatrix*>&
PaxTypeMatrixHistoricalDAO::get(DeleteList& del, const PaxTypeCode& key, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  // some of the objects aren't current, so we have to create a new vector to
  // return and copy all the current objects into it and return them.
  std::vector<const PaxTypeMatrix*>* ret =
      new std::vector<const PaxTypeMatrix*>(ptr->begin(), ptr->end());
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotCurrentCreateDateOnly<PaxTypeMatrix>(ticketDate));

  return *ret;
}

struct PaxTypeMatrixHistoricalDAO::groupByKey
{
public:
  PaxTypeCode prevKey;

  DAOCache& cache;

  groupByKey() : cache(PaxTypeMatrixHistoricalDAO::instance().cache()), ptr(nullptr) {}

  std::vector<const PaxTypeMatrix*>* ptr;

  void operator()(const PaxTypeMatrix* info)
  {
    PaxTypeCode key(info->sabrePaxType());
    if (!(key == prevKey))
    {
      ptr = new std::vector<const PaxTypeMatrix*>;
      cache.put(key, ptr);
      prevKey = key;
    }

    ptr->push_back(info);
  }
};

void
PaxTypeMatrixHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<const PaxTypeMatrix*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetPaxTypeMatrixs ptm(dbAdapter->getAdapter());
    ptm.findAllPaxTypeMatrix(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in QueryGetPaxTypeMatrixsHistorical::load");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::for_each(recs.begin(), recs.end(), groupByKey());
}

std::vector<const PaxTypeMatrix*>*
PaxTypeMatrixHistoricalDAO::create(PaxTypeCode key)
{
  std::vector<const PaxTypeMatrix*>* ret = new std::vector<const PaxTypeMatrix*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetPaxTypeMatrix ptm(dbAdapter->getAdapter());
    ptm.findPaxTypeMatrix(*ret, key);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in PaxTypeMatrixHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
PaxTypeMatrixHistoricalDAO::destroy(PaxTypeCode key, std::vector<const PaxTypeMatrix*>* recs)
{
  std::vector<const PaxTypeMatrix*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
PaxTypeMatrixHistoricalDAO::_name("PaxTypeMatrixHistorical");
std::string
PaxTypeMatrixHistoricalDAO::_cacheClass("Common");
DAOHelper<PaxTypeMatrixHistoricalDAO>
PaxTypeMatrixHistoricalDAO::_helper(_name);
PaxTypeMatrixHistoricalDAO* PaxTypeMatrixHistoricalDAO::_instance = nullptr;
} // namespace tse
