//-------------------------------------------------------------------------------
// Copyright 2007, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/FreeBaggageNotExpiredDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FreeBaggageInfo.h"
#include "DBAccess/Queries/QueryGetFreeBaggageNotExpired.h"

namespace tse
{
log4cxx::LoggerPtr
FreeBaggageNotExpiredDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FreeBaggageNotExpiredDAO"));
FreeBaggageNotExpiredDAO&
FreeBaggageNotExpiredDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FreeBaggageInfo*>&
FreeBaggageNotExpiredDAO::get(DeleteList& del, const CarrierCode& carrier, DateTime& reqDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  CarrierWithDateKey key2(carrier, reqDate);
  DAOCache::pointer_type ptr = cache().get(key2);
  del.copy(ptr);
  return *ptr;
}

std::vector<FreeBaggageInfo*>*
FreeBaggageNotExpiredDAO::create(CarrierWithDateKey key)
{
  std::vector<FreeBaggageInfo*>* ret = new std::vector<FreeBaggageInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFreeBaggageNotExpired bag(dbAdapter->getAdapter());
    bag.findFreeBaggage(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FreeBaggageNotExpiredDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FreeBaggageNotExpiredDAO::destroy(CarrierWithDateKey key, std::vector<FreeBaggageInfo*>* recs)
{
  std::vector<FreeBaggageInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
  {
    delete *i;
  }
  delete recs;
}

std::string
FreeBaggageNotExpiredDAO::_name("FreeBaggageNotExpired");
std::string
FreeBaggageNotExpiredDAO::_cacheClass("Common");
DAOHelper<FreeBaggageNotExpiredDAO>
FreeBaggageNotExpiredDAO::_helper(_name);
FreeBaggageNotExpiredDAO* FreeBaggageNotExpiredDAO::_instance = nullptr;

CarrierWithDateKey
FreeBaggageNotExpiredDAO::createKey(const FreeBaggageInfo* info)
{
  return CarrierWithDateKey(info->carrier(), info->expireDate());
}

void
FreeBaggageNotExpiredDAO::load()
{
  StartupLoaderNoDB<FreeBaggageInfo, FreeBaggageNotExpiredDAO>();
}

// --------------------------------------------------
// Historical DAO: FreeBaggageNotExpiredHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
FreeBaggageNotExpiredHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FreeBaggageNotExpiredHistoricalDAO"));
FreeBaggageNotExpiredHistoricalDAO&
FreeBaggageNotExpiredHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FreeBaggageInfo*>&
FreeBaggageNotExpiredHistoricalDAO::get(DeleteList& del,
                                        const CarrierCode& carrier,
                                        DateTime& reqDate)
{ // Shyam sez that reqDate is essentially the ticketDate. tvlDate is apparently not used so use
  // IsCurrentH w/reqDate

  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FreeBaggageNotExpiredHistoricalKey key(carrier);
  DAOUtils::getDateRange(reqDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<FreeBaggageInfo*>* ret = new std::vector<FreeBaggageInfo*>;
  del.adopt(ret);
  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotCurrentH<FreeBaggageInfo>(reqDate));

  return *ret;
}

std::vector<FreeBaggageInfo*>*
FreeBaggageNotExpiredHistoricalDAO::create(FreeBaggageNotExpiredHistoricalKey key)
{
  std::vector<FreeBaggageInfo*>* ret = new std::vector<FreeBaggageInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFreeBaggageNotExpiredHistorical bag(dbAdapter->getAdapter());
    bag.findFreeBaggage(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FreeBaggageNotExpiredHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FreeBaggageNotExpiredHistoricalDAO::destroy(FreeBaggageNotExpiredHistoricalKey key,
                                            std::vector<FreeBaggageInfo*>* recs)
{
  std::vector<FreeBaggageInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
  {
    delete *i;
  }
  delete recs;
}

std::string
FreeBaggageNotExpiredHistoricalDAO::_name("FreeBaggageNotExpiredHistorical");
std::string
FreeBaggageNotExpiredHistoricalDAO::_cacheClass("Common");
DAOHelper<FreeBaggageNotExpiredHistoricalDAO>
FreeBaggageNotExpiredHistoricalDAO::_helper(_name);
FreeBaggageNotExpiredHistoricalDAO* FreeBaggageNotExpiredHistoricalDAO::_instance = nullptr;

} // namespace tse
