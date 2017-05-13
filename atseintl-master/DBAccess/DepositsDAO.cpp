//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/DepositsDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Deposits.h"
#include "DBAccess/Queries/QueryGetDeposits.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
DepositsDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.DepositsDAO"));

DepositsDAO&
DepositsDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const Deposits*
getDepositsData(const VendorCode& vendor,
                int itemNo,
                DeleteList& deleteList,
                const DateTime& ticketDate,
                bool isHistorical)
{
  if (isHistorical)
  {
    DepositsHistoricalDAO& dao = DepositsHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    DepositsDAO& dao = DepositsDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const Deposits*
DepositsDAO::get(DeleteList& del, const VendorCode& vendor, int itemNo, const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  DepositsKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentG<Deposits> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<Deposits>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<Deposits*>*
DepositsDAO::create(DepositsKey key)
{
  std::vector<Deposits*>* ret = new std::vector<Deposits*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetDeposits dep(dbAdapter->getAdapter());
    dep.findDeposits(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in DepositsDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
DepositsDAO::destroy(DepositsKey key, std::vector<Deposits*>* recs)
{
  std::vector<Deposits*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
DepositsDAO::_name("Deposits");
std::string
DepositsDAO::_cacheClass("Rules");
DAOHelper<DepositsDAO>
DepositsDAO::_helper(_name);
DepositsDAO* DepositsDAO::_instance = nullptr;

DepositsKey
DepositsDAO::createKey(const Deposits* info)
{
  return DepositsKey(info->vendor(), info->itemNo());
}

void
DepositsDAO::load()
{
  StartupLoaderNoDB<Deposits, DepositsDAO>();
}

// --------------------------------------------------
// Historical DAO: DepositsHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
DepositsHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.DepositsHistoricalDAO"));
DepositsHistoricalDAO&
DepositsHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const Deposits*
DepositsHistoricalDAO::get(DeleteList& del,
                           const VendorCode& vendor,
                           int itemNo,
                           const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  DepositsHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (ptr->empty())
    return nullptr;

  del.copy(ptr);

  IsCurrentH<Deposits> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<Deposits>(*iter))
      return *iter;
  }
  return nullptr;
}

std::vector<Deposits*>*
DepositsHistoricalDAO::create(DepositsHistoricalKey key)
{
  std::vector<Deposits*>* ret = new std::vector<Deposits*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetDepositsHistorical dep(dbAdapter->getAdapter());
    dep.findDeposits(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in DepositsHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
DepositsHistoricalDAO::destroy(DepositsHistoricalKey key, std::vector<Deposits*>* recs)
{
  std::vector<Deposits*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
DepositsHistoricalDAO::_name("DepositsHistorical");
std::string
DepositsHistoricalDAO::_cacheClass("Rules");
DAOHelper<DepositsHistoricalDAO>
DepositsHistoricalDAO::_helper(_name);
DepositsHistoricalDAO* DepositsHistoricalDAO::_instance = nullptr;

} // namespace tse
