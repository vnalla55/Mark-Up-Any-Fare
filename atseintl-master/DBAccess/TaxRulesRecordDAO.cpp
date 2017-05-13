//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/TaxRulesRecordDAO.h"

#include "Common/FallbackUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/Queries/QueryGetTaxRulesRecord.h"
#include "DBAccess/TaxRulesRecord.h"

#include <algorithm>

namespace tse
{
FIXEDFALLBACK_DECL(fallbackTaxRulesRecordDAOLogging);

Logger TaxRulesRecordDAO::_logger("atseintl.DBAccess.TaxRulesRecordDAO");

TaxRulesRecordDAO&
TaxRulesRecordDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

size_t
TaxRulesRecordDAO::invalidate(const ObjectKey& objectKey)
{
  bool translationSuccessful = false;
  if (!fallback::fixed::fallbackTaxRulesRecordDAOLogging())
  {
    TaxRulesRecordKey key;
    translationSuccessful = translateKey(objectKey, key);

    if (!translationSuccessful)
      LOG4CXX_ERROR(_logger, "Translate failed for: " << objectKey.toString());
  }

  using Base = DataAccessObject<TaxRulesRecordKey, std::vector<const TaxRulesRecord*>>;
  size_t result = Base::invalidate(objectKey);
  if (result > 0)
  {
    TaxRulesRecordKey key;
    translateKey(objectKey, key);
    unsigned int childrenCleared = keyRemoved(key);

    LOG4CXX_DEBUG(_logger, "TaxRulesRecord key: " << key._a << "|" << key._b << "removed");
    if (!fallback::fixed::fallbackTaxRulesRecordDAOLogging())
    {
      LOG4CXX_INFO(_logger, "TaxRulesRecord key: " << key._a << "|" << key._b
          << ", cleared child caches: " << childrenCleared);
    }
  }
  else if (translationSuccessful)
  {
    // Translate succeded, that means Base::invalidate failed on invalidation
    if (!fallback::fixed::fallbackTaxRulesRecordDAOLogging())
    {
      LOG4CXX_WARN(_logger, "Invalidate failed for: " << objectKey.toString()
          << " (likely empty cache after server startup)");
    }
  }

  return result;
}

const std::vector<const TaxRulesRecord*>&
getTaxRulesRecordData(const NationCode& nation,
                      const Indicator& taxPointTag,
                      const DateTime& date,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical)
{
  if (isHistorical)
  {
    return TaxRulesRecordHistoricalDAO::instance().get(deleteList, nation, taxPointTag, ticketDate);
  }

  return TaxRulesRecordDAO::instance().get(deleteList, nation, taxPointTag, date, ticketDate);
}

const std::vector<const TaxRulesRecord*>&
TaxRulesRecordDAO::get(DeleteList& del,
                       const NationCode& nation,
                       const Indicator& taxPointTag,
                       const DateTime& date,
                       const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  TaxRulesRecordKey key(nation, taxPointTag);
  DAOCache::pointer_type ptr = cache().get(key);

  del.copy(ptr);

  std::vector<const TaxRulesRecord*>* ret =
      new std::vector<const TaxRulesRecord*>(ptr->begin(), ptr->end());

  del.adopt(ret);

  return *ret;
}

TaxRulesRecordKey
TaxRulesRecordDAO::createKey(const TaxRulesRecord* rec)
{
  return TaxRulesRecordKey(rec->nation(), rec->taxPointTag());
}

bool
TaxRulesRecordDAO::translateKey(const ObjectKey& objectKey, TaxRulesRecordKey& key) const
{
  return key.initialized =
             objectKey.getValue("NATION", key._a) && objectKey.getValue("TAXPOINTTAG", key._b);
}

void
TaxRulesRecordDAO::translateKey(const TaxRulesRecordKey& key, ObjectKey& objectKey) const
{
  objectKey.setValue("NATION", key._a);
  objectKey.setValue("TAXPOINTTAG", key._b);
}

bool
TaxRulesRecordDAO::insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
{
  return DummyObjectInserter<TaxRulesRecord, TaxRulesRecordDAO>(flatKey, objectKey).success();
}

TaxRulesRecordDAO::TaxRulesRecordDAO(int cacheSize /*default = 0*/,
                                     const std::string& cacheType /*default = ""*/)
  : DataAccessObject<TaxRulesRecordKey, std::vector<const TaxRulesRecord*> >(cacheSize,
                                                                                   cacheType)
{
}

void
TaxRulesRecordDAO::load()
{
  // TODO: StartupLoaderNoDB<TaxRulesRecord,TaxRulesRecordDAO>() ;
}

std::vector<const TaxRulesRecord*>*
TaxRulesRecordDAO::create(TaxRulesRecordKey key)
{
  std::vector<const TaxRulesRecord*>* ret = new std::vector<const TaxRulesRecord*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTaxRulesRecord queryGetTaxRulesRecord(dbAdapter->getAdapter());
    queryGetTaxRulesRecord.getTaxRulesRecord(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxRulesRecordDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

sfc::CompressedData*
TaxRulesRecordDAO::compress(const std::vector<const TaxRulesRecord*>* recList) const
{
  return compressVector(recList);
}

std::vector<const TaxRulesRecord*>*
TaxRulesRecordDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<const TaxRulesRecord>(compressed);
}

void
TaxRulesRecordDAO::destroy(TaxRulesRecordKey key, std::vector<const TaxRulesRecord*>* recs)
{
  destroyContainer(recs);
}

size_t
TaxRulesRecordDAO::clear()
{
  cacheCleared();
  size_t result(cache().clear());
  LOG4CXX_INFO(_logger, "TaxRulesRecord cache cleared");
  return result;
}

const std::string
TaxRulesRecordDAO::_name("TaxRulesRecord");
const std::string
TaxRulesRecordDAO::_cacheClass("Taxes");
DAOHelper<TaxRulesRecordDAO>
TaxRulesRecordDAO::_helper(_name);
TaxRulesRecordDAO* TaxRulesRecordDAO::_instance = nullptr;

////////////////////////////////////////////////////////
// Historical DAO Object
////////////////////////////////////////////////////////
Logger TaxRulesRecordHistoricalDAO::_logger("atseintl.DBAccess.TaxRulesRecordHistoricalDAO");

TaxRulesRecordHistoricalDAO&
TaxRulesRecordHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

size_t
TaxRulesRecordHistoricalDAO::invalidate(const ObjectKey& objectKey)
{
  bool translationSuccessful = false;
  if (!fallback::fixed::fallbackTaxRulesRecordDAOLogging())
  {
    RulesRecordHistoricalKey key;
    translationSuccessful = translateKey(objectKey, key);

    if (!translationSuccessful)
      LOG4CXX_ERROR(_logger, "Translate failed for: " << objectKey.toString());
  }

  using Base = HistoricalDataAccessObject<RulesRecordHistoricalKey,
                                          std::vector<const TaxRulesRecord*>>;

  size_t result = Base::invalidate(objectKey);
  if (result > 0)
  {
    RulesRecordHistoricalKey key;
    translateKey(objectKey, key);

    unsigned int childrenCleared = keyRemoved(key);

    LOG4CXX_DEBUG(_logger, "TaxRulesRecord key: " << key._a << "|" << key._b << "removed");

    if (!fallback::fixed::fallbackTaxRulesRecordDAOLogging())
    {
      LOG4CXX_INFO(_logger, "RulesRecordHistoricalKey key: " << key._a << "|" << key._b
          << "|" << key._c << "|" << key._d << ", cleared child caches: " << childrenCleared);
    }
  }
  else if (translationSuccessful)
  {
    // Translate succeded, that means Base::invalidate failed on invalidation
    if (!fallback::fixed::fallbackTaxRulesRecordDAOLogging())
    {
      LOG4CXX_WARN(_logger, "Invalidate failed for: " << objectKey.toString()
          << " (likely empty cache after server startup)");
    }
  }

  return result;
}

const std::vector<const TaxRulesRecord*>&
TaxRulesRecordHistoricalDAO::get(DeleteList& del,
                                 const NationCode& nation,
                                 const Indicator& taxPointTag,
                                 const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  RulesRecordHistoricalKey key(nation, taxPointTag);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<const TaxRulesRecord*>* ret =
      new std::vector<const TaxRulesRecord*>(ptr->begin(), ptr->end());

  del.adopt(ret);

  return *ret;
}

std::vector<const TaxRulesRecord*>*
TaxRulesRecordHistoricalDAO::create(RulesRecordHistoricalKey key)
{
  std::vector<const TaxRulesRecord*>* ret = new std::vector<const TaxRulesRecord*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetTaxRulesRecordHistorical queryGetTaxRulesRecordHistorical(dbAdapter->getAdapter());
    queryGetTaxRulesRecordHistorical.getTaxRulesRecord(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxRulesRecordHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TaxRulesRecordHistoricalDAO::destroy(RulesRecordHistoricalKey key,
                                     std::vector<const TaxRulesRecord*>* recList)
{
  std::vector<const TaxRulesRecord*>::iterator i;
  for (i = recList->begin(); i != recList->end(); ++i)
  {
    delete *i;
  }

  delete recList;
}

sfc::CompressedData*
TaxRulesRecordHistoricalDAO::compress(const std::vector<const TaxRulesRecord*>* recList) const
{
  return compressVector(recList);
}

std::vector<const TaxRulesRecord*>*
TaxRulesRecordHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<const TaxRulesRecord>(compressed);
}

bool TaxRulesRecordHistoricalDAO::translateKey(const ObjectKey& objectKey,
                                               RulesRecordHistoricalKey& key) const
{
  if (DAOUtils::CacheBy::NODATES == _cacheBy)
  {
    // temporary fix until cache sets wrong dates in the key
    // startdate
    key._c = Global::startTime().subtractMonths(24);

    // enddate
    key._d = Global::startTime().addYears(1) + tse::Hours(23) + tse::Minutes(59) + tse::Seconds(59);
  }

  return key.initialized = objectKey.getValue("NATION", key._a)
                           && objectKey.getValue("TAXPOINTTAG", key._b);
}

bool
TaxRulesRecordHistoricalDAO::translateKey(const ObjectKey& objectKey,
                                          RulesRecordHistoricalKey& key,
                                          const DateTime ticketDate) const
{
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  return key.initialized = objectKey.getValue("NATION", key._a)
                           && objectKey.getValue("TAXPOINTTAG", key._b);
}

void
TaxRulesRecordHistoricalDAO::translateKey(const RulesRecordHistoricalKey& key, ObjectKey& objectKey)
    const
{
  objectKey.setValue("NATION", key._a);
  objectKey.setValue("TAXPOINTTAG", key._b);
  objectKey.setValue("STARTDATE", key._c);
  objectKey.setValue("ENDDATE", key._d);
}

void
TaxRulesRecordHistoricalDAO::setKeyDateRange(RulesRecordHistoricalKey& key,
                                             const DateTime ticketDate) const
{
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
}

RulesRecordHistoricalKey
TaxRulesRecordHistoricalDAO::createKey(const TaxRulesRecord* rec,
                                       const DateTime& startDate,
                                       const DateTime& endDate)
{
  return RulesRecordHistoricalKey(rec->nation(), rec->taxPointTag(), startDate, endDate);
}

bool
TaxRulesRecordHistoricalDAO::insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
{
  return DummyObjectInserterWithDateRange<TaxRulesRecord, TaxRulesRecordHistoricalDAO>(
             flatKey, objectKey).success();
}

TaxRulesRecordHistoricalDAO::TaxRulesRecordHistoricalDAO(int cacheSize /*= 0*/,
                                                         const std::string& cacheType /*= ""*/)
  : HistoricalDataAccessObject<RulesRecordHistoricalKey, std::vector<const TaxRulesRecord*> >(
        cacheSize, cacheType)
{
}

const std::string
TaxRulesRecordHistoricalDAO::_name("TaxRulesRecordHistorical");
const std::string
TaxRulesRecordHistoricalDAO::_cacheClass("Taxes");
DAOHelper<TaxRulesRecordHistoricalDAO>
TaxRulesRecordHistoricalDAO::_helper(_name);
TaxRulesRecordHistoricalDAO* TaxRulesRecordHistoricalDAO::_instance = nullptr;

////////////////////////////
////////////////////////////
// BY-CODE version
// /////////////////////////////////
// ////////////////////////////////

const std::vector<const TaxRulesRecord*>&
getTaxRulesRecordByCodeData(const TaxCode& taxCode,
                            const DateTime& date,
                            DeleteList& deleteList,
                            const DateTime& ticketDate,
                            bool isHistorical)
{
  if (isHistorical)
  {
    return TaxRulesRecordByCodeHistoricalDAO::instance().get(deleteList, taxCode, ticketDate);
  }

  return TaxRulesRecordByCodeDAO::instance().get(deleteList, taxCode, date, ticketDate);
}


Logger TaxRulesRecordByCodeDAO::_logger("atseintl.DBAccess.TaxRulesRecordByCodeDAO");

TaxRulesRecordByCodeDAO&
TaxRulesRecordByCodeDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<const TaxRulesRecord*>&
TaxRulesRecordByCodeDAO::get(DeleteList& del,
                             const TaxCode& taxCode,
                             const DateTime& date,
                             const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  TaxRulesRecordByCodeKey key(taxCode);
  DAOCache::pointer_type ptr = cache().get(key);

  return *(applyFilter(del, ptr, IsNotEffectiveG<TaxRulesRecord>(date, ticketDate)));
}

TaxRulesRecordByCodeKey
TaxRulesRecordByCodeDAO::createKey(const TaxRulesRecord* rec)
{
  return TaxRulesRecordByCodeKey(rec->taxCode());
}

bool
TaxRulesRecordByCodeDAO::translateKey(const ObjectKey& objectKey, TaxRulesRecordByCodeKey& key) const
{
  return key.initialized = objectKey.getValue("TAXCODE", key._a);
}

void
TaxRulesRecordByCodeDAO::translateKey(const TaxRulesRecordByCodeKey& key, ObjectKey& objectKey) const
{
  objectKey.setValue("TAXCODE", key._a);
}

bool
TaxRulesRecordByCodeDAO::insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
{
  return DummyObjectInserter<TaxRulesRecord, TaxRulesRecordByCodeDAO>(flatKey, objectKey).success();
}

TaxRulesRecordByCodeDAO::TaxRulesRecordByCodeDAO(int cacheSize /*default = 0*/,
                                                 const std::string& cacheType /*default = ""*/,
                                                 size_t version /*= 2*/)
  : DataAccessObject<TaxRulesRecordByCodeKey, std::vector<const TaxRulesRecord*> >(cacheSize,
                                                                                   cacheType,
                                                                                   version)
{
}

void
TaxRulesRecordByCodeDAO::load()
{
  // TODO: StartupLoaderNoDB<TaxRulesRecord,TaxRulesRecordByCodeDAO>() ;
}

std::vector<const TaxRulesRecord*>*
TaxRulesRecordByCodeDAO::create(TaxRulesRecordByCodeKey key)
{
  std::vector<const TaxRulesRecord*>* ret = new std::vector<const TaxRulesRecord*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTaxRulesRecordByCode query(dbAdapter->getAdapter());
    query.getTaxRulesRecord(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxRulesRecordByCodeDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

sfc::CompressedData*
TaxRulesRecordByCodeDAO::compress(const std::vector<const TaxRulesRecord*>* recList) const
{
  return compressVector(recList);
}

std::vector<const TaxRulesRecord*>*
TaxRulesRecordByCodeDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<const TaxRulesRecord>(compressed);
}

void
TaxRulesRecordByCodeDAO::destroy(TaxRulesRecordByCodeKey key, std::vector<const TaxRulesRecord*>* recs)
{
  keyRemoved(key);
  destroyContainer(recs);
}

size_t
TaxRulesRecordByCodeDAO::clear()
{
  cacheCleared();
  size_t result(cache().clear());
  LOG4CXX_INFO(_logger, "TaxRulesRecordByCode cache cleared");
  return result;
}

const std::string TaxRulesRecordByCodeDAO::_name("TaxRulesRecordByCode");
const std::string TaxRulesRecordByCodeDAO::_cacheClass("Taxes");
DAOHelper<TaxRulesRecordByCodeDAO> TaxRulesRecordByCodeDAO::_helper(_name);
TaxRulesRecordByCodeDAO* TaxRulesRecordByCodeDAO::_instance = nullptr;

////////////////////////////////////////////////////////
// Historical DAO Object
////////////////////////////////////////////////////////
Logger TaxRulesRecordByCodeHistoricalDAO::_logger("atseintl.DBAccess.TaxRulesRecordByCodeHistoricalDAO");

TaxRulesRecordByCodeHistoricalDAO&
TaxRulesRecordByCodeHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<const TaxRulesRecord*>&
TaxRulesRecordByCodeHistoricalDAO::get(DeleteList& del,
                                       const TaxCode& taxCode,
                                       const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  RulesRecordByCodeHistoricalKey key(taxCode);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<const TaxRulesRecord*>* ret =
      new std::vector<const TaxRulesRecord*>(ptr->begin(), ptr->end());

  del.adopt(ret);

  return *ret;
}

std::vector<const TaxRulesRecord*>*
TaxRulesRecordByCodeHistoricalDAO::create(RulesRecordByCodeHistoricalKey key)
{
  std::vector<const TaxRulesRecord*>* ret = new std::vector<const TaxRulesRecord*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetTaxRulesRecordByCodeHistorical query(dbAdapter->getAdapter());
    query.getTaxRulesRecord(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxRulesRecordByCodeHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TaxRulesRecordByCodeHistoricalDAO::destroy(RulesRecordByCodeHistoricalKey key,
                                           std::vector<const TaxRulesRecord*>* recList)
{
  std::vector<const TaxRulesRecord*>::iterator i;
  for (i = recList->begin(); i != recList->end(); ++i)
  {
    delete *i;
  }

  delete recList;
}

sfc::CompressedData*
TaxRulesRecordByCodeHistoricalDAO::compress(const std::vector<const TaxRulesRecord*>* recList) const
{
  return compressVector(recList);
}

std::vector<const TaxRulesRecord*>*
TaxRulesRecordByCodeHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<const TaxRulesRecord>(compressed);
}

bool
TaxRulesRecordByCodeHistoricalDAO::translateKey(const ObjectKey& objectKey, RulesRecordByCodeHistoricalKey& key)
    const
{
  objectKey.getValue("STARTDATE", key._b);
  objectKey.getValue("ENDDATE", key._c);
  return key.initialized = objectKey.getValue("TAXCODE", key._a);
}

bool
TaxRulesRecordByCodeHistoricalDAO::translateKey(const ObjectKey& objectKey,
                                                RulesRecordByCodeHistoricalKey& key,
                                                const DateTime ticketDate) const
{
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  return key.initialized = objectKey.getValue("TAXCODE", key._a);
}

void
TaxRulesRecordByCodeHistoricalDAO::translateKey(const RulesRecordByCodeHistoricalKey& key, ObjectKey& objectKey)
    const
{
  objectKey.setValue("TAXCODE", key._a);
}

void
TaxRulesRecordByCodeHistoricalDAO::setKeyDateRange(RulesRecordByCodeHistoricalKey& key,
                                             const DateTime ticketDate) const
{
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
}

RulesRecordByCodeHistoricalKey
TaxRulesRecordByCodeHistoricalDAO::createKey(const TaxRulesRecord* rec,
                                             const DateTime& startDate,
                                             const DateTime& endDate)
{
  return RulesRecordByCodeHistoricalKey(rec->taxCode(), startDate, endDate);
}

bool
TaxRulesRecordByCodeHistoricalDAO::insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
{
  return DummyObjectInserterWithDateRange<TaxRulesRecord, TaxRulesRecordByCodeHistoricalDAO>(
             flatKey, objectKey).success();
}

TaxRulesRecordByCodeHistoricalDAO::TaxRulesRecordByCodeHistoricalDAO(int cacheSize /*= 0*/,
                                                         const std::string& cacheType /*= ""*/,
                                                         size_t version /*= 2*/)
  : HistoricalDataAccessObject<RulesRecordByCodeHistoricalKey, std::vector<const TaxRulesRecord*> >(
        cacheSize, cacheType, version)
{
}

const std::string TaxRulesRecordByCodeHistoricalDAO::_name("TaxRulesRecordByCodeHistorical");
const std::string TaxRulesRecordByCodeHistoricalDAO::_cacheClass("Taxes");
DAOHelper<TaxRulesRecordByCodeHistoricalDAO> TaxRulesRecordByCodeHistoricalDAO::_helper(_name);
TaxRulesRecordByCodeHistoricalDAO* TaxRulesRecordByCodeHistoricalDAO::_instance = nullptr;

// DEPRECATED
// Remove with ATPCO_TAX_X1byCodeCacheRefactor fallback removal
const std::vector<const TaxRulesRecord*>&
getTaxRulesRecordByCodeAndTypeData(const TaxCode& taxCode,
                                   const TaxType& taxType,
                                   const DateTime& date,
                                   DeleteList& deleteList,
                                   const DateTime& ticketDate,
                                   bool isHistorical)
{
  if (isHistorical)
  {
    return TaxRulesRecordByCodeAndTypeHistoricalDAO::instance().get(deleteList, taxCode, taxType, ticketDate);
  }

  return TaxRulesRecordByCodeAndTypeDAO::instance().get(deleteList, taxCode, taxType, date, ticketDate);
}

Logger TaxRulesRecordByCodeAndTypeDAO::_logger("atseintl.DBAccess.TaxRulesRecordByCodeDAO");

TaxRulesRecordByCodeAndTypeDAO&
TaxRulesRecordByCodeAndTypeDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<const TaxRulesRecord*>&
TaxRulesRecordByCodeAndTypeDAO::get(DeleteList& del,
                                    const TaxCode& taxCode,
                                    const TaxType& taxType,
                                    const DateTime& date,
                                    const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  TaxRulesRecordByCodeAndTypeKey key(taxCode, taxType);
  DAOCache::pointer_type ptr = cache().get(key);

  return *(applyFilter(del, ptr, IsNotEffectiveG<TaxRulesRecord>(date, ticketDate)));
}

TaxRulesRecordByCodeAndTypeKey
TaxRulesRecordByCodeAndTypeDAO::createKey(const TaxRulesRecord* rec)
{
  return TaxRulesRecordByCodeAndTypeKey(rec->taxCode(), rec->taxType());
}

bool
TaxRulesRecordByCodeAndTypeDAO::translateKey(const ObjectKey& objectKey, TaxRulesRecordByCodeAndTypeKey& key) const
{
  return key.initialized =
             objectKey.getValue("TAXCODE", key._a) && objectKey.getValue("TAXTYPE", key._b);
}

void
TaxRulesRecordByCodeAndTypeDAO::translateKey(const TaxRulesRecordByCodeAndTypeKey& key, ObjectKey& objectKey) const
{
  objectKey.setValue("TAXCODE", key._a);
  objectKey.setValue("TAXTYPE", key._b);
}

bool
TaxRulesRecordByCodeAndTypeDAO::insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
{
  return DummyObjectInserter<TaxRulesRecord, TaxRulesRecordByCodeAndTypeDAO>(flatKey, objectKey).success();
}

TaxRulesRecordByCodeAndTypeDAO::TaxRulesRecordByCodeAndTypeDAO(int cacheSize /*default = 0*/,
                                                               const std::string& cacheType /*default = ""*/,
                                                               size_t version /* = 2 */)
  : DataAccessObject<TaxRulesRecordByCodeAndTypeKey, std::vector<const TaxRulesRecord*> >(cacheSize,
                                                                                          cacheType,
                                                                                          version)
{
}

void
TaxRulesRecordByCodeAndTypeDAO::load()
{
  // TODO: StartupLoaderNoDB<TaxRulesRecord,TaxRulesRecordByCodeDAO>() ;
}

std::vector<const TaxRulesRecord*>*
TaxRulesRecordByCodeAndTypeDAO::create(TaxRulesRecordByCodeAndTypeKey key)
{
  std::vector<const TaxRulesRecord*>* ret = new std::vector<const TaxRulesRecord*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTaxRulesRecordByCodeAndType query(dbAdapter->getAdapter());
    query.getTaxRulesRecord(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxRulesRecordByCodeAndTypeDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

sfc::CompressedData*
TaxRulesRecordByCodeAndTypeDAO::compress(const std::vector<const TaxRulesRecord*>* recList) const
{
  return compressVector(recList);
}

std::vector<const TaxRulesRecord*>*
TaxRulesRecordByCodeAndTypeDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<const TaxRulesRecord>(compressed);
}

void
TaxRulesRecordByCodeAndTypeDAO::destroy(TaxRulesRecordByCodeAndTypeKey key, std::vector<const TaxRulesRecord*>* recs)
{
  keyRemoved(key);
  destroyContainer(recs);
}

size_t
TaxRulesRecordByCodeAndTypeDAO::clear()
{
  cacheCleared();
  size_t result(cache().clear());
  LOG4CXX_INFO(_logger, "TaxRulesRecordByCode cache cleared");
  return result;
}

const std::string TaxRulesRecordByCodeAndTypeDAO::_name("TaxRulesRecordByCodeAndType");
const std::string TaxRulesRecordByCodeAndTypeDAO::_cacheClass("Taxes");
DAOHelper<TaxRulesRecordByCodeAndTypeDAO> TaxRulesRecordByCodeAndTypeDAO::_helper(_name);
TaxRulesRecordByCodeAndTypeDAO* TaxRulesRecordByCodeAndTypeDAO::_instance = nullptr;

////////////////////////////////////////////////////////
// Historical DAO Object
////////////////////////////////////////////////////////
Logger TaxRulesRecordByCodeAndTypeHistoricalDAO::_logger("atseintl.DBAccess.TaxRulesRecordByCodeHistoricalDAO");

TaxRulesRecordByCodeAndTypeHistoricalDAO&
TaxRulesRecordByCodeAndTypeHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<const TaxRulesRecord*>&
TaxRulesRecordByCodeAndTypeHistoricalDAO::get(DeleteList& del,
                                              const TaxCode& taxCode,
                                              const TaxType& taxType,
                                              const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  RulesRecordByCodeAndTypeHistoricalKey key(taxCode, taxType);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<const TaxRulesRecord*>* ret =
      new std::vector<const TaxRulesRecord*>(ptr->begin(), ptr->end());

  del.adopt(ret);

  return *ret;
}

std::vector<const TaxRulesRecord*>*
TaxRulesRecordByCodeAndTypeHistoricalDAO::create(RulesRecordByCodeAndTypeHistoricalKey key)
{
  std::vector<const TaxRulesRecord*>* ret = new std::vector<const TaxRulesRecord*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetTaxRulesRecordByCodeAndTypeHistorical query(dbAdapter->getAdapter());
    query.getTaxRulesRecord(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TaxRulesRecordByCodeAndTypeHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TaxRulesRecordByCodeAndTypeHistoricalDAO::destroy(RulesRecordByCodeAndTypeHistoricalKey key,
                                                  std::vector<const TaxRulesRecord*>* recList)
{
  std::vector<const TaxRulesRecord*>::iterator i;
  for (i = recList->begin(); i != recList->end(); ++i)
  {
    delete *i;
  }

  delete recList;
}

sfc::CompressedData*
TaxRulesRecordByCodeAndTypeHistoricalDAO::compress(const std::vector<const TaxRulesRecord*>* recList) const
{
  return compressVector(recList);
}

std::vector<const TaxRulesRecord*>*
TaxRulesRecordByCodeAndTypeHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<const TaxRulesRecord>(compressed);
}

bool
TaxRulesRecordByCodeAndTypeHistoricalDAO::translateKey(const ObjectKey& objectKey, RulesRecordByCodeAndTypeHistoricalKey& key)
    const
{
  objectKey.getValue("STARTDATE", key._c);
  objectKey.getValue("ENDDATE", key._d);
  return key.initialized =
             objectKey.getValue("TAXCODE", key._a) && objectKey.getValue("TAXTYPE", key._b);
}

bool
TaxRulesRecordByCodeAndTypeHistoricalDAO::translateKey(const ObjectKey& objectKey,
                                                       RulesRecordByCodeAndTypeHistoricalKey& key,
                                                       const DateTime ticketDate) const
{
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  return key.initialized =
             objectKey.getValue("TAXCODE", key._a) && objectKey.getValue("TAXTAXTYPE", key._b);
}

void
TaxRulesRecordByCodeAndTypeHistoricalDAO::translateKey(const RulesRecordByCodeAndTypeHistoricalKey& key, ObjectKey& objectKey)
    const
{
  objectKey.setValue("TAXCODE", key._a);
  objectKey.setValue("TAXTYPE", key._b);
}

void
TaxRulesRecordByCodeAndTypeHistoricalDAO::setKeyDateRange(RulesRecordByCodeAndTypeHistoricalKey& key,
                                                          const DateTime ticketDate) const
{
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
}

RulesRecordByCodeAndTypeHistoricalKey
TaxRulesRecordByCodeAndTypeHistoricalDAO::createKey(const TaxRulesRecord* rec,
                                                    const DateTime& startDate,
                                                    const DateTime& endDate)
{
  return RulesRecordByCodeAndTypeHistoricalKey(rec->taxCode(), rec->taxType(), startDate, endDate);
}

bool
TaxRulesRecordByCodeAndTypeHistoricalDAO::insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
{
  return DummyObjectInserterWithDateRange<TaxRulesRecord, TaxRulesRecordByCodeAndTypeHistoricalDAO>(
             flatKey, objectKey).success();
}

TaxRulesRecordByCodeAndTypeHistoricalDAO::TaxRulesRecordByCodeAndTypeHistoricalDAO(int cacheSize /*= 0*/,
                                                                                   const std::string& cacheType /*= ""*/,
                                                                                   size_t version /* = 2 */)
  : HistoricalDataAccessObject<RulesRecordByCodeAndTypeHistoricalKey, std::vector<const TaxRulesRecord*> >(
        cacheSize, cacheType, version)
{
}

const std::string TaxRulesRecordByCodeAndTypeHistoricalDAO::_name("TaxRulesRecordByCodeAndTypeHistorical");
const std::string TaxRulesRecordByCodeAndTypeHistoricalDAO::_cacheClass("Taxes");
DAOHelper<TaxRulesRecordByCodeAndTypeHistoricalDAO> TaxRulesRecordByCodeAndTypeHistoricalDAO::_helper(_name);
TaxRulesRecordByCodeAndTypeHistoricalDAO* TaxRulesRecordByCodeAndTypeHistoricalDAO::_instance = nullptr;

} // namespace tse
