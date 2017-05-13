//------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
//

#include "DBAccess/FareDispTemplateDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareDispTemplate.h"
#include "DBAccess/Queries/QueryGetFareDispTemplate.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
FareDispTemplateDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FareDispTemplateDAO"));

FareDispTemplateDAO&
FareDispTemplateDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareDispTemplate*>&
getFareDispTemplateData(const int& templateID,
                        const Indicator& templateType,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical)
{
  if (isHistorical)
  {
    FareDispTemplateHistoricalDAO& dao = FareDispTemplateHistoricalDAO::instance();
    return dao.get(deleteList, templateID, templateType, ticketDate);
  }
  else
  {
    FareDispTemplateDAO& dao = FareDispTemplateDAO::instance();
    return dao.get(deleteList, templateID, templateType, ticketDate);
  }
}

const std::vector<FareDispTemplate*>&
FareDispTemplateDAO::get(DeleteList& del,
                         const int& templateID,
                         const Indicator& templateType,
                         const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareDispTemplateKey key(templateID, templateType);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  return *ptr;
}

FareDispTemplateKey
FareDispTemplateDAO::createKey(FareDispTemplate* info)
{
  return FareDispTemplateKey(info->templateID(), info->templateType());
}

std::vector<FareDispTemplate*>*
FareDispTemplateDAO::create(FareDispTemplateKey key)
{
  std::vector<FareDispTemplate*>* ret = new std::vector<FareDispTemplate*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareDispTemplate dt(dbAdapter->getAdapter());
    dt.findFareDispTemplate(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareDispTemplateDAO::create");
    throw;
  }

  return ret;
}

void
FareDispTemplateDAO::destroy(FareDispTemplateKey key, std::vector<FareDispTemplate*>* recs)
{
  destroyContainer(recs);
}

void
FareDispTemplateDAO::load()
{
  StartupLoader<QueryGetAllFareDispTemplate, FareDispTemplate, FareDispTemplateDAO>();
}

std::string
FareDispTemplateDAO::_name("FareDispTemplate");
std::string
FareDispTemplateDAO::_cacheClass("FareDisplay");

DAOHelper<FareDispTemplateDAO>
FareDispTemplateDAO::_helper(_name);

FareDispTemplateDAO* FareDispTemplateDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: FareDispTemplateHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
FareDispTemplateHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareDispTemplateHistoricalDAO"));
FareDispTemplateHistoricalDAO&
FareDispTemplateHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareDispTemplate*>&
FareDispTemplateHistoricalDAO::get(DeleteList& del,
                                   const int& templateID,
                                   const Indicator& templateType,
                                   const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareDispTemplateKey key(templateID, templateType);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  return *ptr;
}

std::vector<FareDispTemplate*>*
FareDispTemplateHistoricalDAO::create(FareDispTemplateKey key)
{
  std::vector<FareDispTemplate*>* ret = new std::vector<FareDispTemplate*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareDispTemplate dt(dbAdapter->getAdapter());
    dt.findFareDispTemplate(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareDispTemplateHistoricalDAO::create");
    throw;
  }

  return ret;
}

void
FareDispTemplateHistoricalDAO::destroy(FareDispTemplateKey key,
                                       std::vector<FareDispTemplate*>* recs)
{
  destroyContainer(recs);
}

std::string
FareDispTemplateHistoricalDAO::_name("FareDispTemplateHistorical");
std::string
FareDispTemplateHistoricalDAO::_cacheClass("FareDisplay");
DAOHelper<FareDispTemplateHistoricalDAO>
FareDispTemplateHistoricalDAO::_helper(_name);

FareDispTemplateHistoricalDAO* FareDispTemplateHistoricalDAO::_instance = nullptr;

} // namespace tse
