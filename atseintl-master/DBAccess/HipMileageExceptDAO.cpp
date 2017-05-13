//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/HipMileageExceptDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/HipMileageExceptInfo.h"
#include "DBAccess/Queries/QueryGetHipMileageExcept.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
HipMileageExceptDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.HipMileageExceptDAO"));

HipMileageExceptDAO&
HipMileageExceptDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const HipMileageExceptInfo*
getHipMileageExceptData(const VendorCode& vendor,
                        int itemNo,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    HipMileageExceptHistoricalDAO& dao = HipMileageExceptHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    HipMileageExceptDAO& dao = HipMileageExceptDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const HipMileageExceptInfo*
HipMileageExceptDAO::get(DeleteList& del,
                         const VendorCode& vendor,
                         int itemNo,
                         const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  HipMileageExceptKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentG<HipMileageExceptInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (LIKELY(isCurrent(*iter) && NotInhibit<HipMileageExceptInfo>(*iter)))
      return *iter;
  }
  return nullptr;
}

std::vector<const HipMileageExceptInfo*>*
HipMileageExceptDAO::create(HipMileageExceptKey key)
{
  std::vector<const HipMileageExceptInfo*>* ret = new std::vector<const HipMileageExceptInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetHipMileageExcept hme(dbAdapter->getAdapter());
    hme.findHipMileageExcept(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in HipMileageExceptDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
HipMileageExceptDAO::destroy(HipMileageExceptKey key,
                             std::vector<const HipMileageExceptInfo*>* recs)
{
  std::vector<const HipMileageExceptInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i; // lint !e605
  delete recs;
}

std::string
HipMileageExceptDAO::_name("HipMileageExcept");
std::string
HipMileageExceptDAO::_cacheClass("MinFares");
DAOHelper<HipMileageExceptDAO>
HipMileageExceptDAO::_helper(_name);
HipMileageExceptDAO* HipMileageExceptDAO::_instance = nullptr;

HipMileageExceptKey
HipMileageExceptDAO::createKey(const HipMileageExceptInfo* info)
{
  return HipMileageExceptKey(info->vendor(), info->itemNo());
}

void
HipMileageExceptDAO::load()
{
  StartupLoaderNoDB<HipMileageExceptInfo, HipMileageExceptDAO>();
}

// --------------------------------------------------
// Historical DAO: HipMileageExceptHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
HipMileageExceptHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.HipMileageExceptHistoricalDAO"));
HipMileageExceptHistoricalDAO&
HipMileageExceptHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const HipMileageExceptInfo*
HipMileageExceptHistoricalDAO::get(DeleteList& del,
                                   const VendorCode& vendor,
                                   int itemNo,
                                   const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  HipMileageExceptHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<HipMileageExceptInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<HipMileageExceptInfo>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<const HipMileageExceptInfo*>*
HipMileageExceptHistoricalDAO::create(HipMileageExceptHistoricalKey key)
{
  std::vector<const HipMileageExceptInfo*>* ret = new std::vector<const HipMileageExceptInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetHipMileageExceptHistorical hme(dbAdapter->getAdapter());
    hme.findHipMileageExcept(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in HipMileageExceptHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
HipMileageExceptHistoricalDAO::destroy(HipMileageExceptHistoricalKey key,
                                       std::vector<const HipMileageExceptInfo*>* recs)
{
  std::vector<const HipMileageExceptInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
HipMileageExceptHistoricalDAO::_name("HipMileageExceptHistorical");
std::string
HipMileageExceptHistoricalDAO::_cacheClass("MinFares");
DAOHelper<HipMileageExceptHistoricalDAO>
HipMileageExceptHistoricalDAO::_helper(_name);
HipMileageExceptHistoricalDAO* HipMileageExceptHistoricalDAO::_instance = nullptr;

} // namespace tse
