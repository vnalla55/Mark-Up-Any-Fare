//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/CarrierApplicationDAO.h"

#include "Common/Logger.h"
#include "DBAccess/CarrierApplicationInfo.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetCarrierApplicationInfo.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
CarrierApplicationDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.CarrierApplicationDAO"));

CarrierApplicationDAO&
CarrierApplicationDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<CarrierApplicationInfo*>&
getCarrierApplicationData(const VendorCode& vendor,
                          int itemNo,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical,
                          const DateTime& applDate)
{
  if (isHistorical)
  {
    CarrierApplicationHistoricalDAO& dao = CarrierApplicationHistoricalDAO::instance();
    if (applDate != DateTime::emptyDate())
    {
      const std::vector<CarrierApplicationInfo*>& ret =
          dao.get(deleteList, vendor, itemNo, applDate);
      if (ret.size() > 0)
        return ret;
    }
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    CarrierApplicationDAO& dao = CarrierApplicationDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const std::vector<CarrierApplicationInfo*>&
CarrierApplicationDAO::get(DeleteList& del,
                           const VendorCode& vendor,
                           int itemNo,
                           const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  CarrierApplicationKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<CarrierApplicationInfo*>* ret = new std::vector<CarrierApplicationInfo*>;
  del.adopt(ret);

  IsCurrentG<CarrierApplicationInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (LIKELY(isCurrent(*iter) && NotInhibit<CarrierApplicationInfo>(*iter)))
      ret->push_back(*iter);
  }

  return *ret;
}

std::vector<CarrierApplicationInfo*>*
CarrierApplicationDAO::create(CarrierApplicationKey key)
{
  std::vector<CarrierApplicationInfo*>* ret = new std::vector<CarrierApplicationInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCarrierApplicationInfo cai(dbAdapter->getAdapter());
    cai.findCarrierApplicationInfo(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CarrierApplicationDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CarrierApplicationDAO::destroy(CarrierApplicationKey key,
                               std::vector<CarrierApplicationInfo*>* recs)
{
  std::vector<CarrierApplicationInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
CarrierApplicationDAO::_name("CarrierApplication");
std::string
CarrierApplicationDAO::_cacheClass("Rules");
DAOHelper<CarrierApplicationDAO>
CarrierApplicationDAO::_helper(_name);
CarrierApplicationDAO* CarrierApplicationDAO::_instance = nullptr;

CarrierApplicationKey
CarrierApplicationDAO::createKey(const CarrierApplicationInfo* info)
{
  return CarrierApplicationKey(info->vendor(), info->itemNo());
}

void
CarrierApplicationDAO::load()
{
  StartupLoaderNoDB<CarrierApplicationInfo, CarrierApplicationDAO>();
}

// --------------------------------------------------
// Historical DAO: CarrierApplicationHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
CarrierApplicationHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.CarrierApplicationHistoricalDAO"));
CarrierApplicationHistoricalDAO&
CarrierApplicationHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<CarrierApplicationInfo*>&
CarrierApplicationHistoricalDAO::get(DeleteList& del,
                                     const VendorCode& vendor,
                                     int itemNo,
                                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  CarrierApplicationHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<CarrierApplicationInfo*>* ret = new std::vector<CarrierApplicationInfo*>;
  del.adopt(ret);

  IsCurrentH<CarrierApplicationInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<CarrierApplicationInfo>(*iter))
      ret->push_back(*iter);
  }

  return *ret;
}

std::vector<CarrierApplicationInfo*>*
CarrierApplicationHistoricalDAO::create(CarrierApplicationHistoricalKey key)
{
  std::vector<CarrierApplicationInfo*>* ret = new std::vector<CarrierApplicationInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCarrierApplicationInfoHistorical cai(dbAdapter->getAdapter());
    cai.findCarrierApplicationInfo(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CarrierApplicationHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CarrierApplicationHistoricalDAO::destroy(CarrierApplicationHistoricalKey key,
                                         std::vector<CarrierApplicationInfo*>* recs)
{
  std::vector<CarrierApplicationInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
CarrierApplicationHistoricalDAO::_name("CarrierApplicationHistorical");
std::string
CarrierApplicationHistoricalDAO::_cacheClass("Rules");
DAOHelper<CarrierApplicationHistoricalDAO>
CarrierApplicationHistoricalDAO::_helper(_name);
CarrierApplicationHistoricalDAO* CarrierApplicationHistoricalDAO::_instance = nullptr;

} // namespace tse
