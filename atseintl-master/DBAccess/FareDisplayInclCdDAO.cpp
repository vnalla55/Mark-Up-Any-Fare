//------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
//

#include "DBAccess/FareDisplayInclCdDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareDisplayInclCd.h"
#include "DBAccess/Queries/QueryGetFareDisplayInclCd.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
FareDisplayInclCdDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FareDisplayInclCdDAO"));

FareDisplayInclCdDAO&
FareDisplayInclCdDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareDisplayInclCd*>&
getFareDisplayInclCdData(const Indicator& userApplType,
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
    FareDisplayInclCdHistoricalDAO& dao = FareDisplayInclCdHistoricalDAO::instance();
    return dao.get(
        deleteList, userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, ticketDate);
  }
  else
  {
    FareDisplayInclCdDAO& dao = FareDisplayInclCdDAO::instance();
    return dao.get(
        deleteList, userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode, ticketDate);
  }
}

const std::vector<FareDisplayInclCd*>&
FareDisplayInclCdDAO::get(DeleteList& del,
                          const Indicator& userApplType,
                          const UserApplCode& userAppl,
                          const Indicator& pseudoCityType,
                          const PseudoCityCode& pseudoCity,
                          const InclusionCode& inclusionCode,
                          const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareDisplayInclCdKey key(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  return *ptr;
}

FareDisplayInclCdKey
FareDisplayInclCdDAO::createKey(FareDisplayInclCd* info)
{
  return FareDisplayInclCdKey(info->userApplType(),
                              info->userAppl(),
                              info->pseudoCityType(),
                              info->pseudoCity(),
                              info->inclusionCode());
}

std::vector<FareDisplayInclCd*>*
FareDisplayInclCdDAO::create(FareDisplayInclCdKey key)
{
  std::vector<FareDisplayInclCd*>* ret = new std::vector<FareDisplayInclCd*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareDisplayInclCd ic(dbAdapter->getAdapter());
    ic.findFareDisplayInclCd(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareDisplayInclCdDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FareDisplayInclCdDAO::destroy(FareDisplayInclCdKey key, std::vector<FareDisplayInclCd*>* recs)
{
  destroyContainer(recs);
}

void
FareDisplayInclCdDAO::load()
{
  StartupLoader<QueryGetAllFareDisplayInclCd, FareDisplayInclCd, FareDisplayInclCdDAO>();
}

std::string
FareDisplayInclCdDAO::_name("FareDisplayInclCd");
std::string
FareDisplayInclCdDAO::_cacheClass("FareDisplay");

DAOHelper<FareDisplayInclCdDAO>
FareDisplayInclCdDAO::_helper(_name);

FareDisplayInclCdDAO* FareDisplayInclCdDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: FareDisplayInclCdHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
FareDisplayInclCdHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareDisplayInclCdHistoricalDAO"));
FareDisplayInclCdHistoricalDAO&
FareDisplayInclCdHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareDisplayInclCd*>&
FareDisplayInclCdHistoricalDAO::get(DeleteList& del,
                                    const Indicator& userApplType,
                                    const UserApplCode& userAppl,
                                    const Indicator& pseudoCityType,
                                    const PseudoCityCode& pseudoCity,
                                    const InclusionCode& inclusionCode,
                                    const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareDisplayInclCdKey key(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  return *ptr;
}

std::vector<FareDisplayInclCd*>*
FareDisplayInclCdHistoricalDAO::create(FareDisplayInclCdKey key)
{
  std::vector<FareDisplayInclCd*>* ret = new std::vector<FareDisplayInclCd*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareDisplayInclCd ic(dbAdapter->getAdapter());
    ic.findFareDisplayInclCd(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareDisplayInclCdHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FareDisplayInclCdHistoricalDAO::destroy(FareDisplayInclCdKey key,
                                        std::vector<FareDisplayInclCd*>* recs)
{
  destroyContainer(recs);
}

FareDisplayInclCdKey
FareDisplayInclCdHistoricalDAO::createKey(const FareDisplayInclCd* info,
                                          const DateTime& startDate,
                                          const DateTime& endDate)
{
  return FareDisplayInclCdKey(info->userApplType(),
                              info->userAppl(),
                              info->pseudoCityType(),
                              info->pseudoCity(),
                              info->inclusionCode());
}

void
FareDisplayInclCdHistoricalDAO::load()
{
  StartupLoaderNoDB<FareDisplayInclCd, FareDisplayInclCdHistoricalDAO>();
}

std::string
FareDisplayInclCdHistoricalDAO::_name("FareDisplayInclCdHistorical");
std::string
FareDisplayInclCdHistoricalDAO::_cacheClass("FareDisplay");
DAOHelper<FareDisplayInclCdHistoricalDAO>
FareDisplayInclCdHistoricalDAO::_helper(_name);

FareDisplayInclCdHistoricalDAO* FareDisplayInclCdHistoricalDAO::_instance = nullptr;

} // namespace tse
