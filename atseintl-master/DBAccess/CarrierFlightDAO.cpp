//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/CarrierFlightDAO.h"

#include "Common/Logger.h"
#include "DBAccess/CarrierFlight.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetCarrierFlight.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
CarrierFlightDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.CarrierFlightDAO"));

CarrierFlightDAO&
CarrierFlightDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const CarrierFlight*
getCarrierFlightData(const VendorCode& vendor,
                     int itemNo,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    CarrierFlightHistoricalDAO& dao = CarrierFlightHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    CarrierFlightDAO& dao = CarrierFlightDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const CarrierFlight*
CarrierFlightDAO::get(DeleteList& del,
                      const VendorCode& vendor,
                      int itemNo,
                      const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  CarrierFlightKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentG<CarrierFlight> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (LIKELY(isCurrent(*iter) && NotInhibit<CarrierFlight>(*iter)))
      return *iter;
  }
  return nullptr;
}

std::vector<CarrierFlight*>*
CarrierFlightDAO::create(CarrierFlightKey key)
{
  std::vector<CarrierFlight*>* ret = new std::vector<CarrierFlight*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCarrierFlight cf(dbAdapter->getAdapter());
    cf.findCarrierFlight(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CarrierFlightDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CarrierFlightDAO::destroy(CarrierFlightKey key, std::vector<CarrierFlight*>* recs)
{
  std::vector<CarrierFlight*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

CarrierFlightKey
CarrierFlightDAO::createKey(CarrierFlight* info)
{
  return CarrierFlightKey(info->vendor(), info->itemNo());
}

void
CarrierFlightDAO::load()
{
  StartupLoaderNoDB<CarrierFlight, CarrierFlightDAO>();
}

sfc::CompressedData*
CarrierFlightDAO::compress(const std::vector<CarrierFlight*>* vect) const
{
  return compressVector(vect);
}

std::vector<CarrierFlight*>*
CarrierFlightDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<CarrierFlight>(compressed);
}

std::string
CarrierFlightDAO::_name("CarrierFlight");
std::string
CarrierFlightDAO::_cacheClass("Rules");
DAOHelper<CarrierFlightDAO>
CarrierFlightDAO::_helper(_name);
CarrierFlightDAO* CarrierFlightDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: CarrierFlightHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
CarrierFlightHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.CarrierFlightHistoricalDAO"));
CarrierFlightHistoricalDAO&
CarrierFlightHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const CarrierFlight*
CarrierFlightHistoricalDAO::get(DeleteList& del,
                                const VendorCode& vendor,
                                int itemNo,
                                const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  CarrierFlightHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<CarrierFlight> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<CarrierFlight>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<CarrierFlight*>*
CarrierFlightHistoricalDAO::create(CarrierFlightHistoricalKey key)
{
  std::vector<CarrierFlight*>* ret = new std::vector<CarrierFlight*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCarrierFlightHistorical cf(dbAdapter->getAdapter());
    cf.findCarrierFlight(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CarrierFlightHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CarrierFlightHistoricalDAO::destroy(CarrierFlightHistoricalKey key,
                                    std::vector<CarrierFlight*>* recs)
{
  std::vector<CarrierFlight*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
CarrierFlightHistoricalDAO::compress(const std::vector<CarrierFlight*>* vect) const
{
  return compressVector(vect);
}

std::vector<CarrierFlight*>*
CarrierFlightHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<CarrierFlight>(compressed);
}

std::string
CarrierFlightHistoricalDAO::_name("CarrierFlightHistorical");
std::string
CarrierFlightHistoricalDAO::_cacheClass("Rules");
DAOHelper<CarrierFlightHistoricalDAO>
CarrierFlightHistoricalDAO::_helper(_name);
CarrierFlightHistoricalDAO* CarrierFlightHistoricalDAO::_instance = nullptr;

} // namespace tse
