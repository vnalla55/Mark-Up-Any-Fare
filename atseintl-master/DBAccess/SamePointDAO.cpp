//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/SamePointDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetTable993.h"
#include "DBAccess/SamePoint.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

namespace tse
{
log4cxx::LoggerPtr
SamePointDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.SamePointDAO"));
SamePointDAO&
SamePointDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<const SamePoint*>&
getSamePointData(const VendorCode& vendor,
                 int itemNo,
                 DeleteList& deleteList,
                 const DateTime& ticketDate,
                 bool isHistorical)
{
  if (isHistorical)
  {
    SamePointHistoricalDAO& dao = SamePointHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    SamePointDAO& dao = SamePointDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const std::vector<const SamePoint*>&
SamePointDAO::get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  SamePointKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  return *(applyFilter(del, ptr, IsNotCurrentG<SamePoint>(ticketDate)));
}

std::vector<const SamePoint*>*
SamePointDAO::create(SamePointKey key)
{
  std::vector<const SamePoint*>* ret = new std::vector<const SamePoint*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTable993 sp(dbAdapter->getAdapter());
    sp.findSamePoint(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SamePointDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SamePointDAO::destroy(SamePointKey key, std::vector<const SamePoint*>* recs)
{
  std::vector<const SamePoint*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i; // lint !e605
  delete recs;
}

std::string
SamePointDAO::_name("SamePoint");
std::string
SamePointDAO::_cacheClass("Rules");
DAOHelper<SamePointDAO>
SamePointDAO::_helper(_name);
SamePointDAO* SamePointDAO::_instance = nullptr;

SamePointKey
SamePointDAO::createKey(const SamePoint* info)
{
  return SamePointKey(info->vendor(), info->itemNo());
}

void
SamePointDAO::load()
{
  StartupLoaderNoDB<SamePoint, SamePointDAO>();
}

// --------------------------------------------------
// Historical DAO: SamePointHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
SamePointHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SamePointHistoricalDAO"));
SamePointHistoricalDAO&
SamePointHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<const SamePoint*>&
SamePointHistoricalDAO::get(DeleteList& del,
                            const VendorCode& vendor,
                            int itemNo,
                            const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  SamePointHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  // if all of the SamePoint objects are current, then we return the vector from the cache.
  IsNotCurrentH<SamePoint> isNotCurrent(ticketDate);
  std::vector<const SamePoint*>::iterator firstNotCurrent =
      std::find_if(ptr->begin(), ptr->end(), isNotCurrent);
  if (firstNotCurrent == ptr->end())
  {
    return *ptr;
  }

  // some of the SamePoint objects aren't current, so we have to create a new vector to
  // return and copy all the current objects into it and return them.
  std::vector<const SamePoint*>* ret =
      new std::vector<const SamePoint*>(ptr->begin(), firstNotCurrent);
  del.adopt(ret);

  remove_copy_if(firstNotCurrent + 1, ptr->end(), back_inserter(*ret), isNotCurrent);

  return *ret;
}

std::vector<const SamePoint*>*
SamePointHistoricalDAO::create(SamePointHistoricalKey key)
{
  std::vector<const SamePoint*>* ret = new std::vector<const SamePoint*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTable993Historical sp(dbAdapter->getAdapter());
    sp.findSamePoint(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SamePointHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
SamePointHistoricalDAO::destroy(SamePointHistoricalKey key, std::vector<const SamePoint*>* recs)
{
  std::vector<const SamePoint*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
SamePointHistoricalDAO::_name("SamePointHistorical");
std::string
SamePointHistoricalDAO::_cacheClass("Rules");
DAOHelper<SamePointHistoricalDAO>
SamePointHistoricalDAO::_helper(_name);
SamePointHistoricalDAO* SamePointHistoricalDAO::_instance = nullptr;

} // namespace tse
