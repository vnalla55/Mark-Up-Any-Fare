//------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
//

#include "DBAccess/FareDispInclDsplTypeDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareDispInclDsplType.h"
#include "DBAccess/Queries/QueryGetFareDispInclDsplType.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
FareDispInclDsplTypeDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareDispInclDsplTypeDAO"));

FareDispInclDsplTypeDAO&
FareDispInclDsplTypeDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareDispInclDsplType*>&
getFareDispInclDsplTypeData(const Indicator& userApplType,
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
    FareDispInclDsplTypeHistoricalDAO& dao = FareDispInclDsplTypeHistoricalDAO::instance();
    return dao.get(
        deleteList, userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, ticketDate);
  }
  else
  {
    FareDispInclDsplTypeDAO& dao = FareDispInclDsplTypeDAO::instance();
    return dao.get(
        deleteList, userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, ticketDate);
  }
}

const std::vector<FareDispInclDsplType*>&
FareDispInclDsplTypeDAO::get(DeleteList& del,
                             const Indicator& userApplType,
                             const UserApplCode& userAppl,
                             const Indicator& pseudoCityType,
                             const PseudoCityCode& pseudoCity,
                             const InclusionCode& inclusionCode,
                             const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareDispInclDsplTypeKey key(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  return *ptr;
}

FareDispInclDsplTypeKey
FareDispInclDsplTypeDAO::createKey(FareDispInclDsplType* info)
{
  return FareDispInclDsplTypeKey(info->userApplType(),
                                 info->userAppl(),
                                 info->pseudoCityType(),
                                 info->pseudoCity(),
                                 info->inclusionCode());
}

std::vector<FareDispInclDsplType*>*
FareDispInclDsplTypeDAO::create(FareDispInclDsplTypeKey key)
{
  std::vector<FareDispInclDsplType*>* ret = new std::vector<FareDispInclDsplType*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareDispInclDsplType idt(dbAdapter->getAdapter());
    idt.findFareDispInclDsplType(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareDispInclDsplTypeDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FareDispInclDsplTypeDAO::destroy(FareDispInclDsplTypeKey key,
                                 std::vector<FareDispInclDsplType*>* recs)
{
  destroyContainer(recs);
}

void
FareDispInclDsplTypeDAO::load()
{
  StartupLoader<QueryGetAllFareDispInclDsplType, FareDispInclDsplType, FareDispInclDsplTypeDAO>();
}

std::string
FareDispInclDsplTypeDAO::_name("FareDispInclDsplType");
std::string
FareDispInclDsplTypeDAO::_cacheClass("FareDisplay");

DAOHelper<FareDispInclDsplTypeDAO>
FareDispInclDsplTypeDAO::_helper(_name);

FareDispInclDsplTypeDAO* FareDispInclDsplTypeDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: FareDispInclDsplTypeHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
FareDispInclDsplTypeHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareDispInclDsplTypeHistoricalDAO"));
FareDispInclDsplTypeHistoricalDAO&
FareDispInclDsplTypeHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareDispInclDsplType*>&
FareDispInclDsplTypeHistoricalDAO::get(DeleteList& del,
                                       const Indicator& userApplType,
                                       const UserApplCode& userAppl,
                                       const Indicator& pseudoCityType,
                                       const PseudoCityCode& pseudoCity,
                                       const InclusionCode& inclusionCode,
                                       const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareDispInclDsplTypeKey key(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  return *ptr;
}

std::vector<FareDispInclDsplType*>*
FareDispInclDsplTypeHistoricalDAO::create(FareDispInclDsplTypeKey key)
{
  std::vector<FareDispInclDsplType*>* ret = new std::vector<FareDispInclDsplType*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareDispInclDsplType idt(dbAdapter->getAdapter());
    idt.findFareDispInclDsplType(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareDispInclDsplTypeHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FareDispInclDsplTypeHistoricalDAO::destroy(FareDispInclDsplTypeKey key,
                                           std::vector<FareDispInclDsplType*>* recs)
{
  destroyContainer(recs);
}

std::string
FareDispInclDsplTypeHistoricalDAO::_name("FareDispInclDsplTypeHistorical");
std::string
FareDispInclDsplTypeHistoricalDAO::_cacheClass("FareDisplay");
DAOHelper<FareDispInclDsplTypeHistoricalDAO>
FareDispInclDsplTypeHistoricalDAO::_helper(_name);

FareDispInclDsplTypeHistoricalDAO* FareDispInclDsplTypeHistoricalDAO::_instance = nullptr;

} // namespace tse
