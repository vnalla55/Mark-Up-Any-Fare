//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/NegFareCalcDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/NegFareCalcInfo.h"
#include "DBAccess/Queries/QueryGetNegFareCalcInfo.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <algorithm>
#include <functional>

namespace tse
{

log4cxx::LoggerPtr
NegFareCalcDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.NegFareCalcDAO"));

NegFareCalcDAO&
NegFareCalcDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<NegFareCalcInfo*>&
getNegFareCalcData(const VendorCode& vendor,
                   int itemNo,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical)
{
  if (isHistorical)
  {
    NegFareCalcHistoricalDAO& dao = NegFareCalcHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    NegFareCalcDAO& dao = NegFareCalcDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const std::vector<NegFareCalcInfo*>&
NegFareCalcDAO::get(DeleteList& del,
                    const VendorCode& vendor,
                    int itemNo,
                    const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  NegFareCalcKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  /*
  del.copy(ptr);

  std::vector<NegFareCalcInfo*>* ret = new std::vector<NegFareCalcInfo*>;
  del.adopt(ret);

  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret),
                 IsNotCurrentG<NegFareCalcInfo>(ticketDate));

  return *ret;
  */
  return *(applyFilter(del, ptr, IsNotCurrentG<NegFareCalcInfo>(ticketDate)));
}

std::vector<NegFareCalcInfo*>*
NegFareCalcDAO::create(NegFareCalcKey key)
{
  std::vector<NegFareCalcInfo*>* ret = new std::vector<NegFareCalcInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetNegFareCalcInfo nfc(dbAdapter->getAdapter());
    nfc.findNegFareCalcInfo(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in NegFareCalcDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
NegFareCalcDAO::destroy(NegFareCalcKey key, std::vector<NegFareCalcInfo*>* recs)
{
  std::vector<NegFareCalcInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
NegFareCalcDAO::_name("NegFareCalc");
std::string
NegFareCalcDAO::_cacheClass("Rules");
DAOHelper<NegFareCalcDAO>
NegFareCalcDAO::_helper(_name);
NegFareCalcDAO* NegFareCalcDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: NegFareCalcHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
NegFareCalcHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.NegFareCalcHistoricalDAO"));
NegFareCalcHistoricalDAO&
NegFareCalcHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<NegFareCalcInfo*>&
NegFareCalcHistoricalDAO::get(DeleteList& del,
                              const VendorCode& vendor,
                              int itemNo,
                              const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  NegFareCalcHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<NegFareCalcInfo*>* ret = new std::vector<NegFareCalcInfo*>;
  del.adopt(ret);

  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotCurrentH<NegFareCalcInfo>(ticketDate));

  return *ret;
}

std::vector<NegFareCalcInfo*>*
NegFareCalcHistoricalDAO::create(NegFareCalcHistoricalKey key)
{
  std::vector<NegFareCalcInfo*>* ret = new std::vector<NegFareCalcInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetNegFareCalcInfoHistorical nfc(dbAdapter->getAdapter());
    nfc.findNegFareCalcInfo(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in NegFareCalcHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
NegFareCalcHistoricalDAO::destroy(NegFareCalcHistoricalKey key, std::vector<NegFareCalcInfo*>* recs)
{
  std::vector<NegFareCalcInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
NegFareCalcHistoricalDAO::_name("NegFareCalcHistorical");
std::string
NegFareCalcHistoricalDAO::_cacheClass("Rules");
DAOHelper<NegFareCalcHistoricalDAO>
NegFareCalcHistoricalDAO::_helper(_name);
NegFareCalcHistoricalDAO* NegFareCalcHistoricalDAO::_instance = nullptr;

} // namespace tse
