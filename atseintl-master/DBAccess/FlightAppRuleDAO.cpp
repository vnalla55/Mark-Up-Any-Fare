//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/FlightAppRuleDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FlightAppRule.h"
#include "DBAccess/Queries/QueryGetFlightAppRule.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
FlightAppRuleDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FlightAppRuleDAO"));

FlightAppRuleDAO&
FlightAppRuleDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const FlightAppRule*
getFlightAppRuleData(const VendorCode& vendor,
                     int itemNo,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    FlightAppRuleHistoricalDAO& dao = FlightAppRuleHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    FlightAppRuleDAO& dao = FlightAppRuleDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const FlightAppRule*
FlightAppRuleDAO::get(DeleteList& del,
                      const VendorCode& vendor,
                      int itemNo,
                      const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  FlightAppRuleKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  DAOCache::value_type::iterator iter =
      find_if(ptr->begin(), ptr->end(), IsCurrentG<FlightAppRule>(ticketDate));
  if (LIKELY(iter != ptr->end()))
  {
    return *iter;
  }
  return nullptr;
}

std::vector<FlightAppRule*>*
FlightAppRuleDAO::create(FlightAppRuleKey key)
{
  std::vector<FlightAppRule*>* ret = new std::vector<FlightAppRule*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFlightAppRule fltApp(dbAdapter->getAdapter());
    fltApp.findFlightAppRule(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FlightAppRuleDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FlightAppRuleDAO::destroy(FlightAppRuleKey key, std::vector<FlightAppRule*>* recs)
{
  std::vector<FlightAppRule*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

FlightAppRuleKey
FlightAppRuleDAO::createKey(FlightAppRule* info)
{
  return FlightAppRuleKey(info->vendor(), info->itemNo());
}

void
FlightAppRuleDAO::load()
{
  StartupLoaderNoDB<FlightAppRule, FlightAppRuleDAO>();
}

sfc::CompressedData*
FlightAppRuleDAO::compress(const std::vector<FlightAppRule*>* vect) const
{
  return compressVector(vect);
}

std::vector<FlightAppRule*>*
FlightAppRuleDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<FlightAppRule>(compressed);
}

std::string
FlightAppRuleDAO::_name("FlightAppRule");
std::string
FlightAppRuleDAO::_cacheClass("Rules");
DAOHelper<FlightAppRuleDAO>
FlightAppRuleDAO::_helper(_name);
FlightAppRuleDAO* FlightAppRuleDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: FlightAppRuleHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
FlightAppRuleHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FlightAppRuleHistoricalDAO"));
FlightAppRuleHistoricalDAO&
FlightAppRuleHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const FlightAppRule*
FlightAppRuleHistoricalDAO::get(DeleteList& del,
                                const VendorCode& vendor,
                                int itemNo,
                                const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  FlightAppRuleHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  DAOCache::value_type::iterator iter =
      find_if(ptr->begin(), ptr->end(), IsCurrentH<FlightAppRule>(ticketDate));
  if (iter != ptr->end())
  {
    return *iter;
  }
  return nullptr;
}

std::vector<FlightAppRule*>*
FlightAppRuleHistoricalDAO::create(FlightAppRuleHistoricalKey key)
{
  std::vector<FlightAppRule*>* ret = new std::vector<FlightAppRule*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFlightAppRuleHistorical fltApp(dbAdapter->getAdapter());
    fltApp.findFlightAppRule(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FlightAppRuleHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FlightAppRuleHistoricalDAO::destroy(FlightAppRuleHistoricalKey key,
                                    std::vector<FlightAppRule*>* recs)
{
  std::vector<FlightAppRule*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

FlightAppRuleHistoricalKey
FlightAppRuleHistoricalDAO::createKey(const FlightAppRule* info,
                                      const DateTime& startDate,
                                      const DateTime& endDate)
{
  return FlightAppRuleHistoricalKey(info->vendor(), info->itemNo(), startDate, endDate);
}

void
FlightAppRuleHistoricalDAO::load()
{
  StartupLoaderNoDB<FlightAppRule, FlightAppRuleHistoricalDAO>();
}

sfc::CompressedData*
FlightAppRuleHistoricalDAO::compress(const std::vector<FlightAppRule*>* vect) const
{
  return compressVector(vect);
}

std::vector<FlightAppRule*>*
FlightAppRuleHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<FlightAppRule>(compressed);
}

std::string
FlightAppRuleHistoricalDAO::_name("FlightAppRuleHistorical");
std::string
FlightAppRuleHistoricalDAO::_cacheClass("Rules");
DAOHelper<FlightAppRuleHistoricalDAO>
FlightAppRuleHistoricalDAO::_helper(_name);

FlightAppRuleHistoricalDAO* FlightAppRuleHistoricalDAO::_instance = nullptr;

} // namespace tse
