//------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
//

#include "DBAccess/FareDispInclFareTypeDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareDispInclFareType.h"
#include "DBAccess/Queries/QueryGetFareDispInclFareType.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
FareDispInclFareTypeDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareDispInclFareTypeDAO"));

FareDispInclFareTypeDAO&
FareDispInclFareTypeDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareDispInclFareType*>&
getFareDispInclFareTypeData(const Indicator& userApplType,
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
    FareDispInclFareTypeHistoricalDAO& dao = FareDispInclFareTypeHistoricalDAO::instance();
    return dao.get(
        deleteList, userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, ticketDate);
  }
  else
  {
    FareDispInclFareTypeDAO& dao = FareDispInclFareTypeDAO::instance();
    return dao.get(
        deleteList, userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, ticketDate);
  }
}

const std::vector<FareDispInclFareType*>&
FareDispInclFareTypeDAO::get(DeleteList& del,
                             const Indicator& userApplType,
                             const UserApplCode& userAppl,
                             const Indicator& pseudoCityType,
                             const PseudoCityCode& pseudoCity,
                             const InclusionCode& inclusionCode,
                             const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareDispInclFareTypeKey key(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  return *ptr;
}

FareDispInclFareTypeKey
FareDispInclFareTypeDAO::createKey(FareDispInclFareType* info)
{
  return FareDispInclFareTypeKey(info->userApplType(),
                                 info->userAppl(),
                                 info->pseudoCityType(),
                                 info->pseudoCity(),
                                 info->inclusionCode());
}

std::vector<FareDispInclFareType*>*
FareDispInclFareTypeDAO::create(FareDispInclFareTypeKey key)
{
  std::vector<FareDispInclFareType*>* ret = new std::vector<FareDispInclFareType*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareDispInclFareType ift(dbAdapter->getAdapter());
    ift.findFareDispInclFareType(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareDispInclFareTypeDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FareDispInclFareTypeDAO::destroy(FareDispInclFareTypeKey key,
                                 std::vector<FareDispInclFareType*>* recs)
{
  destroyContainer(recs);
}

void
FareDispInclFareTypeDAO::load()
{
  StartupLoader<QueryGetAllFareDispInclFareType, FareDispInclFareType, FareDispInclFareTypeDAO>();
}

std::string
FareDispInclFareTypeDAO::_name("FareDispInclFareType");
std::string
FareDispInclFareTypeDAO::_cacheClass("FareDisplay");

DAOHelper<FareDispInclFareTypeDAO>
FareDispInclFareTypeDAO::_helper(_name);

FareDispInclFareTypeDAO* FareDispInclFareTypeDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: FareDispInclFareTypeHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
FareDispInclFareTypeHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareDispInclFareTypeHistoricalDAO"));
FareDispInclFareTypeHistoricalDAO&
FareDispInclFareTypeHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareDispInclFareType*>&
FareDispInclFareTypeHistoricalDAO::get(DeleteList& del,
                                       const Indicator& userApplType,
                                       const UserApplCode& userAppl,
                                       const Indicator& pseudoCityType,
                                       const PseudoCityCode& pseudoCity,
                                       const InclusionCode& inclusionCode,
                                       const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareDispInclFareTypeKey key(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  return *ptr;
}

std::vector<FareDispInclFareType*>*
FareDispInclFareTypeHistoricalDAO::create(FareDispInclFareTypeKey key)
{
  std::vector<FareDispInclFareType*>* ret = new std::vector<FareDispInclFareType*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareDispInclFareType ift(dbAdapter->getAdapter());
    ift.findFareDispInclFareType(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareDispInclFareTypeHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FareDispInclFareTypeHistoricalDAO::destroy(FareDispInclFareTypeKey key,
                                           std::vector<FareDispInclFareType*>* recs)
{
  destroyContainer(recs);
}

std::string
FareDispInclFareTypeHistoricalDAO::_name("FareDispInclFareTypeHistorical");
std::string
FareDispInclFareTypeHistoricalDAO::_cacheClass("FareDisplay");
DAOHelper<FareDispInclFareTypeHistoricalDAO>
FareDispInclFareTypeHistoricalDAO::_helper(_name);

FareDispInclFareTypeHistoricalDAO* FareDispInclFareTypeHistoricalDAO::_instance = nullptr;

} // namespace tse
