//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/AccompaniedTravelDAO.h"

#include "Common/Logger.h"
#include "DBAccess/AccompaniedTravelInfo.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetAccompaniedTravelInfo.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
AccompaniedTravelDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.AccompaniedTravelDAO"));

AccompaniedTravelDAO&
AccompaniedTravelDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const AccompaniedTravelInfo*
getAccompaniedTravelData(const VendorCode& vendor,
                         int itemNo,
                         DeleteList& deleteList,
                         const DateTime& ticketDate,
                         bool isHistorical)
{
  if (isHistorical)
  {
    AccompaniedTravelHistoricalDAO& dao = AccompaniedTravelHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    AccompaniedTravelDAO& dao = AccompaniedTravelDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const AccompaniedTravelInfo*
AccompaniedTravelDAO::get(DeleteList& del,
                          const VendorCode& vendor,
                          int itemNo,
                          const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  AccompaniedTravelKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentG<AccompaniedTravelInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<AccompaniedTravelInfo>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<AccompaniedTravelInfo*>*
AccompaniedTravelDAO::create(AccompaniedTravelKey key)
{
  std::vector<AccompaniedTravelInfo*>* ret = new std::vector<AccompaniedTravelInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAccompaniedTravelInfo at(dbAdapter->getAdapter());
    at.findAccompaniedTravelInfo(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in AccompaniedTravelDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
AccompaniedTravelDAO::destroy(AccompaniedTravelKey key, std::vector<AccompaniedTravelInfo*>* recs)
{
  std::vector<AccompaniedTravelInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
AccompaniedTravelDAO::_name("AccompaniedTravel");
std::string
AccompaniedTravelDAO::_cacheClass("Rules");
DAOHelper<AccompaniedTravelDAO>
AccompaniedTravelDAO::_helper(_name);
AccompaniedTravelDAO* AccompaniedTravelDAO::_instance = nullptr;

AccompaniedTravelKey
AccompaniedTravelDAO::createKey(const AccompaniedTravelInfo* info)
{
  return AccompaniedTravelKey(info->vendor(), info->itemNo());
}

void
AccompaniedTravelDAO::load()
{
  StartupLoaderNoDB<AccompaniedTravelInfo, AccompaniedTravelDAO>();
}

// --------------------------------------------------
// Historical DAO: AccompaniedTravelHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
AccompaniedTravelHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.AccompaniedTravelHistoricalDAO"));
AccompaniedTravelHistoricalDAO&
AccompaniedTravelHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const AccompaniedTravelInfo*
AccompaniedTravelHistoricalDAO::get(DeleteList& del,
                                    const VendorCode& vendor,
                                    int itemNo,
                                    const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  AccompaniedTravelHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<AccompaniedTravelInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<AccompaniedTravelInfo>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<AccompaniedTravelInfo*>*
AccompaniedTravelHistoricalDAO::create(AccompaniedTravelHistoricalKey key)
{
  std::vector<AccompaniedTravelInfo*>* ret = new std::vector<AccompaniedTravelInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAccompaniedTravelInfoHistorical at(dbAdapter->getAdapter());
    at.findAccompaniedTravelInfo(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in AccompaniedTravelHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
AccompaniedTravelHistoricalDAO::destroy(AccompaniedTravelHistoricalKey key,
                                        std::vector<AccompaniedTravelInfo*>* recs)
{
  std::vector<AccompaniedTravelInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
AccompaniedTravelHistoricalDAO::_name("AccompaniedTravelHistorical");
std::string
AccompaniedTravelHistoricalDAO::_cacheClass("Rules");
DAOHelper<AccompaniedTravelHistoricalDAO>
AccompaniedTravelHistoricalDAO::_helper(_name);
AccompaniedTravelHistoricalDAO* AccompaniedTravelHistoricalDAO::_instance = nullptr;

} // namespace tse
