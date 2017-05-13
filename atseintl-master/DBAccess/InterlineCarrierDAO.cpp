//----------------------------------------------------------------------------
//     (c)2015, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/InterlineCarrierDAO.h"

#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/InterlineCarrierInfo.h"
#include "DBAccess/Queries/QueryGetAllInterlineCarrier.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
InterlineCarrierDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.InterlineCarrierDAO"));

InterlineCarrierDAO&
InterlineCarrierDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<InterlineCarrierInfo*>&
getInterlineCarrierData(DeleteList& deleteList, bool isHistorical)
{
  InterlineCarrierDAO& dao = InterlineCarrierDAO::instance();
  return dao.get(deleteList);
}

const std::vector<InterlineCarrierInfo*>&
InterlineCarrierDAO::get(DeleteList& del)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  return *cache().get(WHOLE_TBL_KEY).get();
}

size_t
InterlineCarrierDAO::clear()
{
  size_t result(cache().clear());
  // refresh the cache
  load();
  return result;
}
void
InterlineCarrierDAO::load()
{
  StartupLoader<QueryGetAllInterlineCarrier, InterlineCarrierInfo, InterlineCarrierDAO>();
}

std::vector<InterlineCarrierInfo*>*
InterlineCarrierDAO::create(CarrierKey key)
{
  std::vector<InterlineCarrierInfo*>* ret = new std::vector<InterlineCarrierInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllInterlineCarrier itc(dbAdapter->getAdapter());
    itc.findAllInterlineCarrier(*ret);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in InterlineCarrierDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
InterlineCarrierDAO::destroy(CarrierKey key, std::vector<InterlineCarrierInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
InterlineCarrierDAO::_name("InterlineCarrier");
std::string
InterlineCarrierDAO::_cacheClass("Common");

DAOHelper<InterlineCarrierDAO>
InterlineCarrierDAO::_helper(_name);
CarrierKey
InterlineCarrierDAO::WHOLE_TBL_KEY(EMPTY_CARRIER);
InterlineCarrierDAO* InterlineCarrierDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: InterlineCarrierHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
InterlineCarrierHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.InterlineCarrierHistoricalDAO"));
InterlineCarrierHistoricalDAO&
InterlineCarrierHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<InterlineCarrierInfo*>&
InterlineCarrierHistoricalDAO::get(DeleteList& del)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  std::shared_ptr<std::vector<CarrierCode>> keys = cache().keys();

  std::vector<InterlineCarrierInfo*>* ret = new std::vector<InterlineCarrierInfo*>;

  for (auto& elem : *keys)
  {
    DAOCache::pointer_type ptr = cache().get(elem);
    del.copy(ptr);
    std::copy(ptr->begin(), ptr->end(), std::back_inserter(*ret));
  }

  del.adopt(ret);

  return *ret;
}

struct InterlineCarrierHistoricalDAO::groupByKey
{
public:
  CarrierCode prevKey;

  DAOCache& cache;

  groupByKey()
    : prevKey(INVALID_CARRIERCODE),
      cache(InterlineCarrierHistoricalDAO::instance().cache()),
      ptr(nullptr)
  {
  }

  std::vector<InterlineCarrierInfo*>* ptr;

  void operator()(InterlineCarrierInfo* info)
  {
    if (info->carrier() != prevKey)
    {
      ptr = new std::vector<InterlineCarrierInfo*>;
      cache.put(info->carrier(), ptr);
      prevKey = info->carrier();
    }
    ptr->push_back(info);
  }
};

void
InterlineCarrierHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<InterlineCarrierInfo*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllInterlineCarrier itc(dbAdapter->getAdapter());
    itc.findAllInterlineCarrier(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in InterlineCarrierHistoricalDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::for_each(recs.begin(), recs.end(), groupByKey());
}

std::vector<InterlineCarrierInfo*>*
InterlineCarrierHistoricalDAO::create(CarrierCode key)
{
  std::vector<InterlineCarrierInfo*>* ret = new std::vector<InterlineCarrierInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllInterlineCarrier itc(dbAdapter->getAdapter());
    itc.findAllInterlineCarrier(*ret);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in InterlineCarrierHistoricalDAO::create");
    throw;
  }

  return ret;
}

void
InterlineCarrierHistoricalDAO::destroy(CarrierCode key, std::vector<InterlineCarrierInfo*>* recs)
{
  std::vector<InterlineCarrierInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
InterlineCarrierHistoricalDAO::_name("InterlineCarrierHistorical");
std::string
InterlineCarrierHistoricalDAO::_cacheClass("Common");
DAOHelper<InterlineCarrierHistoricalDAO>
InterlineCarrierHistoricalDAO::_helper(_name);
InterlineCarrierHistoricalDAO* InterlineCarrierHistoricalDAO::_instance = nullptr;

} // namespace tse
