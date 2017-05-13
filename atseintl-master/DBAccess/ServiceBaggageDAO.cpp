//-------------------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/ServiceBaggageDAO.h"

#include "Common/Logger.h"
#include "Common/Vendor.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetServiceBaggage.h"
#include "DBAccess/ServiceBaggageInfo.h"

namespace tse
{
log4cxx::LoggerPtr
ServiceBaggageDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.ServiceBaggageDAO"));

ServiceBaggageDAO&
ServiceBaggageDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

bool
ServiceBaggageDAO::translateKey(const ObjectKey& objectKey, ServiceBaggageKey& key) const
{
  return key.initialized =
             objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
}

void
ServiceBaggageDAO::translateKey(const ServiceBaggageKey& key, ObjectKey& objectKey) const
{
  objectKey.setValue("VENDOR", key._a);
  objectKey.setValue("ITEMNO", key._b);
}

bool
ServiceBaggageDAO::insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
{
  return DummyObjectInserter<ServiceBaggageInfo, ServiceBaggageDAO>(flatKey, objectKey).success();
}

ServiceBaggageDAO::ServiceBaggageDAO(int cacheSize /* default = 0 */,
                                     const std::string& cacheType /* default = "" */)
  : DataAccessObject<ServiceBaggageKey, std::vector<const ServiceBaggageInfo*> >(cacheSize,
                                                                                 cacheType)
{
}

const std::vector<const ServiceBaggageInfo*>
getServiceBaggageData(const VendorCode& vendor,
                      int itemNo,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical)
{
  if (isHistorical)
  {
    return ServiceBaggageHistoricalDAO::instance().get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    return ServiceBaggageDAO::instance().get(deleteList, vendor, itemNo, ticketDate);
  }
}

const std::vector<const ServiceBaggageInfo*>
ServiceBaggageDAO::get(DeleteList& del,
                       const VendorCode& vendor,
                       int itemNo,
                       const DateTime& ticketDate)
{
  _codeCoverageGetCallCount++;
  std::vector<const ServiceBaggageInfo*> resultVector;
  ServiceBaggageKey key(vendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (!ptr->empty())
  {
    del.copy(ptr);
    IsCurrentG<ServiceBaggageInfo> isCurrent(ticketDate);
    for (DAOCache::value_type::const_iterator iter = ptr->begin(); iter != ptr->end(); ++iter)
    {
      if (LIKELY(isCurrent(*iter)))
        resultVector.push_back(*iter);
    }
  }
  return resultVector;
}

std::vector<const ServiceBaggageInfo*>*
ServiceBaggageDAO::create(const ServiceBaggageKey key)
{
  std::vector<const ServiceBaggageInfo*>* ret = new std::vector<const ServiceBaggageInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetServiceBaggage bo(dbAdapter->getAdapter());
    bo.findServiceBaggageInfo(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in ServiceBaggageDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
ServiceBaggageDAO::destroy(const ServiceBaggageKey key,
                           std::vector<const ServiceBaggageInfo*>* recs)
{
  destroyContainer(recs);
}

const ServiceBaggageKey
ServiceBaggageDAO::createKey(const ServiceBaggageInfo* info)
{
  return ServiceBaggageKey(info->vendor(), info->itemNo());
}

void
ServiceBaggageDAO::load()
{
  StartupLoaderNoDB<ServiceBaggageInfo, ServiceBaggageDAO>();
}

const std::string
ServiceBaggageDAO::_name("ServiceBaggage");
const std::string
ServiceBaggageDAO::_cacheClass("Taxes");
DAOHelper<ServiceBaggageDAO>
ServiceBaggageDAO::_helper(_name);
ServiceBaggageDAO* ServiceBaggageDAO::_instance = nullptr;

log4cxx::LoggerPtr
ServiceBaggageHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.ServiceBaggageHistoricalDAO"));

ServiceBaggageHistoricalDAO&
ServiceBaggageHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<const ServiceBaggageInfo*>
ServiceBaggageHistoricalDAO::get(DeleteList& del,
                                 const VendorCode& vendor,
                                 int itemNo,
                                 const DateTime& ticketDate)
{
  _codeCoverageGetCallCount++;
  std::vector<const ServiceBaggageInfo*> resultVector;
  ServiceBaggageHistoricalKey key(vendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (!ptr->empty())
  {
    del.copy(ptr);
    IsCurrentH<ServiceBaggageInfo> isCurrent(ticketDate);
    for (DAOCache::value_type::const_iterator iter = ptr->begin(); iter != ptr->end(); ++iter)
    {
      if (isCurrent(*iter))
        resultVector.push_back(*iter);
    }
  }
  return resultVector;
}

std::vector<const ServiceBaggageInfo*>*
ServiceBaggageHistoricalDAO::create(const ServiceBaggageHistoricalKey key)
{
  std::vector<const ServiceBaggageInfo*>* ret = new std::vector<const ServiceBaggageInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetServiceBaggageHistorical bo(dbAdapter->getAdapter());
    bo.findServiceBaggageInfo(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in ServiceBaggageHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
ServiceBaggageHistoricalDAO::destroy(const ServiceBaggageHistoricalKey key,
                                     std::vector<const ServiceBaggageInfo*>* recs)
{
  destroyContainer(recs);
}

bool
ServiceBaggageHistoricalDAO::translateKey(const ObjectKey& objectKey,
                                          ServiceBaggageHistoricalKey& key) const
{
  return key.initialized =
             objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
             objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
}

bool
ServiceBaggageHistoricalDAO::translateKey(const ObjectKey& objectKey,
                                          ServiceBaggageHistoricalKey& key,
                                          const DateTime ticketDate) const
{
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  return key.initialized =
             objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
}

void
ServiceBaggageHistoricalDAO::setKeyDateRange(ServiceBaggageHistoricalKey& key,
                                             const DateTime ticketDate) const
{
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
}

ServiceBaggageHistoricalDAO::ServiceBaggageHistoricalDAO(
    int cacheSize, /*default = 0*/
    const std::string& cacheType /*default = "" */)
  : HistoricalDataAccessObject<ServiceBaggageHistoricalKey,
                               std::vector<const ServiceBaggageInfo*> >(cacheSize, cacheType)
{
}

const std::string
ServiceBaggageHistoricalDAO::_name("ServiceBaggageHistorical");
const std::string
ServiceBaggageHistoricalDAO::_cacheClass("Taxes");
DAOHelper<ServiceBaggageHistoricalDAO>
ServiceBaggageHistoricalDAO::_helper(_name);
ServiceBaggageHistoricalDAO* ServiceBaggageHistoricalDAO::_instance = nullptr;

} // namespace tse
