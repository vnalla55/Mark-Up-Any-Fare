//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/RoundTripRuleDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetRoundTripRule.h"
#include "DBAccess/RoundTripRuleItem.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

namespace tse
{
log4cxx::LoggerPtr
RoundTripRuleDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.RoundTripRuleDAO"));
RoundTripRuleDAO&
RoundTripRuleDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const RoundTripRuleItem*
getRoundTripRuleItemData(const VendorCode& vendor,
                         int itemNo,
                         DeleteList& deleteList,
                         const DateTime& ticketDate,
                         bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    RoundTripRuleHistoricalDAO& dao = RoundTripRuleHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    RoundTripRuleDAO& dao = RoundTripRuleDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

RoundTripRuleItem*
RoundTripRuleDAO::get(DeleteList& del,
                      const VendorCode& vendor,
                      int itemNo,
                      const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  RoundTripRuleKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (UNLIKELY(ptr->empty()))
    return nullptr;

  del.copy(ptr);

  IsCurrentG<RoundTripRuleItem> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (LIKELY(isCurrent(*iter) && NotInhibit<RoundTripRuleItem>(*iter)))
      return *iter;
  }
  return nullptr;
}

std::vector<RoundTripRuleItem*>*
RoundTripRuleDAO::create(RoundTripRuleKey key)
{
  std::vector<RoundTripRuleItem*>* ret = new std::vector<RoundTripRuleItem*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetRoundTripRule rtr(dbAdapter->getAdapter());
    rtr.findRoundTripRuleItem(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in RoundTripRuleDAO::load");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
RoundTripRuleDAO::destroy(RoundTripRuleKey key, std::vector<RoundTripRuleItem*>* recs)
{
  std::vector<RoundTripRuleItem*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
RoundTripRuleDAO::_name("RoundTripRule");
std::string
RoundTripRuleDAO::_cacheClass("Rules");
DAOHelper<RoundTripRuleDAO>
RoundTripRuleDAO::_helper(_name);
RoundTripRuleDAO* RoundTripRuleDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: RoundTripRuleHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
RoundTripRuleHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.RoundTripRuleHistoricalDAO"));
RoundTripRuleHistoricalDAO&
RoundTripRuleHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

RoundTripRuleItem*
RoundTripRuleHistoricalDAO::get(DeleteList& del,
                                const VendorCode& vendor,
                                int itemNo,
                                const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  RoundTripRuleHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<RoundTripRuleItem> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<RoundTripRuleItem>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<RoundTripRuleItem*>*
RoundTripRuleHistoricalDAO::create(RoundTripRuleHistoricalKey key)
{
  std::vector<RoundTripRuleItem*>* ret = new std::vector<RoundTripRuleItem*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetRoundTripRuleHistorical rtr(dbAdapter->getAdapter());
    rtr.findRoundTripRuleItem(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in RoundTripRuleHistoricalDAO::load");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
RoundTripRuleHistoricalDAO::destroy(RoundTripRuleHistoricalKey key,
                                    std::vector<RoundTripRuleItem*>* recs)
{
  std::vector<RoundTripRuleItem*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
RoundTripRuleHistoricalDAO::_name("RoundTripRuleHistorical");
std::string
RoundTripRuleHistoricalDAO::_cacheClass("Rules");
DAOHelper<RoundTripRuleHistoricalDAO>
RoundTripRuleHistoricalDAO::_helper(_name);
RoundTripRuleHistoricalDAO* RoundTripRuleHistoricalDAO::_instance = nullptr;

} // namespace tse
