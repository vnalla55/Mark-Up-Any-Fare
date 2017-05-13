//-------------------------------------------------------------------------------
// Copyright 2012, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
//-------------------------------------------------------------------------------
#include "DBAccess/USDotCarrierDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/Queries/QueryGetUSDotCarrier.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
USDotCarrierDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.USDotCarrierDAO"));

USDotCarrierDAO&
USDotCarrierDAO::instance()
{
  if (_instance == nullptr)
    _helper.init();

  return *_instance;
}

bool
getUSDotCarrierData(const CarrierCode& carrier, const DateTime& ticketDate, bool isHistorical)
{
  if (isHistorical)
  {
    USDotCarrierHistoricalDAO& dao = USDotCarrierHistoricalDAO::instance();
    return dao.get(carrier, ticketDate);
  }
  else
  {
    USDotCarrierDAO& dao = USDotCarrierDAO::instance();
    return dao.get(carrier);
  }
}

bool
USDotCarrierDAO::get(const CarrierCode& carrier)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  USDotCarrierKey key(carrier);
  DAOCache::pointer_type ptr = cache().get(key);

  return !ptr->empty();
}

std::vector<USDotCarrier*>*
USDotCarrierDAO::create(USDotCarrierKey key)
{
  std::vector<USDotCarrier*>* ret = new std::vector<USDotCarrier*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());

  try
  {
    QueryGetUSDotCarrier cf(dbAdapter->getAdapter());
    cf.findCarrier(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in USDotCarrierDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

void
USDotCarrierDAO::destroy(USDotCarrierKey key, std::vector<USDotCarrier*>* recs)
{
  std::vector<USDotCarrier*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

USDotCarrierKey
USDotCarrierDAO::createKey(const USDotCarrier* info)
{
  return USDotCarrierKey(info->carrier());
}

void
USDotCarrierDAO::load()
{
  StartupLoaderNoDB<USDotCarrier, USDotCarrierDAO>();
}

std::string
USDotCarrierDAO::_name("USDotCarrier");
std::string
USDotCarrierDAO::_cacheClass("Common");
DAOHelper<USDotCarrierDAO>
USDotCarrierDAO::_helper(_name);
USDotCarrierDAO* USDotCarrierDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: USDotCarrierHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
USDotCarrierHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.USDotCarrierHistoricalDAO"));

USDotCarrierHistoricalDAO&
USDotCarrierHistoricalDAO::instance()
{
  if (_instance == nullptr)
    _helper.init();

  return *_instance;
}

bool
USDotCarrierHistoricalDAO::get(const CarrierCode& carrier, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  USDotCarrierHistoricalKey key(carrier);
  DAOCache::pointer_type ptr = cache().get(key);

  IsCurrentH<USDotCarrier> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();

  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter))
      return true;
  }
  return false;
}

std::vector<USDotCarrier*>*
USDotCarrierHistoricalDAO::create(USDotCarrierHistoricalKey key)
{
  std::vector<USDotCarrier*>* ret = new std::vector<USDotCarrier*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());

  try
  {
    QueryGetUSDotCarrierHistorical cf(dbAdapter->getAdapter());
    cf.findCarrier(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in USDotCarrierHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

void
USDotCarrierHistoricalDAO::destroy(USDotCarrierHistoricalKey key, std::vector<USDotCarrier*>* recs)
{
  std::vector<USDotCarrier*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
USDotCarrierHistoricalDAO::_name("USDotCarrierHistorical");
std::string
USDotCarrierHistoricalDAO::_cacheClass("Common");
DAOHelper<USDotCarrierHistoricalDAO>
USDotCarrierHistoricalDAO::_helper(_name);
USDotCarrierHistoricalDAO* USDotCarrierHistoricalDAO::_instance = nullptr;

} // namespace tse
