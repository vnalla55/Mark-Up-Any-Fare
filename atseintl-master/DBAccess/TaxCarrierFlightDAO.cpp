//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/TaxCarrierFlightDAO.h"

#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetTaxCarrierFlight.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <algorithm>
#include <functional>

namespace tse
{
Logger TaxCarrierFlightDAO::_logger("atseintl.DBAccess.TaxCarrierFlightDAO");

TaxCarrierFlightDAO&
TaxCarrierFlightDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const TaxCarrierFlightInfo*
getTaxCarrierFlightData(const VendorCode& vendor,
                        int itemNo,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    TaxCarrierFlightHistoricalDAO& dao = TaxCarrierFlightHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    TaxCarrierFlightDAO& dao = TaxCarrierFlightDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const TaxCarrierFlightInfo*
TaxCarrierFlightDAO::get(DeleteList& del,
                         const VendorCode& vendor,
                         int itemNo,
                         const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  TaxCarrierFlightKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentG<TaxCarrierFlightInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (LIKELY(isCurrent(*iter)))
      return *iter;
  }
  return nullptr;
}

std::vector<TaxCarrierFlightInfo*>*
TaxCarrierFlightDAO::create(TaxCarrierFlightKey key)
{
  std::vector<TaxCarrierFlightInfo*>* ret = new std::vector<TaxCarrierFlightInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTaxCarrierFlight cf(dbAdapter->getAdapter());
    cf.findCarrierFlight(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxCarrierFlightDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TaxCarrierFlightDAO::destroy(TaxCarrierFlightKey key, std::vector<TaxCarrierFlightInfo*>* recs)
{
  keyRemoved(key);
  std::vector<TaxCarrierFlightInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

size_t
TaxCarrierFlightDAO::clear()
{
  cacheCleared();
  size_t result(cache().clear());
  LOG4CXX_INFO(_logger, "TaxCarrierFlight cache cleared");
  return result;
}

TaxCarrierFlightKey
TaxCarrierFlightDAO::createKey(TaxCarrierFlightInfo* info)
{
  return TaxCarrierFlightKey(info->vendor(), info->itemNo());
}

void
TaxCarrierFlightDAO::load()
{
  StartupLoaderNoDB<TaxCarrierFlightInfo, TaxCarrierFlightDAO>();
}

std::string
TaxCarrierFlightDAO::_name("TaxCarrierFlight");
std::string
TaxCarrierFlightDAO::_cacheClass("Rules");
DAOHelper<TaxCarrierFlightDAO>
TaxCarrierFlightDAO::_helper(_name);
TaxCarrierFlightDAO* TaxCarrierFlightDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: TaxCarrierFlightHistoricalDAO
// --------------------------------------------------
Logger TaxCarrierFlightHistoricalDAO::_logger("atseintl.DBAccess.TaxCarrierFlightHistoricalDAO");
TaxCarrierFlightHistoricalDAO&
TaxCarrierFlightHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const TaxCarrierFlightInfo*
TaxCarrierFlightHistoricalDAO::get(DeleteList& del,
                                   const VendorCode& vendor,
                                   int itemNo,
                                   const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  TaxCarrierFlightHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<TaxCarrierFlightInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<TaxCarrierFlightInfo*>*
TaxCarrierFlightHistoricalDAO::create(TaxCarrierFlightHistoricalKey key)
{
  std::vector<TaxCarrierFlightInfo*>* ret = new std::vector<TaxCarrierFlightInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTaxCarrierFlightHistorical cf(dbAdapter->getAdapter());
    cf.findCarrierFlight(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxCarrierFlightHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TaxCarrierFlightHistoricalDAO::destroy(TaxCarrierFlightHistoricalKey key,
                                       std::vector<TaxCarrierFlightInfo*>* recs)
{
  std::vector<TaxCarrierFlightInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
TaxCarrierFlightHistoricalDAO::_name("TaxCarrierFlightHistorical");
std::string
TaxCarrierFlightHistoricalDAO::_cacheClass("Rules");
DAOHelper<TaxCarrierFlightHistoricalDAO>
TaxCarrierFlightHistoricalDAO::_helper(_name);
TaxCarrierFlightHistoricalDAO* TaxCarrierFlightHistoricalDAO::_instance = nullptr;

} // namespace tse
