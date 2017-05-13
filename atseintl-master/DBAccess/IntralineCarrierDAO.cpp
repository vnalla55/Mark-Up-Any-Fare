//----------------------------------------------------------------------------
//     (c)2015, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/IntralineCarrierDAO.h"

#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/IntralineCarrierInfo.h"
#include "DBAccess/Queries/QueryGetAllIntralineCarrier.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
IntralineCarrierDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.IntralineCarrierDAO"));

IntralineCarrierDAO&
IntralineCarrierDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<IntralineCarrierInfo*>&
getIntralineCarrierData(DeleteList& deleteList, bool isHistorical)
{
  IntralineCarrierDAO& dao = IntralineCarrierDAO::instance();
  return dao.get(deleteList);
}

const std::vector<IntralineCarrierInfo*>&
IntralineCarrierDAO::get(DeleteList& del)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  return *cache().get(WHOLE_TBL_STR_KEY).get();
}

size_t
IntralineCarrierDAO::clear()
{
  size_t result(cache().clear());
  // refresh the cache
  load();
  return result;
}

void
IntralineCarrierDAO::load()
{
  StartupLoader<QueryGetAllIntralineCarrier, IntralineCarrierInfo, IntralineCarrierDAO>();
}

std::vector<IntralineCarrierInfo*>*
IntralineCarrierDAO::create(NameKey key)
{
  std::vector<IntralineCarrierInfo*>* ret = new std::vector<IntralineCarrierInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllIntralineCarrier itc(dbAdapter->getAdapter());
    itc.findAllIntralineCarrier(*ret);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in IntralineCarrierDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
IntralineCarrierDAO::destroy(NameKey key, std::vector<IntralineCarrierInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
IntralineCarrierDAO::_name("IntralineCarrier");
std::string
IntralineCarrierDAO::_cacheClass("Common");

DAOHelper<IntralineCarrierDAO>
IntralineCarrierDAO::_helper(_name);
NameKey
IntralineCarrierDAO::WHOLE_TBL_STR_KEY(EMPTY_STRING());
IntralineCarrierDAO* IntralineCarrierDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: IntralineCarrierHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
IntralineCarrierHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.IntralineCarrierHistoricalDAO"));
IntralineCarrierHistoricalDAO&
IntralineCarrierHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<IntralineCarrierInfo*>&
IntralineCarrierHistoricalDAO::get(DeleteList& del)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  std::shared_ptr<std::vector<std::string>> keys = cache().keys();

  std::vector<IntralineCarrierInfo*>* ret = new std::vector<IntralineCarrierInfo*>;

  for (auto& elem : *keys)
  {
    DAOCache::pointer_type ptr = cache().get(elem);
    del.copy(ptr);
    std::copy(ptr->begin(), ptr->end(), std::back_inserter(*ret));
  }

  del.adopt(ret);

  return *ret;
}

struct IntralineCarrierHistoricalDAO::groupByKey
{
public:
  std::string prevKey;

  DAOCache& cache;

  groupByKey()
    : prevKey(INVALID_CARRIERCODE),
      cache(IntralineCarrierHistoricalDAO::instance().cache()),
      ptr(nullptr)
  {
  }

  std::vector<IntralineCarrierInfo*>* ptr;

  void operator()(IntralineCarrierInfo* info)
  {
    if (info->name() != prevKey)
    {
      ptr = new std::vector<IntralineCarrierInfo*>;
      cache.put(info->name(), ptr);
      prevKey = info->name();
    }
    ptr->push_back(info);
  }
};

void
IntralineCarrierHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<IntralineCarrierInfo*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllIntralineCarrier itc(dbAdapter->getAdapter());
    itc.findAllIntralineCarrier(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in IntralineCarrierHistoricalDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::for_each(recs.begin(), recs.end(), groupByKey());
}

std::vector<IntralineCarrierInfo*>*
IntralineCarrierHistoricalDAO::create(std::string key)
{
  std::vector<IntralineCarrierInfo*>* ret = new std::vector<IntralineCarrierInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllIntralineCarrier itc(dbAdapter->getAdapter());
    itc.findAllIntralineCarrier(*ret);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in IntralineCarrierHistoricalDAO::create");
    throw;
  }

  return ret;
}

void
IntralineCarrierHistoricalDAO::destroy(std::string key, std::vector<IntralineCarrierInfo*>* recs)
{
  std::vector<IntralineCarrierInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
IntralineCarrierHistoricalDAO::_name("IntralineCarrierHistorical");
std::string
IntralineCarrierHistoricalDAO::_cacheClass("Common");
DAOHelper<IntralineCarrierHistoricalDAO>
IntralineCarrierHistoricalDAO::_helper(_name);
IntralineCarrierHistoricalDAO* IntralineCarrierHistoricalDAO::_instance = nullptr;

} // namespace tse
