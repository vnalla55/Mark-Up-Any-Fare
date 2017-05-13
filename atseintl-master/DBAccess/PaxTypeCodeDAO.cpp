//-------------------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/PaxTypeCodeDAO.h"

#include "Common/Logger.h"
#include "Common/Vendor.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/PaxTypeCodeInfo.h"
#include "DBAccess/Queries/QueryGetPaxTypeCode.h"

namespace tse
{
log4cxx::LoggerPtr
PaxTypeCodeDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.PaxTypeCodeDAO"));

PaxTypeCodeDAO&
PaxTypeCodeDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

bool
PaxTypeCodeDAO::translateKey(const ObjectKey& objectKey, PaxTypeCodeKey& key) const
{
  return key.initialized =
             objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
}

void
PaxTypeCodeDAO::translateKey(const PaxTypeCodeKey& key, ObjectKey& objectKey) const
{
  objectKey.setValue("VENDOR", key._a);
  objectKey.setValue("ITEMNO", key._b);
}

bool
PaxTypeCodeDAO::insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
{
  return DummyObjectInserter<PaxTypeCodeInfo, PaxTypeCodeDAO>(flatKey, objectKey).success();
}

PaxTypeCodeDAO::PaxTypeCodeDAO(int cacheSize /* default = 0 */,
                               const std::string& cacheType /* default = "" */)
  : DataAccessObject<PaxTypeCodeKey, std::vector<const PaxTypeCodeInfo*> >(cacheSize, cacheType)
{
}

const std::vector<const PaxTypeCodeInfo*>
getPaxTypeCodeData(const VendorCode& vendor,
                   int itemNo,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical)
{
  if (isHistorical)
  {
    return PaxTypeCodeHistoricalDAO::instance().get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    return PaxTypeCodeDAO::instance().get(deleteList, vendor, itemNo, ticketDate);
  }
}

const std::vector<const PaxTypeCodeInfo*>
PaxTypeCodeDAO::get(DeleteList& del,
                    const VendorCode& vendor,
                    int itemNo,
                    const DateTime& ticketDate)
{
  _codeCoverageGetCallCount++;
  std::vector<const PaxTypeCodeInfo*> resultVector;
  PaxTypeCodeKey key(vendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (!ptr->empty())
  {
    del.copy(ptr);
    IsCurrentG<PaxTypeCodeInfo> isCurrent(ticketDate);
    for (DAOCache::value_type::const_iterator iter = ptr->begin(); iter != ptr->end(); ++iter)
    {
      if (isCurrent(*iter))
        resultVector.push_back(*iter);
    }
  }
  return resultVector;
}

std::vector<const PaxTypeCodeInfo*>*
PaxTypeCodeDAO::create(const PaxTypeCodeKey key)
{
  std::vector<const PaxTypeCodeInfo*>* ret = new std::vector<const PaxTypeCodeInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetPaxTypeCode bo(dbAdapter->getAdapter());
    bo.findPaxTypeCodeInfo(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in PaxTypeCodeDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
PaxTypeCodeDAO::destroy(const PaxTypeCodeKey key, std::vector<const PaxTypeCodeInfo*>* recs)
{
  destroyContainer(recs);
}

const PaxTypeCodeKey
PaxTypeCodeDAO::createKey(const PaxTypeCodeInfo* info)
{
  return PaxTypeCodeKey(info->vendor(), info->itemNo());
}

void
PaxTypeCodeDAO::load()
{
  StartupLoaderNoDB<PaxTypeCodeInfo, PaxTypeCodeDAO>();
}

const std::string
PaxTypeCodeDAO::_name("PaxTypeCode");
const std::string
PaxTypeCodeDAO::_cacheClass("Taxes");
DAOHelper<PaxTypeCodeDAO>
PaxTypeCodeDAO::_helper(_name);
PaxTypeCodeDAO* PaxTypeCodeDAO::_instance = nullptr;

log4cxx::LoggerPtr
PaxTypeCodeHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.PaxTypeCodeHistoricalDAO"));

PaxTypeCodeHistoricalDAO&
PaxTypeCodeHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<const PaxTypeCodeInfo*>
PaxTypeCodeHistoricalDAO::get(DeleteList& del,
                              const VendorCode& vendor,
                              int itemNo,
                              const DateTime& ticketDate)
{
  _codeCoverageGetCallCount++;
  std::vector<const PaxTypeCodeInfo*> resultVector;
  PaxTypeCodeHistoricalKey key(vendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (!ptr->empty())
  {
    del.copy(ptr);
    IsCurrentH<PaxTypeCodeInfo> isCurrent(ticketDate);
    for (DAOCache::value_type::const_iterator iter = ptr->begin(); iter != ptr->end(); ++iter)
    {
      if (isCurrent(*iter))
        resultVector.push_back(*iter);
    }
  }
  return resultVector;
}

std::vector<const PaxTypeCodeInfo*>*
PaxTypeCodeHistoricalDAO::create(const PaxTypeCodeHistoricalKey key)
{
  std::vector<const PaxTypeCodeInfo*>* ret = new std::vector<const PaxTypeCodeInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetPaxTypeCodeHistorical bo(dbAdapter->getAdapter());
    bo.findPaxTypeCodeInfo(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in PaxTypeCodeHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
PaxTypeCodeHistoricalDAO::destroy(const PaxTypeCodeHistoricalKey key,
                                  std::vector<const PaxTypeCodeInfo*>* recs)
{
  destroyContainer(recs);
}

bool
PaxTypeCodeHistoricalDAO::translateKey(const ObjectKey& objectKey, PaxTypeCodeHistoricalKey& key)
    const
{
  return key.initialized =
             objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
             objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
}

bool
PaxTypeCodeHistoricalDAO::translateKey(const ObjectKey& objectKey,
                                       PaxTypeCodeHistoricalKey& key,
                                       const DateTime ticketDate) const
{
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  return key.initialized =
             objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
}

void
PaxTypeCodeHistoricalDAO::setKeyDateRange(PaxTypeCodeHistoricalKey& key, const DateTime ticketDate)
    const
{
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
}

PaxTypeCodeHistoricalDAO::PaxTypeCodeHistoricalDAO(int cacheSize, /*default = 0*/
                                                   const std::string& cacheType /*default = "" */)
  : HistoricalDataAccessObject<PaxTypeCodeHistoricalKey, std::vector<const PaxTypeCodeInfo*> >(
        cacheSize, cacheType)
{
}

const std::string
PaxTypeCodeHistoricalDAO::_name("PaxTypeCodeHistorical");
const std::string
PaxTypeCodeHistoricalDAO::_cacheClass("Taxes");
DAOHelper<PaxTypeCodeHistoricalDAO>
PaxTypeCodeHistoricalDAO::_helper(_name);
PaxTypeCodeHistoricalDAO* PaxTypeCodeHistoricalDAO::_instance = nullptr;

} // namespace tse
