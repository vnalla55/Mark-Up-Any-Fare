//------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
//

#include "DBAccess/FareDispRec8PsgTypeDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareDispRec8PsgType.h"
#include "DBAccess/Queries/QueryGetFareDispRec8PsgType.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
FareDispRec8PsgTypeDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareDispRec8PsgTypeDAO"));

FareDispRec8PsgTypeDAO&
FareDispRec8PsgTypeDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareDispRec8PsgType*>&
getFareDispRec8PsgTypeData(const Indicator& userApplType,
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
    FareDispRec8PsgTypeHistoricalDAO& dao = FareDispRec8PsgTypeHistoricalDAO::instance();
    return dao.get(
        deleteList, userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, ticketDate);
  }
  else
  {
    FareDispRec8PsgTypeDAO& dao = FareDispRec8PsgTypeDAO::instance();
    return dao.get(
        deleteList, userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, ticketDate);
  }
}

const std::vector<FareDispRec8PsgType*>&
FareDispRec8PsgTypeDAO::get(DeleteList& del,
                            const Indicator& userApplType,
                            const UserApplCode& userAppl,
                            const Indicator& pseudoCityType,
                            const PseudoCityCode& pseudoCity,
                            const InclusionCode& inclusionCode,
                            const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareDispRec8PsgTypeKey key(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  return *ptr;
}

FareDispRec8PsgTypeKey
FareDispRec8PsgTypeDAO::createKey(FareDispRec8PsgType* info)
{
  return FareDispRec8PsgTypeKey(info->userApplType(),
                                info->userAppl(),
                                info->pseudoCityType(),
                                info->pseudoCity(),
                                info->inclusionCode());
}

std::vector<FareDispRec8PsgType*>*
FareDispRec8PsgTypeDAO::create(FareDispRec8PsgTypeKey key)
{
  std::vector<FareDispRec8PsgType*>* ret = new std::vector<FareDispRec8PsgType*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareDispRec8PsgType r8pt(dbAdapter->getAdapter());
    r8pt.findFareDispRec8PsgType(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareDispRec8PsgTypeDAO::create");
    throw;
  }

  return ret;
}

void
FareDispRec8PsgTypeDAO::destroy(FareDispRec8PsgTypeKey key, std::vector<FareDispRec8PsgType*>* recs)
{
  destroyContainer(recs);
}

void
FareDispRec8PsgTypeDAO::load()
{
  StartupLoader<QueryGetAllFareDispRec8PsgType, FareDispRec8PsgType, FareDispRec8PsgTypeDAO>();
}

std::string
FareDispRec8PsgTypeDAO::_name("FareDispRec8PsgType");
std::string
FareDispRec8PsgTypeDAO::_cacheClass("FareDisplay");

DAOHelper<FareDispRec8PsgTypeDAO>
FareDispRec8PsgTypeDAO::_helper(_name);

FareDispRec8PsgTypeDAO* FareDispRec8PsgTypeDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: FareDispRec8PsgTypeHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
FareDispRec8PsgTypeHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareDispRec8PsgTypeHistoricalDAO"));
FareDispRec8PsgTypeHistoricalDAO&
FareDispRec8PsgTypeHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareDispRec8PsgType*>&
FareDispRec8PsgTypeHistoricalDAO::get(DeleteList& del,
                                      const Indicator& userApplType,
                                      const UserApplCode& userAppl,
                                      const Indicator& pseudoCityType,
                                      const PseudoCityCode& pseudoCity,
                                      const InclusionCode& inclusionCode,
                                      const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareDispRec8PsgTypeKey key(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  return *ptr;
}

std::vector<FareDispRec8PsgType*>*
FareDispRec8PsgTypeHistoricalDAO::create(FareDispRec8PsgTypeKey key)
{
  std::vector<FareDispRec8PsgType*>* ret = new std::vector<FareDispRec8PsgType*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareDispRec8PsgType r8pt(dbAdapter->getAdapter());
    r8pt.findFareDispRec8PsgType(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareDispRec8PsgTypeHistoricalDAO::create");
    throw;
  }

  return ret;
}

void
FareDispRec8PsgTypeHistoricalDAO::destroy(FareDispRec8PsgTypeKey key,
                                          std::vector<FareDispRec8PsgType*>* recs)
{
  destroyContainer(recs);
}

std::string
FareDispRec8PsgTypeHistoricalDAO::_name("FareDispRec8PsgTypeHistorical");
std::string
FareDispRec8PsgTypeHistoricalDAO::_cacheClass("FareDisplay");
DAOHelper<FareDispRec8PsgTypeHistoricalDAO>
FareDispRec8PsgTypeHistoricalDAO::_helper(_name);

FareDispRec8PsgTypeHistoricalDAO* FareDispRec8PsgTypeHistoricalDAO::_instance = nullptr;

} // namespace tse
