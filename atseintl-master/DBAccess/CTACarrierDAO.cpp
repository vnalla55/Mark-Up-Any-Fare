//-------------------------------------------------------------------------------
// Copyright 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
//-------------------------------------------------------------------------------
#include "DBAccess/CTACarrierDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/Queries/QueryGetCTACarrier.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
CTACarrierDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.CTACarrierDAO"));

CTACarrierDAO&
CTACarrierDAO::instance()
{
  if (_instance == nullptr)
    _helper.init();

  return *_instance;
}

bool
getCTACarrierData(const CarrierCode& carrier, const DateTime& ticketDate, bool isHistorical)
{
  if (isHistorical)
  {
    CTACarrierHistoricalDAO& dao = CTACarrierHistoricalDAO::instance();
    return dao.get(carrier, ticketDate);
  }
  else
  {
    CTACarrierDAO& dao = CTACarrierDAO::instance();
    return dao.get(carrier);
  }
}

bool
CTACarrierDAO::get(const CarrierCode& carrier)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  CTACarrierKey key(carrier);
  DAOCache::pointer_type ptr = cache().get(key);

  return !ptr->empty();
}

std::vector<CTACarrier*>*
CTACarrierDAO::create(CTACarrierKey key)
{
  std::vector<CTACarrier*>* ret = new std::vector<CTACarrier*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());

  try
  {
    QueryGetCTACarrier cf(dbAdapter->getAdapter());
    cf.findCarrier(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CTACarrierDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

void
CTACarrierDAO::destroy(CTACarrierKey key, std::vector<CTACarrier*>* recs)
{
  for (auto& rec : *recs)
    delete rec;

  delete recs;
}

CTACarrierKey
CTACarrierDAO::createKey(const CTACarrier* info)
{
  return CTACarrierKey(info->carrier());
}

void
CTACarrierDAO::load()
{
  StartupLoaderNoDB<CTACarrier, CTACarrierDAO>();
}

std::string
CTACarrierDAO::_name("CTACarrier");
std::string
CTACarrierDAO::_cacheClass("Common");
DAOHelper<CTACarrierDAO>
CTACarrierDAO::_helper(_name);
CTACarrierDAO* CTACarrierDAO::_instance = nullptr;

log4cxx::LoggerPtr
CTACarrierHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.CTACarrierHistoricalDAO"));

CTACarrierHistoricalDAO&
CTACarrierHistoricalDAO::instance()
{
  if (_instance == nullptr)
    _helper.init();

  return *_instance;
}

bool
CTACarrierHistoricalDAO::get(const CarrierCode& carrier, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  CTACarrierHistoricalKey key(carrier);
  DAOCache::pointer_type ptr = cache().get(key);

  IsCurrentH<CTACarrier> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();

  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter))
      return true;
  }
  return false;
}

std::vector<CTACarrier*>*
CTACarrierHistoricalDAO::create(CTACarrierHistoricalKey key)
{
  std::vector<CTACarrier*>* ret = new std::vector<CTACarrier*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());

  try
  {
    QueryGetCTACarrierHistorical cf(dbAdapter->getAdapter());
    cf.findCarrier(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CTACarrierHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

void
CTACarrierHistoricalDAO::destroy(CTACarrierHistoricalKey key, std::vector<CTACarrier*>* recs)
{
  for (auto& rec : *recs)
    delete rec;

  delete recs;
}

std::string
CTACarrierHistoricalDAO::_name("CTACarrierHistorical");
std::string
CTACarrierHistoricalDAO::_cacheClass("Common");
DAOHelper<CTACarrierHistoricalDAO>
CTACarrierHistoricalDAO::_helper(_name);
CTACarrierHistoricalDAO* CTACarrierHistoricalDAO::_instance = nullptr;

} // namespace tse
