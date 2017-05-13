//------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
//

#include "DBAccess/FareDispInclRuleTrfDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareDispInclRuleTrf.h"
#include "DBAccess/Queries/QueryGetFareDispInclRuleTrf.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
FareDispInclRuleTrfDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareDispInclRuleTrfDAO"));

FareDispInclRuleTrfDAO&
FareDispInclRuleTrfDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareDispInclRuleTrf*>&
getFareDispInclRuleTrfData(const Indicator& userApplType,
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
    FareDispInclRuleTrfHistoricalDAO& dao = FareDispInclRuleTrfHistoricalDAO::instance();
    return dao.get(
        deleteList, userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, ticketDate);
  }
  else
  {
    FareDispInclRuleTrfDAO& dao = FareDispInclRuleTrfDAO::instance();
    return dao.get(
        deleteList, userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, ticketDate);
  }
}

const std::vector<FareDispInclRuleTrf*>&
FareDispInclRuleTrfDAO::get(DeleteList& del,
                            const Indicator& userApplType,
                            const UserApplCode& userAppl,
                            const Indicator& pseudoCityType,
                            const PseudoCityCode& pseudoCity,
                            const InclusionCode& inclusionCode,
                            const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareDispInclRuleTrfKey key(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  return *ptr;
}

FareDispInclRuleTrfKey
FareDispInclRuleTrfDAO::createKey(FareDispInclRuleTrf* info)
{
  return FareDispInclRuleTrfKey(info->userApplType(),
                                info->userAppl(),
                                info->pseudoCityType(),
                                info->pseudoCity(),
                                info->inclusionCode());
}

std::vector<FareDispInclRuleTrf*>*
FareDispInclRuleTrfDAO::create(FareDispInclRuleTrfKey key)
{
  std::vector<FareDispInclRuleTrf*>* ret = new std::vector<FareDispInclRuleTrf*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareDispInclRuleTrf irt(dbAdapter->getAdapter());
    irt.findFareDispInclRuleTrf(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareDispInclRuleTrfDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FareDispInclRuleTrfDAO::destroy(FareDispInclRuleTrfKey key, std::vector<FareDispInclRuleTrf*>* recs)
{
  destroyContainer(recs);
}

void
FareDispInclRuleTrfDAO::load()
{
  StartupLoader<QueryGetAllFareDispInclRuleTrf, FareDispInclRuleTrf, FareDispInclRuleTrfDAO>();
}

std::string
FareDispInclRuleTrfDAO::_name("FareDispInclRuleTrf");
std::string
FareDispInclRuleTrfDAO::_cacheClass("FareDisplay");

DAOHelper<FareDispInclRuleTrfDAO>
FareDispInclRuleTrfDAO::_helper(_name);

FareDispInclRuleTrfDAO* FareDispInclRuleTrfDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: FareDispInclRuleTrfHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
FareDispInclRuleTrfHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareDispInclRuleTrfHistoricalDAO"));
FareDispInclRuleTrfHistoricalDAO&
FareDispInclRuleTrfHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareDispInclRuleTrf*>&
FareDispInclRuleTrfHistoricalDAO::get(DeleteList& del,
                                      const Indicator& userApplType,
                                      const UserApplCode& userAppl,
                                      const Indicator& pseudoCityType,
                                      const PseudoCityCode& pseudoCity,
                                      const InclusionCode& inclusionCode,
                                      const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareDispInclRuleTrfKey key(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  return *ptr;
}

std::vector<FareDispInclRuleTrf*>*
FareDispInclRuleTrfHistoricalDAO::create(FareDispInclRuleTrfKey key)
{
  std::vector<FareDispInclRuleTrf*>* ret = new std::vector<FareDispInclRuleTrf*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareDispInclRuleTrf irt(dbAdapter->getAdapter());
    irt.findFareDispInclRuleTrf(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareDispInclRuleTrfHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FareDispInclRuleTrfHistoricalDAO::destroy(FareDispInclRuleTrfKey key,
                                          std::vector<FareDispInclRuleTrf*>* recs)
{
  destroyContainer(recs);
}

std::string
FareDispInclRuleTrfHistoricalDAO::_name("FareDispInclRuleTrfHistorical");
std::string
FareDispInclRuleTrfHistoricalDAO::_cacheClass("FareDisplay");
DAOHelper<FareDispInclRuleTrfHistoricalDAO>
FareDispInclRuleTrfHistoricalDAO::_helper(_name);

FareDispInclRuleTrfHistoricalDAO* FareDispInclRuleTrfHistoricalDAO::_instance = nullptr;

} // namespace tse
