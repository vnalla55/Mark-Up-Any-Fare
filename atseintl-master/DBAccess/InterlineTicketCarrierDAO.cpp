//----------------------------------------------------------------------------
//          File:           InterlineTicketCarrierDAO.cpp
//          Description:    InterlineTicketCarrierDAO
//          Created:        10/1/2010
//          Authors:        Anna Kulig
//
//          Updates:
//
//     (c)2010, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#include "DBAccess/InterlineTicketCarrierDAO.h"

#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/InterlineTicketCarrierInfo.h"
#include "DBAccess/Queries/QueryGetInterlineTicketCarrier.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
InterlineTicketCarrierDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.InterlineTicketCarrierDAO"));

InterlineTicketCarrierDAO&
InterlineTicketCarrierDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<InterlineTicketCarrierInfo*>&
getInterlineTicketCarrierData(const CarrierCode& carrier,
                              const DateTime& date,
                              DeleteList& deleteList,
                              const DateTime& ticketDate,
                              bool isHistorical)
{
  if (isHistorical)
  {
    InterlineTicketCarrierHistoricalDAO& dao = InterlineTicketCarrierHistoricalDAO::instance();
    return dao.get(deleteList, carrier, date, ticketDate);
  }
  else
  {
    InterlineTicketCarrierDAO& dao = InterlineTicketCarrierDAO::instance();
    return dao.get(deleteList, carrier, date, ticketDate);
  }
}

const std::vector<InterlineTicketCarrierInfo*>&
InterlineTicketCarrierDAO::get(DeleteList& del,
                               const CarrierCode& key,
                               const DateTime& date,
                               const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(CarrierKey(key));
  /*
  del.copy(ptr);
  std::vector<InterlineTicketCarrierInfo*>* ret = new std::vector<InterlineTicketCarrierInfo*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret),
  IsNotEffectiveG<InterlineTicketCarrierInfo>(date, ticketDate));
  return *ret;
  */
  return *(applyFilter(del, ptr, IsNotEffectiveG<InterlineTicketCarrierInfo>(date, ticketDate)));
}

void
InterlineTicketCarrierDAO::load()
{
  StartupLoader<QueryGetAllInterlineTicketCarrier,
                InterlineTicketCarrierInfo,
                InterlineTicketCarrierDAO>();
}

CarrierKey
InterlineTicketCarrierDAO::createKey(InterlineTicketCarrierInfo* info)
{
  return CarrierKey(info->carrier());
}

std::vector<InterlineTicketCarrierInfo*>*
InterlineTicketCarrierDAO::create(CarrierKey key)
{
  std::vector<InterlineTicketCarrierInfo*>* ret = new std::vector<InterlineTicketCarrierInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetInterlineTicketCarrier itc(dbAdapter->getAdapter());
    itc.findInterlineTicketCarrier(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in InterlineTicketCarrierDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
InterlineTicketCarrierDAO::destroy(CarrierKey key, std::vector<InterlineTicketCarrierInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
InterlineTicketCarrierDAO::_name("InterlineTicketCarrier");
std::string
InterlineTicketCarrierDAO::_cacheClass("Common");

DAOHelper<InterlineTicketCarrierDAO>
InterlineTicketCarrierDAO::_helper(_name);

InterlineTicketCarrierDAO* InterlineTicketCarrierDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: InterlineTicketCarrierHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
InterlineTicketCarrierHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.InterlineTicketCarrierHistoricalDAO"));
InterlineTicketCarrierHistoricalDAO&
InterlineTicketCarrierHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<InterlineTicketCarrierInfo*>&
InterlineTicketCarrierHistoricalDAO::get(DeleteList& del,
                                         const CarrierCode& key,
                                         const DateTime& date,
                                         const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<InterlineTicketCarrierInfo*>* ret = new std::vector<InterlineTicketCarrierInfo*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveHist<InterlineTicketCarrierInfo>(date, ticketDate));
  return *ret;
}

struct InterlineTicketCarrierHistoricalDAO::groupByKey
{
public:
  CarrierCode prevKey;

  DAOCache& cache;

  groupByKey()
    : prevKey(INVALID_CARRIERCODE),
      cache(InterlineTicketCarrierHistoricalDAO::instance().cache()),
      ptr(nullptr)
  {
  }

  std::vector<InterlineTicketCarrierInfo*>* ptr;

  void operator()(InterlineTicketCarrierInfo* info)
  {
    if (info->carrier() != prevKey)
    {
      ptr = new std::vector<InterlineTicketCarrierInfo*>;
      cache.put(info->carrier(), ptr);
      prevKey = info->carrier();
    }
    ptr->push_back(info);
  }
};

void
InterlineTicketCarrierHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<InterlineTicketCarrierInfo*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllInterlineTicketCarrierHistorical itc(dbAdapter->getAdapter());
    itc.findAllInterlineTicketCarrier(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in InterlineTicketCarrierHistoricalDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::for_each(recs.begin(), recs.end(), groupByKey());
}

std::vector<InterlineTicketCarrierInfo*>*
InterlineTicketCarrierHistoricalDAO::create(CarrierCode key)
{
  std::vector<InterlineTicketCarrierInfo*>* ret = new std::vector<InterlineTicketCarrierInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetInterlineTicketCarrierHistorical itc(dbAdapter->getAdapter());
    itc.findInterlineTicketCarrier(*ret, key);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in InterlineTicketCarrierHistoricalDAO::create");
    throw;
  }

  return ret;
}

void
InterlineTicketCarrierHistoricalDAO::destroy(CarrierCode key,
                                             std::vector<InterlineTicketCarrierInfo*>* recs)
{
  std::vector<InterlineTicketCarrierInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
InterlineTicketCarrierHistoricalDAO::_name("InterlineTicketCarrierHistorical");
std::string
InterlineTicketCarrierHistoricalDAO::_cacheClass("Common");
DAOHelper<InterlineTicketCarrierHistoricalDAO>
InterlineTicketCarrierHistoricalDAO::_helper(_name);
InterlineTicketCarrierHistoricalDAO* InterlineTicketCarrierHistoricalDAO::_instance = nullptr;

} // namespace tse
