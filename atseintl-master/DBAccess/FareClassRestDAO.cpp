//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/FareClassRestDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareClassRestRule.h"
#include "DBAccess/Queries/QueryGetFareClassRest.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <time.h>

namespace tse
{
log4cxx::LoggerPtr
FareClassRestDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FareClassRestDAO"));

FareClassRestDAO&
FareClassRestDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareClassRestRule*>&
getFareClassRestRuleData(const VendorCode& vendor,
                         int itemNo,
                         DeleteList& deleteList,
                         const DateTime& ticketDate,
                         bool isHistorical)
{
  if (isHistorical)
  {
    FareClassRestHistoricalDAO& dao = FareClassRestHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    FareClassRestDAO& dao = FareClassRestDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const std::vector<FareClassRestRule*>&
FareClassRestDAO::get(DeleteList& del,
                      const VendorCode& vendor,
                      int itemNo,
                      const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  FareClassRestKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  /*
  del.copy(ptr);
  std::vector<FareClassRestRule*>* ret = new std::vector<FareClassRestRule*>;
  del.adopt(ret);

  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret),
                 IsNotCurrentG<FareClassRestRule>(ticketDate));

  return *ret;
  */
  return *(applyFilter(del, ptr, IsNotCurrentG<FareClassRestRule>(ticketDate)));
}

std::vector<FareClassRestRule*>*
FareClassRestDAO::create(FareClassRestKey key)
{
  std::vector<FareClassRestRule*>* ret = new std::vector<FareClassRestRule*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareClassRest fcr(dbAdapter->getAdapter());
    fcr.findFareClassRestRule(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareClassRestDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FareClassRestDAO::destroy(FareClassRestKey key, std::vector<FareClassRestRule*>* recs)
{
  std::vector<FareClassRestRule*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

FareClassRestKey
FareClassRestDAO::createKey(FareClassRestRule* info)
{
  return FareClassRestKey(info->vendor(), info->itemNo());
}

void
FareClassRestDAO::load()
{
  StartupLoaderNoDB<FareClassRestRule, FareClassRestDAO>();
}

std::string
FareClassRestDAO::_name("FareClassRest");
std::string
FareClassRestDAO::_cacheClass("Rules");

DAOHelper<FareClassRestDAO>
FareClassRestDAO::_helper(_name);

FareClassRestDAO* FareClassRestDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: FareClassRestHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
FareClassRestHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareClassRestHistoricalDAO"));
FareClassRestHistoricalDAO&
FareClassRestHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareClassRestRule*>&
FareClassRestHistoricalDAO::get(DeleteList& del,
                                const VendorCode& vendor,
                                int itemNo,
                                const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  FareClassRestHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<FareClassRestRule*>* ret = new std::vector<FareClassRestRule*>;
  del.adopt(ret);

  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotCurrentH<FareClassRestRule>(ticketDate));

  return *ret;
}

std::vector<FareClassRestRule*>*
FareClassRestHistoricalDAO::create(FareClassRestHistoricalKey key)
{
  std::vector<FareClassRestRule*>* ret = new std::vector<FareClassRestRule*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareClassRestHistorical fcr(dbAdapter->getAdapter());
    fcr.findFareClassRestRule(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareClassRestHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FareClassRestHistoricalDAO::destroy(FareClassRestHistoricalKey key,
                                    std::vector<FareClassRestRule*>* recs)
{
  std::vector<FareClassRestRule*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
FareClassRestHistoricalDAO::_name("FareClassRestHistorical");
std::string
FareClassRestHistoricalDAO::_cacheClass("Fares");
DAOHelper<FareClassRestHistoricalDAO>
FareClassRestHistoricalDAO::_helper(_name);
FareClassRestHistoricalDAO* FareClassRestHistoricalDAO::_instance = nullptr;

} // namespace tse
