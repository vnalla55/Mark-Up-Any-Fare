//-------------------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/TaxReportingRecordDAO.h"

#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/Vendor.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetTaxReportingRecord.h"
#include "DBAccess/TaxReportingRecordInfo.h"

#include <algorithm>

namespace tse
{
FIXEDFALLBACK_DECL(fallbackErrorsInTaxDAOFix);

Logger TaxReportingRecordDAO::_logger("atseintl.DBAccess.TaxReportingRecordDAO");

TaxReportingRecordDAO&
TaxReportingRecordDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

size_t
TaxReportingRecordDAO::invalidate(const ObjectKey& objectKey)
{
  using Base = DataAccessObject<TaxReportingRecordKey,
                                std::vector<const TaxReportingRecordInfo*>>;

  size_t result = Base::invalidate(objectKey);
  if (result > 0)
  {
    TaxReportingRecordKey key;
    translateKey(objectKey, key);
    keyRemoved(key);
  }

  return result;
}

bool
TaxReportingRecordDAO::translateKey(const ObjectKey& objectKey, TaxReportingRecordKey& key) const
{
  return key.initialized =
             objectKey.getValue("VENDOR", key._a) && objectKey.getValue("NATION", key._b) &&
             objectKey.getValue("TAXCARRIER", key._c) && objectKey.getValue("TAXCODE", key._d) &&
             objectKey.getValue("TAXTYPE", key._e);
}

void
TaxReportingRecordDAO::translateKey(const TaxReportingRecordKey& key, ObjectKey& objectKey) const
{
  objectKey.setValue("VENDOR", key._a);
  objectKey.setValue("NATION", key._b);
  objectKey.setValue("TAXCARRIER", key._c);
  objectKey.setValue("TAXCODE", key._d);
  objectKey.setValue("TAXTYPE", key._e);
}

bool
TaxReportingRecordDAO::insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
{
  return DummyObjectInserter<TaxReportingRecordInfo, TaxReportingRecordDAO>(flatKey, objectKey)
      .success();
}

TaxReportingRecordDAO::TaxReportingRecordDAO(int cacheSize /* default = 0 */,
                                             const std::string& cacheType /* default = "" */,
                                             size_t version /* default = 2 */)
  : DataAccessObject<TaxReportingRecordKey, std::vector<const TaxReportingRecordInfo*> >(cacheSize,
                                                                                         cacheType,
                                                                                         version)
{
}

const std::vector<const TaxReportingRecordInfo*>
getTaxReportingRecordData(const VendorCode& vendor,
                          const NationCode& nation,
                          const CarrierCode& taxCarrier,
                          const TaxCode& taxCode,
                          const TaxType& taxType,
                          const DateTime& date,
                          DeleteList& deleteList,
                          const DateTime& ticketDate,
                          bool isHistorical)
{
  if (isHistorical)
  {
    return TaxReportingRecordHistoricalDAO::instance().get(
        deleteList, vendor, nation, taxCarrier, taxCode, taxType, date, ticketDate);
  }
  else
  {
    return TaxReportingRecordDAO::instance().get(
        deleteList, vendor, nation, taxCarrier, taxCode, taxType, date, ticketDate);
  }
}

namespace {

struct LessByTypeNationDate // for equivalent taxes with possibly different dates
{
  bool operator()(const TaxReportingRecordInfo* lhs, const TaxReportingRecordInfo* rhs) const
  {
    return std::tie(lhs->taxType(), lhs->nationCode(), lhs->effDate()) <
           std::tie(rhs->taxType(), rhs->nationCode(), rhs->effDate());
  }
};

struct EqualByTypeNation
{
  bool operator()(const TaxReportingRecordInfo* lhs, const TaxReportingRecordInfo* rhs) const
  {
    return std::tie(lhs->taxType(), lhs->nationCode()) ==
           std::tie(rhs->taxType(), rhs->nationCode());
  }
};

struct LessByDate
{
  bool operator()(const TaxReportingRecordInfo* lhs, const TaxReportingRecordInfo* rhs) const
  {
    return lhs->effDate() < rhs->effDate();
  }
};

void removeDuplicates(std::vector<const TaxReportingRecordInfo*>& records)
{
  std::sort(records.begin(), records.end(), LessByTypeNationDate());
  records.erase(std::unique(records.begin(), records.end(), EqualByTypeNation()), records.end());
  std::sort(records.begin(), records.end(), LessByDate());
}

} // anonymous namespace

const std::vector<const TaxReportingRecordInfo*>
getAllTaxReportingRecordData(const TaxCode& taxCode,
                             DeleteList& deleteList)
{
  std::vector<const TaxReportingRecordInfo*> current = TaxReportingRecordByCodeDAO::instance().get(
    deleteList, taxCode);
  std::vector<const TaxReportingRecordInfo*> historical
    = TaxReportingRecordByCodeHistoricalDAO::instance().get(deleteList, taxCode);
  historical.insert(historical.end(), current.begin(), current.end());
  removeDuplicates(historical);
  return historical;
}

const std::vector<const TaxReportingRecordInfo*>
TaxReportingRecordDAO::get(DeleteList& del,
                           const VendorCode& vendor,
                           const NationCode& nation,
                           const CarrierCode& taxCarrier,
                           const TaxCode& taxCode,
                           const TaxType& taxType,
                           const DateTime& date,
                           const DateTime& ticketDate)
{
  _codeCoverageGetCallCount++;
  std::vector<const TaxReportingRecordInfo*> resultVector;
  TaxReportingRecordKey key(vendor, nation, taxCarrier, taxCode, taxType);
  DAOCache::pointer_type ptr = cache().get(key);
  if (!ptr->empty())
  {
    del.copy(ptr);
    // IsNotEffectiveG<TaxRulesRecord> checkDate(date, ticketDate);
    IsCurrentG<TaxReportingRecordInfo> checkDate(ticketDate);
    for (DAOCache::value_type::const_iterator iter = ptr->begin(); iter != ptr->end(); ++iter)
    {
      if (checkDate(*iter))
        resultVector.push_back(*iter);
    }
  }
  return resultVector;
}

std::vector<const TaxReportingRecordInfo*>*
TaxReportingRecordDAO::create(const TaxReportingRecordKey key)
{
  std::vector<const TaxReportingRecordInfo*>* ret = new std::vector<const TaxReportingRecordInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTaxReportingRecord bo(dbAdapter->getAdapter());
    bo.findTaxReportingRecordInfo(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxReportingRecordDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TaxReportingRecordDAO::destroy(const TaxReportingRecordKey key,
                               std::vector<const TaxReportingRecordInfo*>* recs)
{
  destroyContainer(recs);
}

size_t
TaxReportingRecordDAO::clear()
{
  cacheCleared();
  size_t result(cache().clear());
  LOG4CXX_INFO(_logger, "TaxReportingRecord cache cleared");
  return result;
}

const TaxReportingRecordKey
TaxReportingRecordDAO::createKey(const TaxReportingRecordInfo* info)
{
  return TaxReportingRecordKey(
      info->vendor(), info->nationCode(), info->taxCarrier(), info->taxCode(), info->taxType());
}

void
TaxReportingRecordDAO::load()
{
  StartupLoaderNoDB<TaxReportingRecordInfo, TaxReportingRecordDAO>();
}

const std::string
TaxReportingRecordDAO::_name("TaxReportingRecord");
const std::string
TaxReportingRecordDAO::_cacheClass("Taxes");
DAOHelper<TaxReportingRecordDAO>
TaxReportingRecordDAO::_helper(_name);
TaxReportingRecordDAO* TaxReportingRecordDAO::_instance = nullptr;

Logger TaxReportingRecordHistoricalDAO::_logger("atseintl.DBAccess.TaxReportingRecordHistoricalDAO");

TaxReportingRecordHistoricalDAO&
TaxReportingRecordHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

size_t
TaxReportingRecordHistoricalDAO::invalidate(const ObjectKey& objectKey)
{
  using Base = HistoricalDataAccessObject<TaxReportingRecordHistoricalKey,
                                          std::vector<const TaxReportingRecordInfo*>>;

  size_t result = Base::invalidate(objectKey);
  if (result > 0)
  {
    TaxReportingRecordHistoricalKey key;
    translateKey(objectKey, key);
    keyRemoved(key);
  }

  return result;
}

const std::vector<const TaxReportingRecordInfo*>
TaxReportingRecordHistoricalDAO::get(DeleteList& del,
                                     const VendorCode& vendor,
                                     const NationCode& nation,
                                     const CarrierCode& taxCarrier,
                                     const TaxCode& taxCode,
                                     const TaxType& taxType,
                                     const DateTime& date,
                                     const DateTime& ticketDate)
{
  _codeCoverageGetCallCount++;
  std::vector<const TaxReportingRecordInfo*> resultVector;
  TaxReportingRecordHistoricalKey key(vendor, nation, taxCarrier, taxCode, taxType);
  DAOUtils::getDateRange(ticketDate, key._f, key._g, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  if (!ptr->empty())
  {
    del.copy(ptr);
    // IsNotEffectiveG<TaxRulesRecord> checkDate(date, ticketDate);
    IsCurrentH<TaxReportingRecordInfo> checkDate(ticketDate);
    for (DAOCache::value_type::const_iterator iter = ptr->begin(); iter != ptr->end(); ++iter)
    {
      if (checkDate(*iter))
        resultVector.push_back(*iter);
    }
  }
  return resultVector;
}

std::vector<const TaxReportingRecordInfo*>*
TaxReportingRecordHistoricalDAO::create(const TaxReportingRecordHistoricalKey key)
{
  std::vector<const TaxReportingRecordInfo*>* ret = new std::vector<const TaxReportingRecordInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTaxReportingRecordHistorical bo(dbAdapter->getAdapter());
    bo.findTaxReportingRecordInfo(*ret, key._a, key._b, key._c, key._d, key._e, key._f, key._g);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxReportingRecordHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TaxReportingRecordHistoricalDAO::destroy(const TaxReportingRecordHistoricalKey key,
                                         std::vector<const TaxReportingRecordInfo*>* recs)
{
  destroyContainer(recs);
}

size_t
TaxReportingRecordHistoricalDAO::clear()
{
  cacheCleared();
  size_t result(cache().clear());
  LOG4CXX_INFO(_logger, "TaxReportingRecordHistorical cache cleared");
  return result;
}

bool
TaxReportingRecordHistoricalDAO::translateKey(const ObjectKey& objectKey,
                                              TaxReportingRecordHistoricalKey& key) const
{
  if (fallback::fixed::fallbackErrorsInTaxDAOFix())
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("NATION", key._b) &&
               objectKey.getValue("TAXCARRIER", key._c) && objectKey.getValue("TAXCODE", key._d) &&
               objectKey.getValue("TAXTYPE", key._e) && objectKey.getValue("STARTDATE", key._c) &&
               objectKey.getValue("ENDDATE", key._d);
  else
    return key.initialized =
               objectKey.getValue("VENDOR", key._a) && objectKey.getValue("NATION", key._b) &&
               objectKey.getValue("TAXCARRIER", key._c) && objectKey.getValue("TAXCODE", key._d) &&
               objectKey.getValue("TAXTYPE", key._e) && objectKey.getValue("STARTDATE", key._f) &&
               objectKey.getValue("ENDDATE", key._g);
}

bool
TaxReportingRecordHistoricalDAO::translateKey(const ObjectKey& objectKey,
                                              TaxReportingRecordHistoricalKey& key,
                                              const DateTime ticketDate) const
{
  DAOUtils::getDateRange(ticketDate, key._f, key._g, _cacheBy);
  return key.initialized =
             objectKey.getValue("VENDOR", key._a) && objectKey.getValue("NATION", key._b) &&
             objectKey.getValue("TAXCARRIER", key._c) && objectKey.getValue("TAXCODE", key._d) &&
             objectKey.getValue("TAXTYPE", key._e);
}

void
TaxReportingRecordHistoricalDAO::setKeyDateRange(TaxReportingRecordHistoricalKey& key,
                                                 const DateTime ticketDate) const
{
  DAOUtils::getDateRange(ticketDate, key._f, key._g, _cacheBy);
}

TaxReportingRecordHistoricalDAO::TaxReportingRecordHistoricalDAO(
    int cacheSize, /*default = 0*/
    const std::string& cacheType /*default = "" */,
    size_t version /* default = 2 */)
  : HistoricalDataAccessObject<TaxReportingRecordHistoricalKey,
                               std::vector<const TaxReportingRecordInfo*> >(cacheSize, cacheType, version)
{
}

const std::string
TaxReportingRecordHistoricalDAO::_name("TaxReportingRecordHistorical");
const std::string
TaxReportingRecordHistoricalDAO::_cacheClass("Taxes");
DAOHelper<TaxReportingRecordHistoricalDAO>
TaxReportingRecordHistoricalDAO::_helper(_name);
TaxReportingRecordHistoricalDAO* TaxReportingRecordHistoricalDAO::_instance = nullptr;


// BY-CODE version

Logger TaxReportingRecordByCodeDAO::_logger("atseintl.DBAccess.TaxReportingRecordByCodeDAO");

TaxReportingRecordByCodeDAO& TaxReportingRecordByCodeDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

bool
TaxReportingRecordByCodeDAO::translateKey(const ObjectKey& objectKey,
                                            TaxReportingRecordByCodeKey& key) const
{
  return key.initialized = objectKey.getValue("TAXCODE", key._a);
}

void
TaxReportingRecordByCodeDAO::translateKey(const TaxReportingRecordByCodeKey& key,
                                            ObjectKey& objectKey) const
{
  objectKey.setValue("TAXCODE", key._a);
}

bool
TaxReportingRecordByCodeDAO::insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
{
  return DummyObjectInserter<TaxReportingRecordInfo,
                             TaxReportingRecordByCodeDAO>(flatKey, objectKey) .success();
}

TaxReportingRecordByCodeDAO::TaxReportingRecordByCodeDAO(int cacheSize /* default = 0 */,
                                             const std::string& cacheType /* default = "" */,
                                             size_t version /* default = 2 */)
  : DataAccessObject<TaxReportingRecordByCodeKey,
                     std::vector<const TaxReportingRecordInfo*> >(cacheSize, cacheType, version)
{
}

std::vector<const TaxReportingRecordInfo*>
TaxReportingRecordByCodeDAO::get(DeleteList& del, const TaxCode& taxCode)
{
  _codeCoverageGetCallCount++;
  std::vector<const TaxReportingRecordInfo*> resultVector;
  TaxReportingRecordByCodeKey key(taxCode);
  DAOCache::pointer_type ptr = cache().get(key);
  if (!ptr->empty())
  {
    del.copy(ptr);
    for (DAOCache::value_type::const_iterator iter = ptr->begin(); iter != ptr->end(); ++iter)
    {
        resultVector.push_back(*iter);
    }
  }
  return resultVector;
}

std::vector<const TaxReportingRecordInfo*>*
TaxReportingRecordByCodeDAO::create(const TaxReportingRecordByCodeKey key)
{
  std::vector<const TaxReportingRecordInfo*>* ret = new std::vector<const TaxReportingRecordInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTaxReportingRecordByCode bo(dbAdapter->getAdapter());
    bo.findTaxReportingRecordInfo(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxReportingRecordByCodeDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TaxReportingRecordByCodeDAO::destroy(const TaxReportingRecordByCodeKey key,
                               std::vector<const TaxReportingRecordInfo*>* recs)
{
  keyRemoved(key);
  destroyContainer(recs);
}

size_t
TaxReportingRecordByCodeDAO::clear()
{
  cacheCleared();
  size_t result(cache().clear());
  LOG4CXX_INFO(_logger, "TaxReportingRecordByCode cache cleared");
  return result;
}

const TaxReportingRecordByCodeKey
TaxReportingRecordByCodeDAO::createKey(const TaxReportingRecordInfo* info)
{
  return TaxReportingRecordByCodeKey(info->taxCode());
}

void
TaxReportingRecordByCodeDAO::load()
{
  StartupLoaderNoDB<TaxReportingRecordInfo, TaxReportingRecordByCodeDAO>();
}

const std::string
TaxReportingRecordByCodeDAO::_name("TaxReportingRecordByCode");

const std::string
TaxReportingRecordByCodeDAO::_cacheClass("Taxes");
DAOHelper<TaxReportingRecordByCodeDAO>
TaxReportingRecordByCodeDAO::_helper(_name);
TaxReportingRecordByCodeDAO* TaxReportingRecordByCodeDAO::_instance = nullptr;

// -------------------------------

Logger TaxReportingRecordByCodeHistoricalDAO::
  _logger("atseintl.DBAccess.TaxReportingRecordByCodeHistoricalDAO");

TaxReportingRecordByCodeHistoricalDAO&
TaxReportingRecordByCodeHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

std::vector<const TaxReportingRecordInfo*>
TaxReportingRecordByCodeHistoricalDAO::get(DeleteList& del,
                                     const TaxCode& taxCode)
{
  _codeCoverageGetCallCount++;
  std::vector<const TaxReportingRecordInfo*> resultVector;
  TaxReportingRecordByCodeHistoricalKey key(taxCode);
  DAOCache::pointer_type ptr = cache().get(key);
  if (!ptr->empty())
  {
    del.copy(ptr);
    for (DAOCache::value_type::const_iterator iter = ptr->begin(); iter != ptr->end(); ++iter)
    {
      resultVector.push_back(*iter);
    }
  }
  return resultVector;
}

std::vector<const TaxReportingRecordInfo*>*
TaxReportingRecordByCodeHistoricalDAO::create(const TaxReportingRecordByCodeHistoricalKey key)
{
  std::vector<const TaxReportingRecordInfo*>* ret = new std::vector<const TaxReportingRecordInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTaxReportingRecordByCodeHistorical bo(dbAdapter->getAdapter());
    bo.findTaxReportingRecordInfo(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxReportingRecordByCodeHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TaxReportingRecordByCodeHistoricalDAO::destroy(const TaxReportingRecordByCodeHistoricalKey key,
                                               std::vector<const TaxReportingRecordInfo*>* recs)
{
  keyRemoved(key);
  destroyContainer(recs);
}

size_t
TaxReportingRecordByCodeHistoricalDAO::clear()
{
  cacheCleared();
  size_t result(cache().clear());
  LOG4CXX_INFO(_logger, "TaxReportingRecordByCodeHistorical cache cleared");
  return result;
}

bool
TaxReportingRecordByCodeHistoricalDAO::translateKey(const ObjectKey& objectKey,
                                              TaxReportingRecordByCodeHistoricalKey& key) const
{
  return key.initialized = objectKey.getValue("TAXCODE", key._a);
}

TaxReportingRecordByCodeHistoricalDAO::TaxReportingRecordByCodeHistoricalDAO(
    int cacheSize, /*default = 0*/
    const std::string& cacheType /*default = "" */,
    size_t version /* default = 2 */)
  : HistoricalDataAccessObject<TaxReportingRecordByCodeHistoricalKey,
                               std::vector<const TaxReportingRecordInfo*> >(cacheSize, cacheType, version)
{
}

const std::string
TaxReportingRecordByCodeHistoricalDAO::_name("TaxReportingRecordByCodeHistorical");
const std::string
TaxReportingRecordByCodeHistoricalDAO::_cacheClass("Taxes");
DAOHelper<TaxReportingRecordByCodeHistoricalDAO>
TaxReportingRecordByCodeHistoricalDAO::_helper(_name);
TaxReportingRecordByCodeHistoricalDAO* TaxReportingRecordByCodeHistoricalDAO::_instance = nullptr;


} // namespace tse
