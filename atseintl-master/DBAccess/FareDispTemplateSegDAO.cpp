//------------------------------------------------------------------------------
// Copyright 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//------------------------------------------------------------------------------
//

#include "DBAccess/FareDispTemplateSegDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareDispTemplateSeg.h"
#include "DBAccess/Queries/QueryGetFareDispTemplateSeg.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
FareDispTemplateSegDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareDispTemplateSegDAO"));

FareDispTemplateSegDAO&
FareDispTemplateSegDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareDispTemplateSeg*>&
getFareDispTemplateSegData(const int& templateID,
                           const Indicator& templateType,
                           DeleteList& deleteList,
                           const DateTime& ticketDate,
                           bool isHistorical)
{
  if (isHistorical)
  {
    FareDispTemplateSegHistoricalDAO& dao = FareDispTemplateSegHistoricalDAO::instance();
    return dao.get(deleteList, templateID, templateType, ticketDate);
  }
  else
  {
    FareDispTemplateSegDAO& dao = FareDispTemplateSegDAO::instance();
    return dao.get(deleteList, templateID, templateType, ticketDate);
  }
}

const std::vector<FareDispTemplateSeg*>&
FareDispTemplateSegDAO::get(DeleteList& del,
                            const int& templateID,
                            const Indicator& templateType,
                            const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareDispTemplateSegKey key(templateID, templateType);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  return *ptr;
}

FareDispTemplateSegKey
FareDispTemplateSegDAO::createKey(FareDispTemplateSeg* info)
{
  return FareDispTemplateSegKey(info->templateID(), info->templateType());
}

std::vector<FareDispTemplateSeg*>*
FareDispTemplateSegDAO::create(FareDispTemplateSegKey key)
{
  std::vector<FareDispTemplateSeg*>* ret = new std::vector<FareDispTemplateSeg*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareDispTemplateSeg dts(dbAdapter->getAdapter());
    dts.findFareDispTemplateSeg(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareDispTemplateSegDAO::create");
    throw;
  }

  return ret;
}

void
FareDispTemplateSegDAO::destroy(FareDispTemplateSegKey key, std::vector<FareDispTemplateSeg*>* recs)
{
  destroyContainer(recs);
}

void
FareDispTemplateSegDAO::load()
{
  StartupLoader<QueryGetAllFareDispTemplateSeg, FareDispTemplateSeg, FareDispTemplateSegDAO>();
}

std::string
FareDispTemplateSegDAO::_name("FareDispTemplateSeg");
std::string
FareDispTemplateSegDAO::_cacheClass("FareDisplay");

DAOHelper<FareDispTemplateSegDAO>
FareDispTemplateSegDAO::_helper(_name);

FareDispTemplateSegDAO* FareDispTemplateSegDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: FareDispTemplateSegHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
FareDispTemplateSegHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareDispTemplateSegHistoricalDAO"));
FareDispTemplateSegHistoricalDAO&
FareDispTemplateSegHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareDispTemplateSeg*>&
FareDispTemplateSegHistoricalDAO::get(DeleteList& del,
                                      const int& templateID,
                                      const Indicator& templateType,
                                      const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareDispTemplateSegKey key(templateID, templateType);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  return *ptr;
}

std::vector<FareDispTemplateSeg*>*
FareDispTemplateSegHistoricalDAO::create(FareDispTemplateSegKey key)
{
  std::vector<FareDispTemplateSeg*>* ret = new std::vector<FareDispTemplateSeg*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareDispTemplateSeg dts(dbAdapter->getAdapter());
    dts.findFareDispTemplateSeg(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareDispTemplateSegHistoricalDAO::create");
    throw;
  }

  return ret;
}

void
FareDispTemplateSegHistoricalDAO::destroy(FareDispTemplateSegKey key,
                                          std::vector<FareDispTemplateSeg*>* recs)
{
  destroyContainer(recs);
}

std::string
FareDispTemplateSegHistoricalDAO::_name("FareDispTemplateSegHistorical");
std::string
FareDispTemplateSegHistoricalDAO::_cacheClass("FareDisplay");
DAOHelper<FareDispTemplateSegHistoricalDAO>
FareDispTemplateSegHistoricalDAO::_helper(_name);

FareDispTemplateSegHistoricalDAO* FareDispTemplateSegHistoricalDAO::_instance = nullptr;

} // namespace tse
