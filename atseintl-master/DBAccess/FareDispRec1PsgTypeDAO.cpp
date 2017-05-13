//------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
//

#include "DBAccess/FareDispRec1PsgTypeDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareDispRec1PsgType.h"
#include "DBAccess/Queries/QueryGetFareDispRec1PsgType.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
FareDispRec1PsgTypeDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareDispRec1PsgTypeDAO"));

FareDispRec1PsgTypeDAO&
FareDispRec1PsgTypeDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareDispRec1PsgType*>&
getFareDispRec1PsgTypeData(const Indicator& userApplType,
                           const UserApplCode& userAppl,
                           const Indicator& pseudoCityType,
                           const PseudoCityCode& pseudoCity,
                           const InclusionCode& inclusionCode,
                           DeleteList& deleteList,
                           const DateTime& ticketDate,
                           bool isHistorical)
{
  if (isHistorical)
  {
    FareDispRec1PsgTypeHistoricalDAO& dao = FareDispRec1PsgTypeHistoricalDAO::instance();
    return dao.get(
        deleteList, userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, ticketDate);
  }
  else
  {
    FareDispRec1PsgTypeDAO& dao = FareDispRec1PsgTypeDAO::instance();
    return dao.get(
        deleteList, userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, ticketDate);
  }
}

const std::vector<FareDispRec1PsgType*>&
FareDispRec1PsgTypeDAO::get(DeleteList& del,
                            const Indicator& userApplType,
                            const UserApplCode& userAppl,
                            const Indicator& pseudoCityType,
                            const PseudoCityCode& pseudoCity,
                            const InclusionCode& inclusionCode,
                            const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareDispRec1PsgTypeKey key(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  return *ptr;
}

FareDispRec1PsgTypeKey
FareDispRec1PsgTypeDAO::createKey(FareDispRec1PsgType* info)
{
  return FareDispRec1PsgTypeKey(info->userApplType(),
                                info->userAppl(),
                                info->pseudoCityType(),
                                info->pseudoCity(),
                                info->inclusionCode());
}

std::vector<FareDispRec1PsgType*>*
FareDispRec1PsgTypeDAO::create(FareDispRec1PsgTypeKey key)
{
  std::vector<FareDispRec1PsgType*>* ret = new std::vector<FareDispRec1PsgType*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareDispRec1PsgType r1pt(dbAdapter->getAdapter());
    r1pt.findFareDispRec1PsgType(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareDispRec1PsgTypeDAO::create");
    throw;
  }

  return ret;
}

void
FareDispRec1PsgTypeDAO::destroy(FareDispRec1PsgTypeKey key, std::vector<FareDispRec1PsgType*>* recs)
{
  destroyContainer(recs);
}

void
FareDispRec1PsgTypeDAO::load()
{
  StartupLoader<QueryGetAllFareDispRec1PsgType, FareDispRec1PsgType, FareDispRec1PsgTypeDAO>();
}

std::string
FareDispRec1PsgTypeDAO::_name("FareDispRec1PsgType");
std::string
FareDispRec1PsgTypeDAO::_cacheClass("FareDisplay");

DAOHelper<FareDispRec1PsgTypeDAO>
FareDispRec1PsgTypeDAO::_helper(_name);

FareDispRec1PsgTypeDAO* FareDispRec1PsgTypeDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: FareDispRec1PsgTypeHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
FareDispRec1PsgTypeHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareDispRec1PsgTypeHistoricalDAO"));
FareDispRec1PsgTypeHistoricalDAO&
FareDispRec1PsgTypeHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareDispRec1PsgType*>&
FareDispRec1PsgTypeHistoricalDAO::get(DeleteList& del,
                                      const Indicator& userApplType,
                                      const UserApplCode& userAppl,
                                      const Indicator& pseudoCityType,
                                      const PseudoCityCode& pseudoCity,
                                      const InclusionCode& inclusionCode,
                                      const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareDispRec1PsgTypeKey key(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  return *ptr;
}

std::vector<FareDispRec1PsgType*>*
FareDispRec1PsgTypeHistoricalDAO::create(FareDispRec1PsgTypeKey key)
{
  std::vector<FareDispRec1PsgType*>* ret = new std::vector<FareDispRec1PsgType*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareDispRec1PsgType r1pt(dbAdapter->getAdapter());
    r1pt.findFareDispRec1PsgType(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareDispRec1PsgTypeHistoricalDAO::create");
    throw;
  }

  return ret;
}

void
FareDispRec1PsgTypeHistoricalDAO::destroy(FareDispRec1PsgTypeKey key,
                                          std::vector<FareDispRec1PsgType*>* recs)
{
  destroyContainer(recs);
}

std::string
FareDispRec1PsgTypeHistoricalDAO::_name("FareDispRec1PsgTypeHistorical");
std::string
FareDispRec1PsgTypeHistoricalDAO::_cacheClass("FareDisplay");
DAOHelper<FareDispRec1PsgTypeHistoricalDAO>
FareDispRec1PsgTypeHistoricalDAO::_helper(_name);

FareDispRec1PsgTypeHistoricalDAO* FareDispRec1PsgTypeHistoricalDAO::_instance = nullptr;

} // namespace tse
