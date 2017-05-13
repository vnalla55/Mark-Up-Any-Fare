//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/CurrencyDAO.h"

#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DBAccess/Currency.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetCurrencies.h"

namespace tse
{
log4cxx::LoggerPtr
CurrencyDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.CurrencyDAO"));

CurrencyDAO&
CurrencyDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const Currency*
getCurrencyData(const CurrencyCode& currencyCode,
                const DateTime& date,
                DeleteList& deleteList,
                const DateTime& ticketDate,
                bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    CurrencyHistoricalDAO& dao = CurrencyHistoricalDAO::instance();
    return dao.get(deleteList, currencyCode, date, ticketDate);
  }
  else
  {
    CurrencyDAO& dao = CurrencyDAO::instance();
    return dao.get(deleteList, currencyCode, date, ticketDate);
  }
}

const Currency*
CurrencyDAO::get(DeleteList& del,
                 const CurrencyCode& key,
                 const DateTime& date,
                 const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(CurrencyKey(key));
  del.copy(ptr);
  Currency* ret = nullptr;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveG<Currency>(date, ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

const std::vector<CurrencyCode>&
getAllCurrenciesData(DeleteList& deleteList, bool isHistorical)
{
  if (isHistorical)
  {
    return CurrencyHistoricalDAO::instance().getAll(deleteList);
  }
  else
  {
    return CurrencyDAO::instance().getAll(deleteList);
  }
}

const std::vector<CurrencyCode>&
CurrencyDAO::getAll(DeleteList& del)
{
  // Track calls for code coverage
  _codeCoverageGetAllCallCount++;

  std::vector<CurrencyCode>* recs = new std::vector<CurrencyCode>;
  del.adopt(recs);

  std::shared_ptr<std::vector<CurrencyKey>> keys;
  keys = cache().keys();

  if (keys != nullptr)
  {
    for (auto& elem : *keys)
    {
      recs->push_back(elem._a);
    }
  }

  return *recs;
}

CurrencyKey
CurrencyDAO::createKey(Currency* info)
{
  return CurrencyKey(info->cur());
}

std::vector<Currency*>*
CurrencyDAO::create(CurrencyKey key)
{
  std::vector<Currency*>* ret = new std::vector<Currency*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCurrencies cur(dbAdapter->getAdapter());
    cur.findCurrency(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CurrencyDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CurrencyDAO::destroy(CurrencyKey key, std::vector<Currency*>* recs)
{
  destroyContainer(recs);
}

void
CurrencyDAO::load()
{
  StartupLoader<QueryGetAllCurrencies, Currency, CurrencyDAO>();
}

sfc::CompressedData* CurrencyDAO::compress(const std::vector<Currency*>* vect) const
{
  return compressVector(vect);
}

std::vector<Currency*>* CurrencyDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<Currency>(compressed);
}

std::string
CurrencyDAO::_name("Currency");
std::string
CurrencyDAO::_cacheClass("Currency");
DAOHelper<CurrencyDAO>
CurrencyDAO::_helper(_name);
CurrencyDAO* CurrencyDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: CurrencyHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
CurrencyHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.CurrencyHistoricalDAO"));
CurrencyHistoricalDAO&
CurrencyHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const Currency*
CurrencyHistoricalDAO::get(DeleteList& del,
                           const CurrencyCode& key,
                           const DateTime& date,
                           const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  Currency* ret = nullptr;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveH<Currency>(date, ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

const std::vector<CurrencyCode>&
CurrencyHistoricalDAO::getAll(DeleteList& del)
{
  // Track calls for code coverage
  _codeCoverageGetAllCallCount++;

  std::vector<CurrencyCode>* recs = new std::vector<CurrencyCode>;
  del.adopt(recs);

  std::shared_ptr<std::vector<CurrencyCode>> keys;
  keys = cache().keys();

  std::copy(keys->begin(), keys->end(), back_inserter(*recs));

  return *recs;
}

std::vector<Currency*>*
CurrencyHistoricalDAO::create(CurrencyCode key)
{
  std::vector<Currency*>* ret = new std::vector<Currency*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCurrenciesHistorical cur(dbAdapter->getAdapter());
    cur.findCurrency(*ret, key);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CurrencyHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CurrencyHistoricalDAO::destroy(CurrencyCode key, std::vector<Currency*>* recs)
{
  destroyContainer(recs);
}

struct CurrencyHistoricalDAO::groupByKey
{
public:
  CurrencyCode prevKey;

  DAOCache& cache;

  groupByKey()
    : prevKey(INVALID_CURRENCYCODE), cache(CurrencyHistoricalDAO::instance().cache()), ptr(nullptr)
  {
  }

  std::vector<Currency*>* ptr;

  void operator()(Currency* info)
  {
    if (info->cur() != prevKey)
    {
      ptr = new std::vector<Currency*>;
      cache.put(info->cur(), ptr);
      prevKey = info->cur();
    }
    ptr->push_back(info);
  }
};

void
CurrencyHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  std::vector<Currency*> recs;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllCurrenciesHistorical cur(dbAdapter->getAdapter());
    cur.findAllCurrency(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CurrencyHistoricalDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::for_each(recs.begin(), recs.end(), groupByKey());
}

sfc::CompressedData* CurrencyHistoricalDAO::compress(const std::vector<Currency*>* vect) const
{
  return compressVector(vect);
}

std::vector<Currency*>* CurrencyHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<Currency>(compressed);
}

std::string
CurrencyHistoricalDAO::_name("CurrencyHistorical");
std::string
CurrencyHistoricalDAO::_cacheClass("Currency");
DAOHelper<CurrencyHistoricalDAO>
CurrencyHistoricalDAO::_helper(_name);
CurrencyHistoricalDAO* CurrencyHistoricalDAO::_instance = nullptr;

} // namespace tse
