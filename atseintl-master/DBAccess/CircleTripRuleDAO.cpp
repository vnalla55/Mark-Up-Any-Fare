//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/CircleTripRuleDAO.h"

#include "Common/Logger.h"
#include "DBAccess/CircleTripRuleItem.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetCircleTripRuleItem.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <time.h>

namespace tse
{
log4cxx::LoggerPtr
CircleTripRuleDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.CircleTripRuleDAO"));

CircleTripRuleDAO&
CircleTripRuleDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const CircleTripRuleItem*
getCircleTripRuleItemData(const VendorCode& vendor,
                          int itemNo,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical)
{
  if (isHistorical)
  {
    CircleTripRuleHistoricalDAO& dao = CircleTripRuleHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    CircleTripRuleDAO& dao = CircleTripRuleDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

CircleTripRuleItem*
CircleTripRuleDAO::get(DeleteList& del,
                       const VendorCode& vendor,
                       int itemNo,
                       const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  CircleTripRuleKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentG<CircleTripRuleItem> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<CircleTripRuleItem>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<CircleTripRuleItem*>*
CircleTripRuleDAO::create(CircleTripRuleKey key)
{
  std::vector<CircleTripRuleItem*>* ret = new std::vector<CircleTripRuleItem*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCircleTripRuleItem ctr(dbAdapter->getAdapter());
    ctr.findCircleTripRuleItem(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CircleTripRuleDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CircleTripRuleDAO::destroy(CircleTripRuleKey key, std::vector<CircleTripRuleItem*>* recs)
{
  std::vector<CircleTripRuleItem*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

CircleTripRuleKey
CircleTripRuleDAO::createKey(CircleTripRuleItem* info)
{
  return CircleTripRuleKey(info->vendor(), info->itemNo());
}

void
CircleTripRuleDAO::load()
{
  StartupLoaderNoDB<CircleTripRuleItem, CircleTripRuleDAO>();
}

std::string
CircleTripRuleDAO::_name("CircleTripRule");
std::string
CircleTripRuleDAO::_cacheClass("Rules");
DAOHelper<CircleTripRuleDAO>
CircleTripRuleDAO::_helper(_name);
CircleTripRuleDAO* CircleTripRuleDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: CircleTripRuleHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
CircleTripRuleHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.CircleTripRuleHistoricalDAO"));
CircleTripRuleHistoricalDAO&
CircleTripRuleHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

CircleTripRuleItem*
CircleTripRuleHistoricalDAO::get(DeleteList& del,
                                 const VendorCode& vendor,
                                 int itemNo,
                                 const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  CircleTripRuleHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<CircleTripRuleItem> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<CircleTripRuleItem>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<CircleTripRuleItem*>*
CircleTripRuleHistoricalDAO::create(CircleTripRuleHistoricalKey key)
{
  std::vector<CircleTripRuleItem*>* ret = new std::vector<CircleTripRuleItem*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCircleTripRuleItemHistorical ctr(dbAdapter->getAdapter());
    ctr.findCircleTripRuleItem(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CircleTripRuleHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CircleTripRuleHistoricalDAO::destroy(CircleTripRuleHistoricalKey key,
                                     std::vector<CircleTripRuleItem*>* recs)
{
  std::vector<CircleTripRuleItem*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
CircleTripRuleHistoricalDAO::_name("CircleTripRuleHistorical");
std::string
CircleTripRuleHistoricalDAO::_cacheClass("Rules");
DAOHelper<CircleTripRuleHistoricalDAO>
CircleTripRuleHistoricalDAO::_helper(_name);
CircleTripRuleHistoricalDAO* CircleTripRuleHistoricalDAO::_instance = nullptr;

} // namespace tse
