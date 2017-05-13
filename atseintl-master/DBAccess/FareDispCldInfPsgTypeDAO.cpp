//------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
//

#include "DBAccess/FareDispCldInfPsgTypeDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareDispCldInfPsgType.h"
#include "DBAccess/Queries/QueryGetFareDispCldInfPsgType.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
FareDispCldInfPsgTypeDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareDispCldInfPsgTypeDAO"));

FareDispCldInfPsgTypeDAO&
FareDispCldInfPsgTypeDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareDispCldInfPsgType*>&
getFareDispCldInfPsgTypeData(const Indicator& userApplType,
                             const UserApplCode& userAppl,
                             const Indicator& pseudoCityType,
                             const PseudoCityCode& pseudoCity,
                             const InclusionCode& inclusionCode,
                             const Indicator& psgTypeInd,
                             DeleteList& deleteList,
                             const DateTime& ticketDate,
                             bool isHistorical)
{
  if (isHistorical)
  {
    FareDispCldInfPsgTypeHistoricalDAO& dao = FareDispCldInfPsgTypeHistoricalDAO::instance();
    return dao.get(deleteList,
                   userApplType,
                   userAppl,
                   pseudoCityType,
                   pseudoCity,
                   inclusionCode,
                   psgTypeInd,
                   ticketDate);
  }
  else
  {
    FareDispCldInfPsgTypeDAO& dao = FareDispCldInfPsgTypeDAO::instance();
    return dao.get(deleteList,
                   userApplType,
                   userAppl,
                   pseudoCityType,
                   pseudoCity,
                   inclusionCode,
                   psgTypeInd,
                   ticketDate);
  }
}

const std::vector<FareDispCldInfPsgType*>&
FareDispCldInfPsgTypeDAO::get(DeleteList& del,
                              const Indicator& userApplType,
                              const UserApplCode& userAppl,
                              const Indicator& pseudoCityType,
                              const PseudoCityCode& pseudoCity,
                              const InclusionCode& inclusionCode,
                              const Indicator& psgTypeInd,
                              const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareDispCldInfPsgTypeKey key(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<FareDispCldInfPsgType*>* ret = new std::vector<FareDispCldInfPsgType*>;
  del.adopt(ret);
  DAOCache::value_type::iterator i = ptr->begin();
  for (; i != ptr->end(); ++i)
  {
    if ((*i)->psgTypeInd() == psgTypeInd)
      ret->push_back(*i);
  }
  return *ret;
}

FareDispCldInfPsgTypeKey
FareDispCldInfPsgTypeDAO::createKey(FareDispCldInfPsgType* info)
{
  return FareDispCldInfPsgTypeKey(info->userApplType(),
                                  info->userAppl(),
                                  info->pseudoCityType(),
                                  info->pseudoCity(),
                                  info->inclusionCode());
}

std::vector<FareDispCldInfPsgType*>*
FareDispCldInfPsgTypeDAO::create(FareDispCldInfPsgTypeKey key)
{
  std::vector<FareDispCldInfPsgType*>* ret = new std::vector<FareDispCldInfPsgType*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareDispCldInfPsgType cipt(dbAdapter->getAdapter());
    cipt.findFareDispCldInfPsgType(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareDispCldInfPsgTypeDAO::create");
    throw;
  }

  return ret;
}

void
FareDispCldInfPsgTypeDAO::destroy(FareDispCldInfPsgTypeKey key,
                                  std::vector<FareDispCldInfPsgType*>* recs)
{
  destroyContainer(recs);
}

void
FareDispCldInfPsgTypeDAO::load()
{
  StartupLoader<QueryGetAllFareDispCldInfPsgType,
                FareDispCldInfPsgType,
                FareDispCldInfPsgTypeDAO>();
}

std::string
FareDispCldInfPsgTypeDAO::_name("FareDispCldInfPsgType");
std::string
FareDispCldInfPsgTypeDAO::_cacheClass("FareDisplay");

DAOHelper<FareDispCldInfPsgTypeDAO>
FareDispCldInfPsgTypeDAO::_helper(_name);

FareDispCldInfPsgTypeDAO* FareDispCldInfPsgTypeDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: FareDispCldInfPsgTypeHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
FareDispCldInfPsgTypeHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareDispCldInfPsgTypeHistoricalDAO"));
FareDispCldInfPsgTypeHistoricalDAO&
FareDispCldInfPsgTypeHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareDispCldInfPsgType*>&
FareDispCldInfPsgTypeHistoricalDAO::get(DeleteList& del,
                                        const Indicator& userApplType,
                                        const UserApplCode& userAppl,
                                        const Indicator& pseudoCityType,
                                        const PseudoCityCode& pseudoCity,
                                        const InclusionCode& inclusionCode,
                                        const Indicator& psgTypeInd,
                                        const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareDispCldInfPsgTypeKey key(userApplType, userAppl, pseudoCityType, pseudoCity, inclusionCode);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<FareDispCldInfPsgType*>* ret = new std::vector<FareDispCldInfPsgType*>;
  del.adopt(ret);
  DAOCache::value_type::iterator i = ptr->begin();
  for (; i != ptr->end(); ++i)
  {
    if ((*i)->psgTypeInd() == psgTypeInd)
      ret->push_back(*i);
  }
  return *ret;
}

std::vector<FareDispCldInfPsgType*>*
FareDispCldInfPsgTypeHistoricalDAO::create(FareDispCldInfPsgTypeKey key)
{
  std::vector<FareDispCldInfPsgType*>* ret = new std::vector<FareDispCldInfPsgType*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareDispCldInfPsgType cipt(dbAdapter->getAdapter());
    cipt.findFareDispCldInfPsgType(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareDispCldInfPsgTypeHistoricalDAO::create");
    throw;
  }

  return ret;
}

void
FareDispCldInfPsgTypeHistoricalDAO::destroy(FareDispCldInfPsgTypeKey key,
                                            std::vector<FareDispCldInfPsgType*>* recs)
{
  destroyContainer(recs);
}

std::string
FareDispCldInfPsgTypeHistoricalDAO::_name("FareDispCldInfPsgTypeHistorical");
std::string
FareDispCldInfPsgTypeHistoricalDAO::_cacheClass("FareDisplay");
DAOHelper<FareDispCldInfPsgTypeHistoricalDAO>
FareDispCldInfPsgTypeHistoricalDAO::_helper(_name);

FareDispCldInfPsgTypeHistoricalDAO* FareDispCldInfPsgTypeHistoricalDAO::_instance = nullptr;

} // namespace tse
