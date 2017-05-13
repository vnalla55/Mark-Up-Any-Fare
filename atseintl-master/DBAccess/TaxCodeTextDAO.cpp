//-------------------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/TaxCodeTextDAO.h"

#include "Common/Logger.h"
#include "Common/Vendor.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetTaxCodeText.h"
#include "DBAccess/TaxCodeTextInfo.h"

namespace tse
{
log4cxx::LoggerPtr
TaxCodeTextDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.TaxCodeTextDAO"));

TaxCodeTextDAO&
TaxCodeTextDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

bool
TaxCodeTextDAO::translateKey(const ObjectKey& objectKey, TaxCodeTextKey& key) const
{
  return key.initialized =
             objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
}

void
TaxCodeTextDAO::translateKey(const TaxCodeTextKey& key, ObjectKey& objectKey) const
{
  objectKey.setValue("VENDOR", key._a);
  objectKey.setValue("ITEMNO", key._b);
}

bool
TaxCodeTextDAO::insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
{
  return DummyObjectInserter<TaxCodeTextInfo, TaxCodeTextDAO>(flatKey, objectKey).success();
}

TaxCodeTextDAO::TaxCodeTextDAO(int cacheSize /* default = 0 */,
                               const std::string& cacheType /* default = "" */)
  : DataAccessObject<TaxCodeTextKey, std::vector<const TaxCodeTextInfo*> >(cacheSize, cacheType)
{
}

const std::vector<const TaxCodeTextInfo*>
getTaxCodeTextData(const VendorCode& vendor,
                   int itemNo,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical)
{
  if (isHistorical)
  {
    return TaxCodeTextHistoricalDAO::instance().get(deleteList, vendor, itemNo, ticketDate);
  }
  else
  {
    return TaxCodeTextDAO::instance().get(deleteList, vendor, itemNo, ticketDate);
  }
}

const std::vector<const TaxCodeTextInfo*>
TaxCodeTextDAO::get(DeleteList& del,
                    const VendorCode& vendor,
                    int itemNo,
                    const DateTime& ticketDate)
{
  _codeCoverageGetCallCount++;
  std::vector<const TaxCodeTextInfo*> resultVector;
  TaxCodeTextKey key(vendor, itemNo);
  DAOCache::pointer_type ptr = cache().get(key);
  if (!ptr->empty())
  {
    del.copy(ptr);
    IsCurrentG<TaxCodeTextInfo> isCurrent(ticketDate);
    for (DAOCache::value_type::const_iterator iter = ptr->begin(); iter != ptr->end(); ++iter)
    {
      if (isCurrent(*iter))
        resultVector.push_back(*iter);
    }
  }
  return resultVector;
}

std::vector<const TaxCodeTextInfo*>*
TaxCodeTextDAO::create(const TaxCodeTextKey key)
{
  std::vector<const TaxCodeTextInfo*>* ret = new std::vector<const TaxCodeTextInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTaxCodeText bo(dbAdapter->getAdapter());
    bo.findTaxCodeTextInfo(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxCodeTextDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TaxCodeTextDAO::destroy(const TaxCodeTextKey key, std::vector<const TaxCodeTextInfo*>* recs)
{
  destroyContainer(recs);
}

const TaxCodeTextKey
TaxCodeTextDAO::createKey(const TaxCodeTextInfo* info)
{
  return TaxCodeTextKey(info->vendor(), info->itemNo());
}

void
TaxCodeTextDAO::load()
{
  StartupLoaderNoDB<TaxCodeTextInfo, TaxCodeTextDAO>();
}

const std::string
TaxCodeTextDAO::_name("TaxCodeText");
const std::string
TaxCodeTextDAO::_cacheClass("Taxes");
DAOHelper<TaxCodeTextDAO>
TaxCodeTextDAO::_helper(_name);
TaxCodeTextDAO* TaxCodeTextDAO::_instance = nullptr;

log4cxx::LoggerPtr
TaxCodeTextHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.TaxCodeTextHistoricalDAO"));

TaxCodeTextHistoricalDAO&
TaxCodeTextHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<const TaxCodeTextInfo*>
TaxCodeTextHistoricalDAO::get(DeleteList& del,
                              const VendorCode& vendor,
                              int itemNo,
                              const DateTime& ticketDate)
{
  _codeCoverageGetCallCount++;
  std::vector<const TaxCodeTextInfo*> resultVector;
  TaxCodeTextHistoricalKey key(vendor, itemNo);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (!ptr->empty())
  {
    del.copy(ptr);
    IsCurrentH<TaxCodeTextInfo> isCurrent(ticketDate);
    for (DAOCache::value_type::const_iterator iter = ptr->begin(); iter != ptr->end(); ++iter)
    {
      if (isCurrent(*iter))
        resultVector.push_back(*iter);
    }
  }
  return resultVector;
}

std::vector<const TaxCodeTextInfo*>*
TaxCodeTextHistoricalDAO::create(const TaxCodeTextHistoricalKey key)
{
  std::vector<const TaxCodeTextInfo*>* ret = new std::vector<const TaxCodeTextInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTaxCodeTextHistorical bo(dbAdapter->getAdapter());
    bo.findTaxCodeTextInfo(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxCodeTextHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TaxCodeTextHistoricalDAO::destroy(const TaxCodeTextHistoricalKey key,
                                  std::vector<const TaxCodeTextInfo*>* recs)
{
  destroyContainer(recs);
}

bool
TaxCodeTextHistoricalDAO::translateKey(const ObjectKey& objectKey, TaxCodeTextHistoricalKey& key)
    const
{
  return key.initialized =
             objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b) &&
             objectKey.getValue("STARTDATE", key._c) && objectKey.getValue("ENDDATE", key._d);
}

bool
TaxCodeTextHistoricalDAO::translateKey(const ObjectKey& objectKey,
                                       TaxCodeTextHistoricalKey& key,
                                       const DateTime ticketDate) const
{
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  return key.initialized =
             objectKey.getValue("VENDOR", key._a) && objectKey.getValue("ITEMNO", key._b);
}

void
TaxCodeTextHistoricalDAO::setKeyDateRange(TaxCodeTextHistoricalKey& key, const DateTime ticketDate)
    const
{
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
}

TaxCodeTextHistoricalDAO::TaxCodeTextHistoricalDAO(int cacheSize, /*default = 0*/
                                                   const std::string& cacheType /*default = "" */)
  : HistoricalDataAccessObject<TaxCodeTextHistoricalKey, std::vector<const TaxCodeTextInfo*> >(
        cacheSize, cacheType)
{
}

const std::string
TaxCodeTextHistoricalDAO::_name("TaxCodeTextHistorical");
const std::string
TaxCodeTextHistoricalDAO::_cacheClass("Taxes");
DAOHelper<TaxCodeTextHistoricalDAO>
TaxCodeTextHistoricalDAO::_helper(_name);
TaxCodeTextHistoricalDAO* TaxCodeTextHistoricalDAO::_instance = nullptr;

} // namespace tse
