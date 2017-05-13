//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/JointCarrierDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/JointCarrier.h"
#include "DBAccess/Queries/QueryGetJointCxr.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
JointCarrierDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.JointCarrierDAO"));

JointCarrierDAO&
JointCarrierDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<const JointCarrier*>&
getJointCarrierData(const VendorCode& vendor,
                    int itemNo,
                    const DateTime& date,
                    DeleteList& deleteList,
                    const DateTime& ticketDate,
                    bool isHistorical)
{
  if (isHistorical)
  {
    JointCarrierHistoricalDAO& dao = JointCarrierHistoricalDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    JointCarrierDAO& dao = JointCarrierDAO::instance();
    return dao.get(deleteList, vendor, itemNo, ticketDate);
  }
}

const std::vector<const JointCarrier*>&
JointCarrierDAO::get(DeleteList& del,
                     const VendorCode& vendor,
                     int itemNo,
                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, false);
  JointCarrierKey key(rcVendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<const JointCarrier*>* ret = new std::vector<const JointCarrier*>;
  del.adopt(ret);

  IsCurrentG<JointCarrier> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<JointCarrier>(*iter))
      ret->push_back(*iter);
  }
  return *ret;
}

std::vector<const JointCarrier*>*
JointCarrierDAO::create(JointCarrierKey key)
{
  std::vector<const JointCarrier*>* ret = new std::vector<const JointCarrier*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetJointCxr jc(dbAdapter->getAdapter());
    jc.findJointCarrier(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in JointCarrierDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
JointCarrierDAO::destroy(JointCarrierKey key, std::vector<const JointCarrier*>* recs)
{
  std::vector<const JointCarrier*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i; // lint !e605
  delete recs;
}

std::string
JointCarrierDAO::_name("JointCarrier");
std::string
JointCarrierDAO::_cacheClass("Rules");
DAOHelper<JointCarrierDAO>
JointCarrierDAO::_helper(_name);
JointCarrierDAO* JointCarrierDAO::_instance = nullptr;

JointCarrierKey
JointCarrierDAO::createKey(const JointCarrier* info)
{
  return JointCarrierKey(info->vendor(), info->itemNo());
}

void
JointCarrierDAO::load()
{
  StartupLoaderNoDB<JointCarrier, JointCarrierDAO>();
}

// --------------------------------------------------
// Historical DAO: JointCarrierHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
JointCarrierHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.JointCarrierHistoricalDAO"));
JointCarrierHistoricalDAO&
JointCarrierHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<const JointCarrier*>&
JointCarrierHistoricalDAO::get(DeleteList& del,
                               const VendorCode& vendor,
                               int itemNo,
                               const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode rcVendor = getRCVendorData(vendor, del, ticketDate, true);
  JointCarrierHistoricalKey key(rcVendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<const JointCarrier*>* ret = new std::vector<const JointCarrier*>;
  del.adopt(ret);

  IsCurrentH<JointCarrier> isCurrent(ticketDate);
  DAOCache::value_type::iterator iter = ptr->begin();
  for (; iter != ptr->end(); ++iter)
  {
    if (isCurrent(*iter) && NotInhibit<JointCarrier>(*iter))
      ret->push_back(*iter);
  }
  return *ret;
}

std::vector<const JointCarrier*>*
JointCarrierHistoricalDAO::create(JointCarrierHistoricalKey key)
{
  std::vector<const JointCarrier*>* ret = new std::vector<const JointCarrier*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetJointCxrHistorical jc(dbAdapter->getAdapter());
    jc.findJointCarrier(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in JointCarrierHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
JointCarrierHistoricalDAO::destroy(JointCarrierHistoricalKey key,
                                   std::vector<const JointCarrier*>* recs)
{
  std::vector<const JointCarrier*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i; // lint !e605
  delete recs;
}

std::string
JointCarrierHistoricalDAO::_name("JointCarrierHistorical");
std::string
JointCarrierHistoricalDAO::_cacheClass("Rules");
DAOHelper<JointCarrierHistoricalDAO>
JointCarrierHistoricalDAO::_helper(_name);
JointCarrierHistoricalDAO* JointCarrierHistoricalDAO::_instance = nullptr;

} // namespace tse
