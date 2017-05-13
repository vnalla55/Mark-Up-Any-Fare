//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/CarrierCombinationDAO.h"

#include "Common/Logger.h"
#include "DBAccess/CarrierCombination.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetCarrierComb.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <algorithm>
#include <functional>

using namespace std;

namespace tse
{
log4cxx::LoggerPtr
CarrierCombinationDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.CarrierCombinationDAO"));

CarrierCombinationDAO&
CarrierCombinationDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<CarrierCombination*>&
getCarrierCombinationData(const VendorCode& vendor,
                          const int itemNo,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    CarrierCombinationHistoricalDAO& dao = CarrierCombinationHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    CarrierCombinationDAO& dao = CarrierCombinationDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const std::vector<CarrierCombination*>&
CarrierCombinationDAO::get(DeleteList& del,
                           const VendorCode& vendor,
                           int itemNo,
                           const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  CarrierCombinationKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<CarrierCombination*>* ret(new std::vector<CarrierCombination*>);
  del.adopt(ret);

  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotCurrentG<CarrierCombination>(ticketDate));

  return *ret;
}

std::vector<CarrierCombination*>*
CarrierCombinationDAO::create(CarrierCombinationKey key)
{
  std::vector<CarrierCombination*>* ret = new std::vector<CarrierCombination*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCarrierComb cc(dbAdapter->getAdapter());
    cc.findCarrierCombination(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CarrierCombinationDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CarrierCombinationDAO::destroy(CarrierCombinationKey key, std::vector<CarrierCombination*>* recs)
{
  destroyContainer(recs);
}

CarrierCombinationKey
CarrierCombinationDAO::createKey(CarrierCombination* info)
{
  return CarrierCombinationKey(info->vendor(), info->itemNo());
}

void
CarrierCombinationDAO::load()
{
  StartupLoaderNoDB<CarrierCombination, CarrierCombinationDAO>();
}

std::string
CarrierCombinationDAO::_name("CarrierCombination");
std::string
CarrierCombinationDAO::_cacheClass("Rules");
DAOHelper<CarrierCombinationDAO>
CarrierCombinationDAO::_helper(_name);
CarrierCombinationDAO* CarrierCombinationDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: CarrierCombinationHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
CarrierCombinationHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.CarrierCombinationHistoricalDAO"));
CarrierCombinationHistoricalDAO&
CarrierCombinationHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<CarrierCombination*>&
CarrierCombinationHistoricalDAO::get(DeleteList& del,
                                     const VendorCode& vendor,
                                     int itemNo,
                                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  CarrierCombinationHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<CarrierCombination*>* ret(new std::vector<CarrierCombination*>);
  del.adopt(ret);

  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotCurrentH<CarrierCombination>(ticketDate));

  return *ret;
}

std::vector<CarrierCombination*>*
CarrierCombinationHistoricalDAO::create(CarrierCombinationHistoricalKey key)
{
  std::vector<CarrierCombination*>* ret = new std::vector<CarrierCombination*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCarrierCombHistorical cc(dbAdapter->getAdapter());
    cc.findCarrierCombination(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CarrierCombinationHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CarrierCombinationHistoricalDAO::destroy(CarrierCombinationHistoricalKey key,
                                         std::vector<CarrierCombination*>* recs)
{
  destroyContainer(recs);
}

std::string
CarrierCombinationHistoricalDAO::_name("CarrierCombinationHistorical");
std::string
CarrierCombinationHistoricalDAO::_cacheClass("Rules");
DAOHelper<CarrierCombinationHistoricalDAO>
CarrierCombinationHistoricalDAO::_helper(_name);
CarrierCombinationHistoricalDAO* CarrierCombinationHistoricalDAO::_instance = nullptr;

} // namespace tse
