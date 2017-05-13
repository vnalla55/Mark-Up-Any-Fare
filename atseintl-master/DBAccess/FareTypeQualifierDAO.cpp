//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/FareTypeQualifierDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareTypeQualifier.h"
#include "DBAccess/Queries/QueryGetFareTypeQualifier.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
FareTypeQualifierDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FareTypeQualifierDAO"));

FareTypeQualifierDAO&
FareTypeQualifierDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareTypeQualifier*>&
getFareTypeQualifierData(const Indicator& userApplType,
                         const UserApplCode& userAppl,
                         const FareType& qualifier,
                         DeleteList& deleteList,
                         const DateTime& ticketDate,
                         bool isHistorical)
{
  if (isHistorical)
  {
    FareTypeQualifierHistoricalDAO& dao = FareTypeQualifierHistoricalDAO::instance();
    return dao.get(deleteList, userApplType, userAppl, qualifier, ticketDate);
  }
  else
  {
    FareTypeQualifierDAO& dao = FareTypeQualifierDAO::instance();
    return dao.get(deleteList, userApplType, userAppl, qualifier, ticketDate);
  }
}

const std::vector<FareTypeQualifier*>&
FareTypeQualifierDAO::get(DeleteList& del,
                          const Indicator userApplType,
                          const UserApplCode& userAppl,
                          const FareType& fareTypeQualifier,
                          const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareTypeQualifierKey key(userApplType, userAppl, fareTypeQualifier);
  DAOCache::pointer_type ptr = cache().get(key);
  /*
  del.copy(ptr);
  std::vector<FareTypeQualifier*>* recs = new std::vector<FareTypeQualifier*>;
  del.adopt(recs);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*recs),
                 IsNotCurrentG<FareTypeQualifier>(ticketDate));

  return *recs;
  */
  return *(applyFilter(del, ptr, IsNotCurrentG<FareTypeQualifier>(ticketDate)));
}

FareTypeQualifierKey
FareTypeQualifierDAO::createKey(FareTypeQualifier* info)
{
  return FareTypeQualifierKey(info->userApplType(), info->userAppl(), info->fareTypeQualifier());
}

void
FareTypeQualifierDAO::load()
{
  StartupLoader<QueryGetAllFareTypeQualifier, FareTypeQualifier, FareTypeQualifierDAO>();
}

std::vector<FareTypeQualifier*>*
FareTypeQualifierDAO::create(FareTypeQualifierKey key)
{
  std::vector<FareTypeQualifier*>* ret = new std::vector<FareTypeQualifier*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareTypeQualifier fcc(dbAdapter->getAdapter());
    fcc.findFareTypeQualifier(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareTypeQualifierDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FareTypeQualifierDAO::destroy(FareTypeQualifierKey key, std::vector<FareTypeQualifier*>* recs)
{
  destroyContainer(recs);
}

std::string
FareTypeQualifierDAO::_name("FareTypeQualifier");
std::string
FareTypeQualifierDAO::_cacheClass("Common");

DAOHelper<FareTypeQualifierDAO>
FareTypeQualifierDAO::_helper(_name);

FareTypeQualifierDAO* FareTypeQualifierDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: FareTypeQualifierHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
FareTypeQualifierHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareTypeQualifierHistoricalDAO"));
FareTypeQualifierHistoricalDAO&
FareTypeQualifierHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareTypeQualifier*>&
FareTypeQualifierHistoricalDAO::get(DeleteList& del,
                                    const Indicator userApplType,
                                    const UserApplCode& userAppl,
                                    const FareType& fareTypeQualifier,
                                    const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  std::vector<FareTypeQualifier*>* recs = new std::vector<FareTypeQualifier*>;
  del.adopt(recs);

  FareTypeQualifierKey key(userApplType, userAppl, fareTypeQualifier);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  //    copy(ptr->begin(), ptr->end(), back_inserter(*recs));
  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*recs), IsNotCurrentH<FareTypeQualifier>(ticketDate));

  return *recs;
}

struct FareTypeQualifierHistoricalDAO::groupByKey
{
public:
  FareTypeQualifierKey prevKey;

  DAOCache& cache;

  groupByKey() : cache(FareTypeQualifierHistoricalDAO::instance().cache()), ptr(nullptr) {}

  std::vector<FareTypeQualifier*>* ptr;

  void operator()(FareTypeQualifier* info)
  {
    FareTypeQualifierKey key(info->userApplType(), info->userAppl(), info->fareTypeQualifier());
    if (!(key == prevKey))
    {
      ptr = new std::vector<FareTypeQualifier*>;
      cache.put(key, ptr);
      prevKey = key;
    }
    ptr->push_back(info);
  }
};

void
FareTypeQualifierHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<FareTypeQualifier*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());

  try
  {
    QueryGetAllFareTypeQualifierHistorical fcc(dbAdapter->getAdapter());
    fcc.findAllFareTypeQualifier(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareTypeQualifierHistoricalDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::for_each(recs.begin(), recs.end(), groupByKey());
}

std::vector<FareTypeQualifier*>*
FareTypeQualifierHistoricalDAO::create(FareTypeQualifierKey key)
{
  std::vector<FareTypeQualifier*>* ret = new std::vector<FareTypeQualifier*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareTypeQualifierHistorical fcc(dbAdapter->getAdapter());
    fcc.findFareTypeQualifier(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareTypeQualifierHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FareTypeQualifierHistoricalDAO::destroy(FareTypeQualifierKey key,
                                        std::vector<FareTypeQualifier*>* recs)
{
  destroyContainer(recs);
}

std::string
FareTypeQualifierHistoricalDAO::_name("FareTypeQualifierHistorical");
std::string
FareTypeQualifierHistoricalDAO::_cacheClass("Common");
DAOHelper<FareTypeQualifierHistoricalDAO>
FareTypeQualifierHistoricalDAO::_helper(_name);

FareTypeQualifierHistoricalDAO* FareTypeQualifierHistoricalDAO::_instance = nullptr;

} // namespace tse
