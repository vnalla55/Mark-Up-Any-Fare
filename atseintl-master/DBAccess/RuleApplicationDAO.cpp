//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/RuleApplicationDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetRuleAppl.h"
#include "DBAccess/RuleApplication.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

namespace tse
{
log4cxx::LoggerPtr
RuleApplicationDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.RuleApplicationDAO"));
RuleApplicationDAO&
RuleApplicationDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const RuleApplication*
getRuleApplicationData(const VendorCode& vendor,
                       int itemNo,
                       DeleteList& deleteList,
                       const DateTime& ticketDate,
                       bool isHistorical)
{
  if (isHistorical)
  {
    RuleApplicationHistoricalDAO& dao = RuleApplicationHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    RuleApplicationDAO& dao = RuleApplicationDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const RuleApplication*
RuleApplicationDAO::get(DeleteList& del,
                        const VendorCode& vendor,
                        int itemNo,
                        const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  RuleApplicationKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentG<RuleApplication> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<RuleApplication>(*iter))
      return *iter;
  }
  return nullptr;
}

RuleApplicationKey
RuleApplicationDAO::createKey(RuleApplication* info)
{
  return RuleApplicationKey(info->vendor(), info->itemNo());
}

void
RuleApplicationDAO::load()
{
  StartupLoaderNoDB<RuleApplication, RuleApplicationDAO>();
}

std::vector<RuleApplication*>*
RuleApplicationDAO::create(RuleApplicationKey key)
{
  std::vector<RuleApplication*>* ret = new std::vector<RuleApplication*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetRuleAppl ra(dbAdapter->getAdapter());
    ra.findRuleApplication(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in RuleApplicationDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
RuleApplicationDAO::destroy(RuleApplicationKey key, std::vector<RuleApplication*>* recs)
{
  destroyContainer(recs);
}

std::string
RuleApplicationDAO::_name("RuleApplication");
std::string
RuleApplicationDAO::_cacheClass("Rules");
DAOHelper<RuleApplicationDAO>
RuleApplicationDAO::_helper(_name);
RuleApplicationDAO* RuleApplicationDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: RuleApplicationHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
RuleApplicationHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.RuleApplicationHistoricalDAO"));
RuleApplicationHistoricalDAO&
RuleApplicationHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const RuleApplication*
RuleApplicationHistoricalDAO::get(DeleteList& del,
                                  const VendorCode& vendor,
                                  int itemNo,
                                  const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  RuleApplicationHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<RuleApplication> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<RuleApplication>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<RuleApplication*>*
RuleApplicationHistoricalDAO::create(RuleApplicationHistoricalKey key)
{
  std::vector<RuleApplication*>* ret = new std::vector<RuleApplication*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetRuleApplHistorical ra(dbAdapter->getAdapter());
    ra.findRuleApplication(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in RuleApplicationHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
RuleApplicationHistoricalDAO::destroy(RuleApplicationHistoricalKey key,
                                      std::vector<RuleApplication*>* recs)
{
  std::vector<RuleApplication*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
RuleApplicationHistoricalDAO::_name("RuleApplicationHistorical");
std::string
RuleApplicationHistoricalDAO::_cacheClass("Rules");
DAOHelper<RuleApplicationHistoricalDAO>
RuleApplicationHistoricalDAO::_helper(_name);
RuleApplicationHistoricalDAO* RuleApplicationHistoricalDAO::_instance = nullptr;

} // namespace tse
