//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/JLimitationDAO.h"

#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/LimitationJrny.h"
#include "DBAccess/Queries/QueryGetLimitations.h"

namespace tse
{
const char*
JLimitationDAO::_dummyCacheKey("");

log4cxx::LoggerPtr
JLimitationDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.JLimitationDAO"));

JLimitationDAO&
JLimitationDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<LimitationJrny*>&
getJLimitationData(const DateTime& date,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical)
{
  if (isHistorical)
  {
    JLimitationHistoricalDAO& dao = JLimitationHistoricalDAO::instance();
    return dao.get(deleteList, JLimitationDAO::_dummyCacheKey, date, ticketDate);
  }
  else
  {
    JLimitationDAO& dao = JLimitationDAO::instance();
    return dao.get(deleteList, JLimitationDAO::_dummyCacheKey, date, ticketDate);
  }
}

const std::vector<LimitationJrny*>&
JLimitationDAO::get(DeleteList& del,
                    const UserApplCode& key,
                    const DateTime& date,
                    const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(UserApplKey(key));
  /*
  del.copy(ptr);
  std::vector<LimitationJrny*>* ret = new std::vector<LimitationJrny*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret),
  IsNotEffectiveG<LimitationJrny>(date, ticketDate));
  return *ret;
  */
  return *(applyFilter(del, ptr, IsNotEffectiveG<LimitationJrny>(date, ticketDate)));
}

std::vector<LimitationJrny*>*
JLimitationDAO::create(UserApplKey key)
{
  std::vector<LimitationJrny*>* ret = new std::vector<LimitationJrny*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetLimitJrnyBase ljb(dbAdapter->getAdapter());
    ljb.findLimitationJrny(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in JLimitationDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
JLimitationDAO::destroy(UserApplKey key, std::vector<LimitationJrny*>* recs)
{
  destroyContainer(recs);
}

UserApplKey
JLimitationDAO::createKey(LimitationJrny* info)
{
  return UserApplKey(_dummyCacheKey);
}

void
JLimitationDAO::load()
{
  StartupLoader<QueryGetAllLimitJrnyBase, LimitationJrny, JLimitationDAO>();
}

bool
JLimitationDAO::insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
{
  LimitationJrny* info(new LimitationJrny);
  LimitationJrny::dummyData(*info);
  DAOCache::pointer_type ptr = cache().get(UserApplKey(_dummyCacheKey));
  bool alreadyExists(false);

  for (std::vector<LimitationJrny*>::const_iterator bit = ptr->begin(); bit != ptr->end(); ++bit)
  {
    const LimitationJrny* thisLimitation((*bit));
    if (thisLimitation->userAppl() == info->userAppl())
    {
      alreadyExists = true;
      break;
    }
  }

  if (alreadyExists)
  {
    delete info;
  }
  else
  {
    ptr->push_back(info);
    cache().getCacheImpl()->queueDiskPut(UserApplKey(_dummyCacheKey), true);
  }

  return true;
}

std::string
JLimitationDAO::_name("JLimitation");
std::string
JLimitationDAO::_cacheClass("MinFares");
DAOHelper<JLimitationDAO>
JLimitationDAO::_helper(_name);
JLimitationDAO* JLimitationDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: JLimitationHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
JLimitationHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.JLimitationHistoricalDAO"));
JLimitationHistoricalDAO&
JLimitationHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<LimitationJrny*>&
JLimitationHistoricalDAO::get(DeleteList& del,
                              const UserApplCode& key,
                              const DateTime& date,
                              const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<LimitationJrny*>* ret = new std::vector<LimitationJrny*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveHist<LimitationJrny>(date, ticketDate));
  return *ret;
}

std::vector<LimitationJrny*>*
JLimitationHistoricalDAO::create(UserApplCode key)
{
  std::vector<LimitationJrny*>* ret = new std::vector<LimitationJrny*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetLimitJrnyBaseHistorical ljb(dbAdapter->getAdapter());
    ljb.findLimitationJrny(*ret, key);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in JLimitationHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
JLimitationHistoricalDAO::destroy(UserApplCode key, std::vector<LimitationJrny*>* recs)
{
  destroyContainer(recs);
}

struct JLimitationHistoricalDAO::groupByKey
{
public:
  UserApplCode prevKey;

  DAOCache& cache;

  groupByKey()
    : prevKey(INVALID_USERAPPLCODE), cache(JLimitationHistoricalDAO::instance().cache()), ptr(nullptr)
  {
  }

  std::vector<LimitationJrny*>* ptr;

  void operator()(LimitationJrny* info)
  {
    if (info->userAppl() != prevKey)
    {
      ptr = new std::vector<LimitationJrny*>;
      cache.put(info->userAppl(), ptr);
      prevKey = info->userAppl();
    }
    ptr->push_back(info);
  }
};

void
JLimitationHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<LimitationJrny*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllLimitJrnyBaseHistorical ljb(dbAdapter->getAdapter());
    ljb.findAllLimitationJrny(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in JLimitationHistoricalDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::for_each(recs.begin(), recs.end(), groupByKey());
}

std::string
JLimitationHistoricalDAO::_name("JLimitationHistorical");
std::string
JLimitationHistoricalDAO::_cacheClass("MinFares");
DAOHelper<JLimitationHistoricalDAO>
JLimitationHistoricalDAO::_helper(_name);
JLimitationHistoricalDAO* JLimitationHistoricalDAO::_instance = nullptr;

} // namespace tse
