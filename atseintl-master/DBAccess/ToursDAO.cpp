//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/ToursDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetTours.h"
#include "DBAccess/Tours.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

namespace tse
{
log4cxx::LoggerPtr
ToursDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.ToursDAO"));
ToursDAO&
ToursDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const Tours*
getToursData(const VendorCode& vendor,
             int itemNo,
             DeleteList& deleteList,
             const DateTime& ticketDate,
             bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    ToursHistoricalDAO& dao = ToursHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    ToursDAO& dao = ToursDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const Tours*
ToursDAO::get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  ToursKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (UNLIKELY(ptr->empty()))
    return nullptr;

  del.copy(ptr);

  IsCurrentG<Tours> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (LIKELY(isCurrent(*iter) && NotInhibit<Tours>(*iter)))
      return *iter;
  }
  return nullptr;
}

std::vector<Tours*>*
ToursDAO::create(ToursKey key)
{
  std::vector<Tours*>* ret = new std::vector<Tours*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTours trs(dbAdapter->getAdapter());
    trs.findTours(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in ToursDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
ToursDAO::destroy(ToursKey key, std::vector<Tours*>* recs)
{
  std::vector<Tours*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
ToursDAO::_name("Tours");
std::string
ToursDAO::_cacheClass("Rules");
DAOHelper<ToursDAO>
ToursDAO::_helper(_name);
ToursDAO* ToursDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: ToursHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
ToursHistoricalDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.ToursHistoricalDAO"));
ToursHistoricalDAO&
ToursHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const Tours*
ToursHistoricalDAO::get(DeleteList& del,
                        const VendorCode& vendor,
                        int itemNo,
                        const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  ToursHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<Tours> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<Tours>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<Tours*>*
ToursHistoricalDAO::create(ToursHistoricalKey key)
{
  std::vector<Tours*>* ret = new std::vector<Tours*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetToursHistorical trs(dbAdapter->getAdapter());
    trs.findTours(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in ToursHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
ToursHistoricalDAO::destroy(ToursHistoricalKey key, std::vector<Tours*>* recs)
{
  std::vector<Tours*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
ToursHistoricalDAO::_name("ToursHistorical");
std::string
ToursHistoricalDAO::_cacheClass("Rules");
DAOHelper<ToursHistoricalDAO>
ToursHistoricalDAO::_helper(_name);
ToursHistoricalDAO* ToursHistoricalDAO::_instance = nullptr;

} // namespace tse
