//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/FCLimitationDAO.h"

#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/LimitationFare.h"
#include "DBAccess/Queries/QueryGetLimitations.h"

namespace tse
{
const char*
FCLimitationDAO::_dummyCacheKey("");

log4cxx::LoggerPtr
FCLimitationDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FCLimitationDAO"));

FCLimitationDAO&
FCLimitationDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<LimitationFare*>&
getFCLimitationData(const DateTime& date,
                    DeleteList& deleteList,
                    const DateTime& ticketDate,
                    bool isHistorical)
{
  if (isHistorical)
  {
    FCLimitationHistoricalDAO& dao = FCLimitationHistoricalDAO::instance();
    return dao.get(deleteList, FCLimitationDAO::_dummyCacheKey, date, ticketDate);
  }
  else
  {
    FCLimitationDAO& dao = FCLimitationDAO::instance();
    return dao.get(deleteList, FCLimitationDAO::_dummyCacheKey, date, ticketDate);
  }
}

const std::vector<LimitationFare*>&
FCLimitationDAO::get(DeleteList& del,
                     const UserApplCode& key,
                     const DateTime& date,
                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(UserApplCode(key));
  /*
  del.copy(ptr);
  std::vector<LimitationFare*>* ret = new std::vector<LimitationFare*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret),
  IsNotEffectiveG<LimitationFare>(date, ticketDate));
  return *ret;
  */
  return *(applyFilter(del, ptr, IsNotEffectiveG<LimitationFare>(date, ticketDate)));
}

std::vector<LimitationFare*>*
FCLimitationDAO::create(UserApplKey key)
{
  std::vector<LimitationFare*>* ret = new std::vector<LimitationFare*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetLimitFareBase lfb(dbAdapter->getAdapter());
    lfb.findLimitationFare(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FCLimitationDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FCLimitationDAO::destroy(UserApplKey key, std::vector<LimitationFare*>* recs)
{
  destroyContainer(recs);
}

UserApplKey
FCLimitationDAO::createKey(LimitationFare* info)
{
  return UserApplKey(_dummyCacheKey);
}

void
FCLimitationDAO::load()
{
  StartupLoader<QueryGetAllLimitFareBase, LimitationFare, FCLimitationDAO>();
}

bool
FCLimitationDAO::insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
{
  LimitationFare* info(new LimitationFare);
  LimitationFare::dummyData(*info);
  DAOCache::pointer_type ptr = cache().get(UserApplKey(_dummyCacheKey));
  bool alreadyExists(false);

  for (std::vector<LimitationFare*>::const_iterator bit = ptr->begin(); bit != ptr->end(); ++bit)
  {
    const LimitationFare* thisLimitation((*bit));
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
FCLimitationDAO::_name("FCLimitation");
std::string
FCLimitationDAO::_cacheClass("MinFares");
DAOHelper<FCLimitationDAO>
FCLimitationDAO::_helper(_name);
FCLimitationDAO* FCLimitationDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: FCLimitationHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
FCLimitationHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FCLimitationHistoricalDAO"));
FCLimitationHistoricalDAO&
FCLimitationHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<LimitationFare*>&
FCLimitationHistoricalDAO::get(DeleteList& del,
                               const UserApplCode& key,
                               const DateTime& date,
                               const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<LimitationFare*>* ret = new std::vector<LimitationFare*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveHist<LimitationFare>(date, ticketDate));
  return *ret;
}

std::vector<LimitationFare*>*
FCLimitationHistoricalDAO::create(UserApplCode key)
{
  std::vector<LimitationFare*>* ret = new std::vector<LimitationFare*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetLimitFareBaseHistorical lfb(dbAdapter->getAdapter());
    lfb.findLimitationFare(*ret, key);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FCLimitationHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FCLimitationHistoricalDAO::destroy(UserApplCode key, std::vector<LimitationFare*>* recs)
{
  destroyContainer(recs);
}

struct FCLimitationHistoricalDAO::groupByKey
{
public:
  UserApplCode prevKey;

  DAOCache& cache;

  groupByKey()
    : prevKey(INVALID_USERAPPLCODE), cache(FCLimitationHistoricalDAO::instance().cache()), ptr(nullptr)
  {
  }

  std::vector<LimitationFare*>* ptr;

  void operator()(LimitationFare* info)
  {
    if (info->userAppl() != prevKey)
    {
      ptr = new std::vector<LimitationFare*>;
      cache.put(info->userAppl(), ptr);
      prevKey = info->userAppl();
    }
    ptr->push_back(info);
  }
};

void
FCLimitationHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<LimitationFare*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllLimitFareBaseHistorical lfb(dbAdapter->getAdapter());
    lfb.findAllLimitationFare(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FCLimitationHistoricalDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::for_each(recs.begin(), recs.end(), groupByKey());
}

std::string
FCLimitationHistoricalDAO::_name("FCLimitationHistorical");
std::string
FCLimitationHistoricalDAO::_cacheClass("MinFares");
DAOHelper<FCLimitationHistoricalDAO>
FCLimitationHistoricalDAO::_helper(_name);
FCLimitationHistoricalDAO* FCLimitationHistoricalDAO::_instance = nullptr;

} // namespace tse
