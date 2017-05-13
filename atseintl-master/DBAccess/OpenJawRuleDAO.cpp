//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/OpenJawRuleDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/OpenJawRule.h"
#include "DBAccess/Queries/QueryGetOpenJawRule.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

namespace tse
{
log4cxx::LoggerPtr
OpenJawRuleDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.OpenJawRuleDAO"));

OpenJawRuleDAO&
OpenJawRuleDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const OpenJawRule*
getOpenJawRuleData(const VendorCode& vendor,
                   const int itemNo,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical)
{
  if (isHistorical)
  {
    OpenJawRuleHistoricalDAO& dao = OpenJawRuleHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    OpenJawRuleDAO& dao = OpenJawRuleDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const OpenJawRule*
OpenJawRuleDAO::get(DeleteList& del,
                    const VendorCode& vendor,
                    int itemNo,
                    const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  OpenJawRuleKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentG<OpenJawRule> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<OpenJawRule>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<OpenJawRule*>*
OpenJawRuleDAO::create(OpenJawRuleKey key)
{
  std::vector<OpenJawRule*>* ret = new std::vector<OpenJawRule*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetOpenJawRule ojr(dbAdapter->getAdapter());
    ojr.findOpenJawRule(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in OpenJawRuleDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
OpenJawRuleDAO::destroy(OpenJawRuleKey key, std::vector<OpenJawRule*>* recs)
{
  std::vector<OpenJawRule*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

OpenJawRuleKey
OpenJawRuleDAO::createKey(OpenJawRule* info)
{
  return OpenJawRuleKey(info->vendor(), info->itemNo());
}

void
OpenJawRuleDAO::load()
{
  StartupLoaderNoDB<OpenJawRule, OpenJawRuleDAO>();
}

std::string
OpenJawRuleDAO::_name("OpenJawRule");
std::string
OpenJawRuleDAO::_cacheClass("Rules");
DAOHelper<OpenJawRuleDAO>
OpenJawRuleDAO::_helper(_name);
OpenJawRuleDAO* OpenJawRuleDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: OpenJawRuleHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
OpenJawRuleHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.OpenJawRuleHistoricalDAO"));
OpenJawRuleHistoricalDAO&
OpenJawRuleHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const OpenJawRule*
OpenJawRuleHistoricalDAO::get(DeleteList& del,
                              const VendorCode& vendor,
                              int itemNo,
                              const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  OpenJawRuleHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<OpenJawRule> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<OpenJawRule>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<OpenJawRule*>*
OpenJawRuleHistoricalDAO::create(OpenJawRuleHistoricalKey key)
{
  std::vector<OpenJawRule*>* ret = new std::vector<OpenJawRule*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetOpenJawRuleHistorical ojr(dbAdapter->getAdapter());
    ojr.findOpenJawRule(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in OpenJawRuleHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
OpenJawRuleHistoricalDAO::destroy(OpenJawRuleHistoricalKey key, std::vector<OpenJawRule*>* recs)
{
  std::vector<OpenJawRule*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
OpenJawRuleHistoricalDAO::_name("OpenJawRuleHistorical");
std::string
OpenJawRuleHistoricalDAO::_cacheClass("Rules");
DAOHelper<OpenJawRuleHistoricalDAO>
OpenJawRuleHistoricalDAO::_helper(_name);
OpenJawRuleHistoricalDAO* OpenJawRuleHistoricalDAO::_instance = nullptr;

} // namespace tse
