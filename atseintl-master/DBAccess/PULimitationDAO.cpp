//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/PULimitationDAO.h"

#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/LimitationCmn.h"
#include "DBAccess/Queries/QueryGetLimitations.h"

namespace tse
{
const char*
PULimitationDAO::_dummyCacheKey("");
log4cxx::LoggerPtr
PULimitationDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.PULimitationDAO"));
PULimitationDAO&
PULimitationDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<LimitationCmn*>&
PULimitationDAO::get(DeleteList& del,
                     const UserApplCode& key,
                     const DateTime& date,
                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(UserApplKey(key));
  /*
  del.copy(ptr);
  std::vector<LimitationCmn*>* ret = new std::vector<LimitationCmn*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), IsNotEffectiveG<LimitationCmn>(date,
  ticketDate));
  return *ret;
  */
  return *(applyFilter(del, ptr, IsNotEffectiveG<LimitationCmn>(date, ticketDate)));
}

std::vector<LimitationCmn*>*
PULimitationDAO::create(UserApplKey key)
{
  std::vector<LimitationCmn*>* ret = new std::vector<LimitationCmn*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetLimitationPU lpu(dbAdapter->getAdapter());
    lpu.findLimitationPU(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in PULimitationDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
PULimitationDAO::destroy(UserApplKey key, std::vector<LimitationCmn*>* recs)
{
  destroyContainer(recs);
}

UserApplKey
PULimitationDAO::createKey(LimitationCmn* info)
{
  return UserApplKey(_dummyCacheKey);
}

void
PULimitationDAO::load()
{
  StartupLoader<QueryGetAllLimitationPU, LimitationCmn, PULimitationDAO>();
}

bool
PULimitationDAO::insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
{
  LimitationCmn* info(new LimitationCmn);
  LimitationCmn::dummyData(*info);
  DAOCache::pointer_type ptr = cache().get(UserApplKey(_dummyCacheKey));
  bool alreadyExists(false);

  for (std::vector<LimitationCmn*>::const_iterator bit = ptr->begin(); bit != ptr->end(); ++bit)
  {
    const LimitationCmn* thisLimitation((*bit));
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
PULimitationDAO::_name("PULimitation");
std::string
PULimitationDAO::_cacheClass("MinFares");
DAOHelper<PULimitationDAO>
PULimitationDAO::_helper(_name);
PULimitationDAO* PULimitationDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: PULimitationHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
PULimitationHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.PULimitationHistoricalDAO"));
PULimitationHistoricalDAO&
PULimitationHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<LimitationCmn*>&
PULimitationHistoricalDAO::get(DeleteList& del,
                               const UserApplCode& key,
                               const DateTime& date,
                               const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<LimitationCmn*>* ret = new std::vector<LimitationCmn*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveHist<LimitationCmn>(date, ticketDate));
  return *ret;
}

std::vector<LimitationCmn*>*
PULimitationHistoricalDAO::create(UserApplCode key)
{
  std::vector<LimitationCmn*>* ret = new std::vector<LimitationCmn*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetLimitationPUHistorical lpu(dbAdapter->getAdapter());
    lpu.findLimitationPU(*ret, key);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in PULimitationHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
PULimitationHistoricalDAO::destroy(UserApplCode key, std::vector<LimitationCmn*>* recs)
{
  destroyContainer(recs);
}

struct PULimitationHistoricalDAO::groupByKey
{
public:
  UserApplCode prevKey;

  DAOCache& cache;

  groupByKey()
    : prevKey(INVALID_USERAPPLCODE), cache(PULimitationHistoricalDAO::instance().cache()), ptr(nullptr)
  {
  }

  std::vector<LimitationCmn*>* ptr;

  void operator()(LimitationCmn* info)
  {
    if (info->userAppl() != prevKey)
    {
      ptr = new std::vector<LimitationCmn*>;
      cache.put(info->userAppl(), ptr);
      prevKey = info->userAppl();
    }
    ptr->push_back(info);
  }
};

void
PULimitationHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<LimitationCmn*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllLimitationPUHistorical lpu(dbAdapter->getAdapter());
    lpu.findAllLimitationPU(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in PULimitationHistoricalDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::for_each(recs.begin(), recs.end(), groupByKey());
}

std::string
PULimitationHistoricalDAO::_name("PULimitationHistorical");
std::string
PULimitationHistoricalDAO::_cacheClass("MinFares");
DAOHelper<PULimitationHistoricalDAO>
PULimitationHistoricalDAO::_helper(_name);
PULimitationHistoricalDAO* PULimitationHistoricalDAO::_instance = nullptr;

} // namespace tse
