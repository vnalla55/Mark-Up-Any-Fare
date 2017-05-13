//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/EndOnEndDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/EndOnEnd.h"
#include "DBAccess/Queries/QueryGetEndOnEnd.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
EndOnEndDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.EndOnEndDAO"));

EndOnEndDAO&
EndOnEndDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const EndOnEnd*
getEndOnEndData(const VendorCode& vendor,
                int itemNo,
                DeleteList& deleteList,
                const DateTime& ticketDate,
                bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    EndOnEndHistoricalDAO& dao = EndOnEndHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    EndOnEndDAO& dao = EndOnEndDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const EndOnEnd*
EndOnEndDAO::get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  EndOnEndKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentG<EndOnEnd> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (LIKELY(isCurrent(*iter) && NotInhibit<EndOnEnd>(*iter)))
      return *iter;
  }
  return nullptr;
}

std::vector<const EndOnEnd*>*
EndOnEndDAO::create(EndOnEndKey key)
{
  std::vector<const EndOnEnd*>* ret = new std::vector<const EndOnEnd*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetEndOnEnd eoe(dbAdapter->getAdapter());
    eoe.findEndOnEnd(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in EndOnEndDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
EndOnEndDAO::destroy(EndOnEndKey key, std::vector<const EndOnEnd*>* recs)
{
  destroyContainer(recs);
}

EndOnEndKey
EndOnEndDAO::createKey(const EndOnEnd* info)
{
  return EndOnEndKey(info->vendor(), info->itemNo());
}

void
EndOnEndDAO::load()
{
  StartupLoaderNoDB<EndOnEnd, EndOnEndDAO>();
}

std::string
EndOnEndDAO::_name("EndOnEnd");
std::string
EndOnEndDAO::_cacheClass("Rules");
DAOHelper<EndOnEndDAO>
EndOnEndDAO::_helper(_name);
EndOnEndDAO* EndOnEndDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: EndOnEndHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
EndOnEndHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.EndOnEndHistoricalDAO"));
EndOnEndHistoricalDAO&
EndOnEndHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const EndOnEnd*
EndOnEndHistoricalDAO::get(DeleteList& del,
                           const VendorCode& vendor,
                           int itemNo,
                           const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  EndOnEndHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<EndOnEnd> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<EndOnEnd>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<const EndOnEnd*>*
EndOnEndHistoricalDAO::create(EndOnEndHistoricalKey key)
{
  std::vector<const EndOnEnd*>* ret = new std::vector<const EndOnEnd*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetEndOnEndHistorical eoe(dbAdapter->getAdapter());
    eoe.findEndOnEnd(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in EndOnEndHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
EndOnEndHistoricalDAO::destroy(EndOnEndHistoricalKey key, std::vector<const EndOnEnd*>* recs)
{
  destroyContainer(recs);
}

std::string
EndOnEndHistoricalDAO::_name("EndOnEndHistorical");
std::string
EndOnEndHistoricalDAO::_cacheClass("Rules");
DAOHelper<EndOnEndHistoricalDAO>
EndOnEndHistoricalDAO::_helper(_name);
EndOnEndHistoricalDAO* EndOnEndHistoricalDAO::_instance = nullptr;

} // namespace tse
