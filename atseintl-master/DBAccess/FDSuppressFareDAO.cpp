//----------------------------------------------------------------------------
//  File: FDSuppressFareDAO.cpp
//
//  Author: Partha Kumar Chakraborti
//
//  Copyright Sabre 2005
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#include "DBAccess/FDSuppressFareDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/Queries/QueryGetSuppressFares.h"

namespace tse
{
log4cxx::LoggerPtr
FDSuppressFareDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FDSuppressFareDAO"));

std::string
FDSuppressFareDAO::_name("FDSuppressFare");
std::string
FDSuppressFareDAO::_cacheClass("FareDisplay");

DAOHelper<FDSuppressFareDAO>
FDSuppressFareDAO::_helper(_name);

FDSuppressFareDAO* FDSuppressFareDAO::_instance = nullptr;

FDSuppressFareDAO&
FDSuppressFareDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

std::vector<const FDSuppressFare*>&
getSuppressFareListData(const PseudoCityCode& pseudoCityCode,
                        const Indicator pseudoCityType,
                        const TJRGroup& ssgGroupNo,
                        const CarrierCode& carrierCode,
                        const DateTime& travelDate,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical)
{
  if (isHistorical)
  {
    FDSuppressFareHistoricalDAO& dao = FDSuppressFareHistoricalDAO::instance();
    return dao.get(deleteList,
                   pseudoCityCode,
                   pseudoCityType,
                   ssgGroupNo,
                   carrierCode,
                   travelDate,
                   ticketDate);
  }
  else
  {
    FDSuppressFareDAO& dao = FDSuppressFareDAO::instance();
    return dao.get(deleteList,
                   pseudoCityCode,
                   pseudoCityType,
                   ssgGroupNo,
                   carrierCode,
                   travelDate,
                   ticketDate);
  }
}

std::vector<const FDSuppressFare*>&
FDSuppressFareDAO::get(DeleteList& del,
                       const PseudoCityCode& pseudoCityCode,
                       const Indicator pseudoCityType,
                       const TJRGroup& ssgGroupNo,
                       const CarrierCode& carrierCode,
                       const DateTime& date,
                       const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FDSuppressFareKey key(pseudoCityCode, pseudoCityType, ssgGroupNo, carrierCode);

  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  return *ptr;
}

std::vector<const FDSuppressFare*>*
FDSuppressFareDAO::create(FDSuppressFareKey key)
{
  std::vector<const FDSuppressFare*>* ret = new std::vector<const FDSuppressFare*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSuppressFarePccCc sfpc(dbAdapter->getAdapter());
    sfpc.findSuppressFares(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FDSuppressFareDAO::create()");
    throw;
  }

  return ret;
}

void
FDSuppressFareDAO::destroy(FDSuppressFareKey key, std::vector<const FDSuppressFare*>* t)
{
  try
  {
    std::vector<const FDSuppressFare*>::iterator i;
    for (i = t->begin(); i != t->end(); i++)
      delete *i; // lint !e605
    delete t;
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FDSuppressFareDAO::destroy");
    throw;
  }

  return;
}

FDSuppressFareKey
FDSuppressFareDAO::createKey(const FDSuppressFare* info)
{
  return FDSuppressFareKey(
      info->pseudoCityCode(), info->pseudoCityType(), info->ssgGroupNo(), info->carrier());
}

void
FDSuppressFareDAO::load()
{
  StartupLoaderNoDB<FDSuppressFare, FDSuppressFareDAO>();
}

// --------------------------------------------------
// Historical DAO: FDSuppressFareHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
FDSuppressFareHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FDSuppressFareHistoricalDAO"));
std::string
FDSuppressFareHistoricalDAO::_name("FDSuppressFareHistorical");
std::string
FDSuppressFareHistoricalDAO::_cacheClass("FareDisplay");
DAOHelper<FDSuppressFareHistoricalDAO>
FDSuppressFareHistoricalDAO::_helper(_name);

FDSuppressFareHistoricalDAO* FDSuppressFareHistoricalDAO::_instance = nullptr;

FDSuppressFareHistoricalDAO&
FDSuppressFareHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

std::vector<const FDSuppressFare*>&
FDSuppressFareHistoricalDAO::get(DeleteList& del,
                                 const PseudoCityCode& pseudoCityCode,
                                 const Indicator pseudoCityType,
                                 const TJRGroup& ssgGroupNo,
                                 const CarrierCode& carrierCode,
                                 const DateTime& date,
                                 const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FDSuppressFareKey key(pseudoCityCode, pseudoCityType, ssgGroupNo, carrierCode);

  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  return *ptr;
}

std::vector<const FDSuppressFare*>*
FDSuppressFareHistoricalDAO::create(FDSuppressFareKey key)
{
  std::vector<const FDSuppressFare*>* ret = new std::vector<const FDSuppressFare*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetSuppressFarePccCc sfpc(dbAdapter->getAdapter());
    sfpc.findSuppressFares(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FDSuppressFareHistoricalDAO::create()");
    throw;
  }

  return ret;
}

void
FDSuppressFareHistoricalDAO::destroy(FDSuppressFareKey key, std::vector<const FDSuppressFare*>* t)
{
  try
  {
    std::vector<const FDSuppressFare*>::iterator i;
    for (i = t->begin(); i != t->end(); i++)
      delete *i; // lint !e605
    delete t;
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FDSuppressFareHistoricalDAO::destroy");
    throw;
  }
  return;
}

FDSuppressFareKey
FDSuppressFareHistoricalDAO::createKey(const FDSuppressFare* info,
                                       const DateTime& startDate,
                                       const DateTime& endDate)
{
  return FDSuppressFareKey(
      info->pseudoCityCode(), info->pseudoCityType(), info->ssgGroupNo(), info->carrier());
}

void
FDSuppressFareHistoricalDAO::load()
{
  StartupLoaderNoDB<FDSuppressFare, FDSuppressFareHistoricalDAO>();
}
}
