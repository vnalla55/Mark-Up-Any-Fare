//-------------------------------------------------------------------------------
// Copyright 2007, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/FreeBaggageDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FreeBaggageInfo.h"
#include "DBAccess/Queries/QueryGetFreeBaggage.h"

namespace tse
{
log4cxx::LoggerPtr
FreeBaggageDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FreeBaggageDAO"));
FreeBaggageDAO&
FreeBaggageDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FreeBaggageInfo*>&
FreeBaggageDAO::get(DeleteList& del, const CarrierCode& key)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(CarrierKey(key));
  del.copy(ptr);
  return *ptr;
}

std::vector<FreeBaggageInfo*>*
FreeBaggageDAO::create(CarrierKey key)
{
  std::vector<FreeBaggageInfo*>* ret = new std::vector<FreeBaggageInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFreeBaggage bag(dbAdapter->getAdapter());
    bag.findFreeBaggage(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FreeBaggageDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FreeBaggageDAO::destroy(CarrierKey key, std::vector<FreeBaggageInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
FreeBaggageDAO::_name("FreeBaggage");
std::string
FreeBaggageDAO::_cacheClass("Common");
DAOHelper<FreeBaggageDAO>
FreeBaggageDAO::_helper(_name);
FreeBaggageDAO* FreeBaggageDAO::_instance = nullptr;

CarrierKey
FreeBaggageDAO::createKey(const FreeBaggageInfo* info)
{
  return CarrierKey(info->carrier());
}

void
FreeBaggageDAO::load()
{
  StartupLoaderNoDB<FreeBaggageInfo, FreeBaggageDAO>();
}

// --------------------------------------------------
// Historical DAO: FreeBaggageHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
FreeBaggageHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FreeBaggageHistoricalDAO"));
FreeBaggageHistoricalDAO&
FreeBaggageHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FreeBaggageInfo*>&
FreeBaggageHistoricalDAO::get(DeleteList& del,
                              const CarrierCode& carrier,
                              const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FreeBaggageHistoricalKey key(carrier);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<FreeBaggageInfo*>* ret = new std::vector<FreeBaggageInfo*>;
  del.adopt(ret);
  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotCurrentH<FreeBaggageInfo>(ticketDate));

  return *ret;
}

std::vector<FreeBaggageInfo*>*
FreeBaggageHistoricalDAO::create(FreeBaggageHistoricalKey key)
{
  std::vector<FreeBaggageInfo*>* ret = new std::vector<FreeBaggageInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFreeBaggageHistorical bag(dbAdapter->getAdapter());
    bag.findFreeBaggage(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FreeBaggageHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FreeBaggageHistoricalDAO::destroy(FreeBaggageHistoricalKey key, std::vector<FreeBaggageInfo*>* recs)
{
  std::vector<FreeBaggageInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
  {
    delete *i;
  }
  delete recs;
}

std::string
FreeBaggageHistoricalDAO::_name("FreeBaggageHistorical");
std::string
FreeBaggageHistoricalDAO::_cacheClass("Common");
DAOHelper<FreeBaggageHistoricalDAO>
FreeBaggageHistoricalDAO::_helper(_name);
FreeBaggageHistoricalDAO* FreeBaggageHistoricalDAO::_instance = nullptr;

} // namespace tse
