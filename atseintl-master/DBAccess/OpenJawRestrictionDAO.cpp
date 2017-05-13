//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/OpenJawRestrictionDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/OpenJawRestriction.h"
#include "DBAccess/Queries/QueryGetOpenJawRestriction.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

namespace tse
{
log4cxx::LoggerPtr
OpenJawRestrictionDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.OpenJawRestrictionDAO"));

OpenJawRestrictionDAO&
OpenJawRestrictionDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<OpenJawRestriction*>&
getOpenJawRestrictionData(const VendorCode& vendor,
                          int itemNo,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical)
{
  if (isHistorical)
  {
    OpenJawRestrictionHistoricalDAO& dao = OpenJawRestrictionHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    OpenJawRestrictionDAO& dao = OpenJawRestrictionDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const std::vector<OpenJawRestriction*>&
OpenJawRestrictionDAO::get(DeleteList& del,
                           const VendorCode& vendor,
                           int itemNo,
                           const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  OpenJawRestrictionKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  /*
  del.copy(ptr);

  std::vector<OpenJawRestriction*>* ret = new std::vector<OpenJawRestriction*>;
  del.adopt(ret);

  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret),
                 IsNotCurrentG<OpenJawRestriction>(ticketDate));

  return *ret;
  */
  return *(applyFilter(del, ptr, IsNotCurrentG<OpenJawRestriction>(ticketDate)));
}

std::vector<OpenJawRestriction*>*
OpenJawRestrictionDAO::create(OpenJawRestrictionKey key)
{
  std::vector<OpenJawRestriction*>* ret = new std::vector<OpenJawRestriction*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetOpenJawRestriction ojr(dbAdapter->getAdapter());
    ojr.findOpenJawRestriction(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in OpenJawRestrictionDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
OpenJawRestrictionDAO::destroy(OpenJawRestrictionKey key, std::vector<OpenJawRestriction*>* recs)
{
  std::vector<OpenJawRestriction*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
OpenJawRestrictionDAO::_name("OpenJawRestriction");
std::string
OpenJawRestrictionDAO::_cacheClass("Rules");
DAOHelper<OpenJawRestrictionDAO>
OpenJawRestrictionDAO::_helper(_name);
OpenJawRestrictionDAO* OpenJawRestrictionDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: OpenJawRestrictionHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
OpenJawRestrictionHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.OpenJawRestrictionHistoricalDAO"));
OpenJawRestrictionHistoricalDAO&
OpenJawRestrictionHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<OpenJawRestriction*>&
OpenJawRestrictionHistoricalDAO::get(DeleteList& del,
                                     const VendorCode& vendor,
                                     int itemNo,
                                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  OpenJawRestrictionHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<OpenJawRestriction*>* ret = new std::vector<OpenJawRestriction*>;
  del.adopt(ret);

  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotCurrentH<OpenJawRestriction>(ticketDate));

  return *ret;
}

std::vector<OpenJawRestriction*>*
OpenJawRestrictionHistoricalDAO::create(OpenJawRestrictionHistoricalKey key)
{
  std::vector<OpenJawRestriction*>* ret = new std::vector<OpenJawRestriction*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetOpenJawRestrictionHistorical ojr(dbAdapter->getAdapter());
    ojr.findOpenJawRestriction(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in OpenJawRestrictionHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
OpenJawRestrictionHistoricalDAO::destroy(OpenJawRestrictionHistoricalKey key,
                                         std::vector<OpenJawRestriction*>* recs)
{
  std::vector<OpenJawRestriction*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
OpenJawRestrictionHistoricalDAO::_name("OpenJawRestrictionHistorical");
std::string
OpenJawRestrictionHistoricalDAO::_cacheClass("Rules");
DAOHelper<OpenJawRestrictionHistoricalDAO>
OpenJawRestrictionHistoricalDAO::_helper(_name);
OpenJawRestrictionHistoricalDAO* OpenJawRestrictionHistoricalDAO::_instance = nullptr;

} // namespace tse
