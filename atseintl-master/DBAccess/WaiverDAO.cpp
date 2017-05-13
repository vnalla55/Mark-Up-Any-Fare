//-------------------------------------------------------------------------------
// @ 2007, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/WaiverDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetTable987.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"
#include "DBAccess/Waiver.h"

namespace tse
{
log4cxx::LoggerPtr
WaiverDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.WaiverDAO"));
WaiverDAO&
WaiverDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<Waiver*>&
getWaiverData(const VendorCode& vendor,
              int itemNo,
              DeleteList& deleteList,
              const DateTime& ticketDate,
              bool isHistorical,
              const DateTime& applDate)
{
  if (isHistorical)
  {
    WaiverHistoricalDAO& dao = WaiverHistoricalDAO::instance();
    if (applDate != DateTime::emptyDate())
    {
      std::vector<Waiver*>& ret = dao.get(deleteList, vendor, itemNo, applDate);
      if (ret.size() > 0)
        return ret;
    }
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    WaiverDAO& dao = WaiverDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

std::vector<Waiver*>&
WaiverDAO::get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  WaiverKey key(vendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<Waiver*>* ret = new std::vector<Waiver*>;
  del.adopt(ret);

  IsCurrentG<Waiver> isCurrent(ticketDate);
  std::vector<Waiver*>::const_iterator i = ptr->begin();
  std::vector<Waiver*>::const_iterator ie = ptr->end();
  for (; i != ie; i++)
  {
    if (isCurrent(*i) && NotInhibit<Waiver>(*i))
      ret->push_back(*i);
  }
  return *ret;
}

std::vector<Waiver*>*
WaiverDAO::create(WaiverKey key)
{
  std::vector<Waiver*>* ret = new std::vector<Waiver*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTable987 query(dbAdapter->getAdapter());
    query.findWaiver(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in WaiverDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
WaiverDAO::destroy(WaiverKey key, std::vector<Waiver*>* recs)
{
  std::vector<Waiver*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
WaiverDAO::_name("Waiver");
std::string
WaiverDAO::_cacheClass("Rules");
DAOHelper<WaiverDAO>
WaiverDAO::_helper(_name);
WaiverDAO* WaiverDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: WaiverHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
WaiverHistoricalDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.WaiverHistoricalDAO"));
WaiverHistoricalDAO&
WaiverHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

std::vector<Waiver*>&
WaiverHistoricalDAO::get(DeleteList& del,
                         const VendorCode& vendor,
                         int itemNo,
                         const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  WaiverKey key(vendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<Waiver*>* ret = new std::vector<Waiver*>;
  del.adopt(ret);

  IsCurrentH<Waiver> isCurrent(ticketDate);
  std::vector<Waiver*>::const_iterator i = ptr->begin();
  std::vector<Waiver*>::const_iterator ie = ptr->end();
  for (; i != ie; i++)
  {
    if (isCurrent(*i) && NotInhibit<Waiver>(*i))
      ret->push_back(*i);
  }
  return *ret;
}

struct WaiverHistoricalDAO::groupByKey
{
public:
  WaiverKey prevKey;

  DAOCache& cache;

  groupByKey() : cache(WaiverHistoricalDAO::instance().cache()), ptr(nullptr) {}

  std::vector<Waiver*>* ptr;

  void operator()(Waiver* info)
  {
    WaiverKey key(info->vendor(), info->itemNo());
    if (!(key == prevKey))
    {
      ptr = new std::vector<Waiver*>;
      cache.put(key, ptr);
      prevKey = key;
    }
    ptr->push_back(info);
  }
};

void
WaiverHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<Waiver*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllTable987Historical ssb(dbAdapter->getAdapter());
    ssb.findAllWaivers(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in WaiverHistoricalDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::for_each(recs.begin(), recs.end(), groupByKey());
}

std::vector<Waiver*>*
WaiverHistoricalDAO::create(WaiverKey key)
{
  std::vector<Waiver*>* ret = new std::vector<Waiver*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTable987Historical query(dbAdapter->getAdapter());
    query.findWaiver(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in WaiverHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
WaiverHistoricalDAO::destroy(WaiverKey key, std::vector<Waiver*>* recs)
{
  std::vector<Waiver*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
WaiverHistoricalDAO::_name("WaiverHistorical");
std::string
WaiverHistoricalDAO::_cacheClass("Rules");
DAOHelper<WaiverHistoricalDAO>
WaiverHistoricalDAO::_helper(_name);
WaiverHistoricalDAO* WaiverHistoricalDAO::_instance = nullptr;

} // namespace tse
