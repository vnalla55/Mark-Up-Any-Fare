//-----------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly
// prohibited.
//-----------------------------------------------------------------------------

#include "DBAccess/VoluntaryRefundsDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetVoluntaryRefundsInfo.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"
#include "DBAccess/VoluntaryRefundsInfo.h"

namespace tse
{
log4cxx::LoggerPtr
VoluntaryRefundsDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.VoluntaryRefundsDAO"));

VoluntaryRefundsDAO&
VoluntaryRefundsDAO::instance()
{
  if (_instance == nullptr)
    _helper.init();

  return *_instance;
}

const VoluntaryRefundsInfo*
getVoluntaryRefundsData(const VendorCode& vendor,
                        int itemNo,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical,
                        const DateTime& applDate)
{
  if (isHistorical)
  {
    VoluntaryRefundsHistoricalDAO& dao = VoluntaryRefundsHistoricalDAO::instance();
    if (applDate != DateTime::emptyDate())
    {
      const VoluntaryRefundsInfo* vci = dao.get(deleteList, vendor, itemNo, applDate);
      if (vci)
        return vci;
    }
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    VoluntaryRefundsDAO& dao = VoluntaryRefundsDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const VoluntaryRefundsInfo*
VoluntaryRefundsDAO::get(DeleteList& del,
                         const VendorCode& vendor,
                         int itemNo,
                         const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  DAOCache::pointer_type ptr = cache().get(VoluntaryRefundsKey(rcVendor, itemNo));
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentG<VoluntaryRefundsInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<VoluntaryRefundsInfo>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<VoluntaryRefundsInfo*>*
VoluntaryRefundsDAO::create(VoluntaryRefundsKey key)
{
  std::vector<VoluntaryRefundsInfo*>* ret = new std::vector<VoluntaryRefundsInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(_cacheClass);
  try
  {
    QueryGetVoluntaryRefundsInfo vr(dbAdapter->getAdapter());
    vr.find(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in VoluntaryRefundsDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
VoluntaryRefundsDAO::destroy(VoluntaryRefundsKey key, std::vector<VoluntaryRefundsInfo*>* vr)
{
  std::vector<VoluntaryRefundsInfo*>::iterator i;
  for (i = vr->begin(); i != vr->end(); ++i)
    delete *i;
  delete vr;
}

std::string
VoluntaryRefundsDAO::_name("VoluntaryRefunds");
std::string
VoluntaryRefundsDAO::_cacheClass("Rules");
DAOHelper<VoluntaryRefundsDAO>
VoluntaryRefundsDAO::_helper(_name);
VoluntaryRefundsDAO* VoluntaryRefundsDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: VoluntaryRefundsHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
VoluntaryRefundsHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.VoluntaryRefundsHistoricalDAO"));

VoluntaryRefundsHistoricalDAO&
VoluntaryRefundsHistoricalDAO::instance()
{
  if (_instance == nullptr)
    _helper.init();

  return *_instance;
}

const VoluntaryRefundsInfo*
VoluntaryRefundsHistoricalDAO::get(DeleteList& del,
                                   const VendorCode& vendor,
                                   int itemNo,
                                   const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  VoluntaryRefundsHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<VoluntaryRefundsInfo> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<VoluntaryRefundsInfo>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<VoluntaryRefundsInfo*>*
VoluntaryRefundsHistoricalDAO::create(VoluntaryRefundsHistoricalKey key)
{
  std::vector<VoluntaryRefundsInfo*>* ret = new std::vector<VoluntaryRefundsInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(_cacheClass);
  try
  {
    QueryGetVoluntaryRefundsInfoHistorical vr(dbAdapter->getAdapter());
    vr.find(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in VoluntaryRefundsHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
VoluntaryRefundsHistoricalDAO::destroy(VoluntaryRefundsHistoricalKey key,
                                       std::vector<VoluntaryRefundsInfo*>* vr)
{
  std::vector<VoluntaryRefundsInfo*>::iterator i;
  for (i = vr->begin(); i != vr->end(); ++i)
    delete *i;
  delete vr;
}

std::string
VoluntaryRefundsHistoricalDAO::_name("VoluntaryRefundsHistorical");
std::string
VoluntaryRefundsHistoricalDAO::_cacheClass("Rules");
DAOHelper<VoluntaryRefundsHistoricalDAO>
VoluntaryRefundsHistoricalDAO::_helper(_name);
VoluntaryRefundsHistoricalDAO* VoluntaryRefundsHistoricalDAO::_instance = nullptr;

} // namespace tse
