//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/VisitAnotherCountryDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetVisitAnotherCountry.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"
#include "DBAccess/VisitAnotherCountry.h"

namespace tse
{
log4cxx::LoggerPtr
VisitAnotherCountryDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.VisitAnotherCountryDAO"));
VisitAnotherCountryDAO&
VisitAnotherCountryDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const VisitAnotherCountry*
getVisitAnotherCountryData(const VendorCode& vendor,
                           int itemNo,
                           DeleteList& deleteList,
                           const DateTime& ticketDate,
                           bool isHistorical)
{
  if (isHistorical)
  {
    VisitAnotherCountryHistoricalDAO& dao = VisitAnotherCountryHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    VisitAnotherCountryDAO& dao = VisitAnotherCountryDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const VisitAnotherCountry*
VisitAnotherCountryDAO::get(DeleteList& del,
                            const VendorCode& vendor,
                            int itemNo,
                            const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  VisitAnotherCountryKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentG<VisitAnotherCountry> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<VisitAnotherCountry>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<VisitAnotherCountry*>*
VisitAnotherCountryDAO::create(VisitAnotherCountryKey key)
{
  std::vector<VisitAnotherCountry*>* ret = new std::vector<VisitAnotherCountry*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetVisitAnotherCountry vac(dbAdapter->getAdapter());
    vac.findVisitAnotherCountry(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in VisitAnotherCountryDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
VisitAnotherCountryDAO::destroy(VisitAnotherCountryKey key, std::vector<VisitAnotherCountry*>* recs)
{
  std::vector<VisitAnotherCountry*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
VisitAnotherCountryDAO::_name("VisitAnotherCountry");
std::string
VisitAnotherCountryDAO::_cacheClass("Rules");
DAOHelper<VisitAnotherCountryDAO>
VisitAnotherCountryDAO::_helper(_name);
VisitAnotherCountryDAO* VisitAnotherCountryDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: VisitAnotherCountryHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
VisitAnotherCountryHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.VisitAnotherCountryHistoricalDAO"));
VisitAnotherCountryHistoricalDAO&
VisitAnotherCountryHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const VisitAnotherCountry*
VisitAnotherCountryHistoricalDAO::get(DeleteList& del,
                                      const VendorCode& vendor,
                                      int itemNo,
                                      const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  VisitAnotherCountryHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<VisitAnotherCountry> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<VisitAnotherCountry>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<VisitAnotherCountry*>*
VisitAnotherCountryHistoricalDAO::create(VisitAnotherCountryHistoricalKey key)
{
  std::vector<VisitAnotherCountry*>* ret = new std::vector<VisitAnotherCountry*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetVisitAnotherCountryHistorical vac(dbAdapter->getAdapter());
    vac.findVisitAnotherCountry(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in VisitAnotherCountryHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
VisitAnotherCountryHistoricalDAO::destroy(VisitAnotherCountryHistoricalKey key,
                                          std::vector<VisitAnotherCountry*>* recs)
{
  std::vector<VisitAnotherCountry*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
VisitAnotherCountryHistoricalDAO::_name("VisitAnotherCountryHistorical");
std::string
VisitAnotherCountryHistoricalDAO::_cacheClass("Rules");
DAOHelper<VisitAnotherCountryHistoricalDAO>
VisitAnotherCountryHistoricalDAO::_helper(_name);
VisitAnotherCountryHistoricalDAO* VisitAnotherCountryHistoricalDAO::_instance = nullptr;

} // namespace tse
