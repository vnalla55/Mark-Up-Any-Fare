//-------------------------------------------------------------------------------
// Copyright 2004, 2005, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/TariffInhibitsDAO.h"

#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetTrfInhib.h"
#include "DBAccess/TariffInhibits.h"

namespace tse
{
log4cxx::LoggerPtr
TariffInhibitsDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.TariffInhibitsDAO"));
TariffInhibitsDAO&
TariffInhibitsDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const Indicator
getTariffInhibitData(const VendorCode& vendor,
                     const Indicator tariffCrossRefType,
                     const CarrierCode& carrier,
                     const TariffNumber& fareTariff,
                     const TariffCode& ruleTariffCode,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    TariffInhibitsHistoricalDAO& dao = TariffInhibitsHistoricalDAO::instance();
    return dao.getTariffInhibit(
        vendor, tariffCrossRefType, carrier, fareTariff, ruleTariffCode, deleteList, ticketDate);
  }
  else
  {
    TariffInhibitsDAO& dao = TariffInhibitsDAO::instance();
    return dao.getTariffInhibit(
        vendor, tariffCrossRefType, carrier, fareTariff, ruleTariffCode, deleteList);
  }
}

const Indicator
TariffInhibitsDAO::getTariffInhibit(const VendorCode& vendor,
                                    const Indicator tariffCrossRefType,
                                    const CarrierCode& carrier,
                                    const TariffNumber& fareTariff,
                                    const TariffCode& ruleTariffCode,
                                    DeleteList& del)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  TariffInhibitsKey key(vendor, tariffCrossRefType, carrier, fareTariff, ruleTariffCode);
  DAOCache::pointer_type pVect = cache().get(key);
  del.copy(pVect);
  if (pVect->size() == 0)
    return 'N';

  return (*pVect->begin())->inhibit();
}

TariffInhibitsKey
TariffInhibitsDAO::createKey(TariffInhibits* info)
{
  return TariffInhibitsKey(info->vendor(),
                           info->tariffCrossRefType(),
                           info->carrier(),
                           info->fareTariff(),
                           info->ruleTariffCode());
}

std::vector<TariffInhibits*>*
TariffInhibitsDAO::create(TariffInhibitsKey key)
{
  std::vector<TariffInhibits*>* ret = new std::vector<TariffInhibits*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTrfInhib ti(dbAdapter->getAdapter());
    ti.findTariffInhibits(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TariffInhibitsDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TariffInhibitsDAO::destroy(TariffInhibitsKey key, std::vector<TariffInhibits*>* recs)
{
  destroyContainer(recs);
}

void
TariffInhibitsDAO::load()
{
  StartupLoader<QueryGetAllTrfInhib, TariffInhibits, TariffInhibitsDAO>();
}

sfc::CompressedData*
TariffInhibitsDAO::compress(const std::vector<TariffInhibits*>* vect) const
{
  return compressVector(vect);
}

std::vector<TariffInhibits*>*
TariffInhibitsDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<TariffInhibits>(compressed);
}

std::string
TariffInhibitsDAO::_name("TariffInhibits");
std::string
TariffInhibitsDAO::_cacheClass("Common");
DAOHelper<TariffInhibitsDAO>
TariffInhibitsDAO::_helper(_name);
TariffInhibitsDAO* TariffInhibitsDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: TariffInhibitsHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
TariffInhibitsHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.TariffInhibitsHistoricalDAO"));
TariffInhibitsHistoricalDAO&
TariffInhibitsHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const Indicator
TariffInhibitsHistoricalDAO::getTariffInhibit(const VendorCode& vendor,
                                              const Indicator tariffCrossRefType,
                                              const CarrierCode& carrier,
                                              const TariffNumber& fareTariff,
                                              const TariffCode& ruleTariffCode,
                                              DeleteList& del,
                                              const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  TariffInhibitsKey key(vendor, tariffCrossRefType, carrier, fareTariff, ruleTariffCode);
  DAOCache::pointer_type pVect = cache().get(key);
  del.copy(pVect);

  if (pVect->size() == 0)
    return 'N';

  // some of the objects may not be current, so we have to create a new vector for the current ones
  std::vector<TariffInhibits*>* ret =
      new std::vector<TariffInhibits*>(pVect->begin(), pVect->end());
  del.adopt(ret);
  remove_copy_if(pVect->begin(),
                 pVect->end(),
                 back_inserter(*ret),
                 IsNotCurrentCreateDateOnly<TariffInhibits>(ticketDate));
  if (pVect->size() == 0)
    return 'N';

  return (*ret->begin())->inhibit();
}

struct TariffInhibitsHistoricalDAO::groupByKey
{
public:
  TariffInhibitsKey prevKey;

  DAOCache& cache;

  groupByKey() : cache(TariffInhibitsHistoricalDAO::instance().cache()), ptr(nullptr) {}

  std::vector<TariffInhibits*>* ptr;

  void operator()(TariffInhibits* info)
  {
    TariffInhibitsKey key(info->vendor(),
                          info->tariffCrossRefType(),
                          info->carrier(),
                          info->fareTariff(),
                          info->ruleTariffCode());
    if (!(key == prevKey))
    {
      ptr = new std::vector<TariffInhibits*>;
      cache.put(key, ptr);
      prevKey = key;
    }

    ptr->push_back(info);
  }
};

std::vector<TariffInhibits*>*
TariffInhibitsHistoricalDAO::create(TariffInhibitsKey key)
{
  std::vector<TariffInhibits*>* ret = new std::vector<TariffInhibits*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetTrfInhib ti(dbAdapter->getAdapter());
    ti.findTariffInhibits(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TariffInhibitsHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
TariffInhibitsHistoricalDAO::destroy(TariffInhibitsKey key, std::vector<TariffInhibits*>* recs)
{
  destroyContainer(recs);
}

void
TariffInhibitsHistoricalDAO::load()
{
  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<TariffInhibits*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllTrfInhib ti(dbAdapter->getAdapter());
    ti.findAllTariffInhibits(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TariffInhibitsHistoricalDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::for_each(recs.begin(), recs.end(), groupByKey());
}

sfc::CompressedData*
TariffInhibitsHistoricalDAO::compress(const std::vector<TariffInhibits*>* vect) const
{
  return compressVector(vect);
}

std::vector<TariffInhibits*>*
TariffInhibitsHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<TariffInhibits>(compressed);
}

std::string
TariffInhibitsHistoricalDAO::_name("TariffInhibitsHistorical");
std::string
TariffInhibitsHistoricalDAO::_cacheClass("Common");
DAOHelper<TariffInhibitsHistoricalDAO>
TariffInhibitsHistoricalDAO::_helper(_name);
TariffInhibitsHistoricalDAO* TariffInhibitsHistoricalDAO::_instance = nullptr;

} // namespace tse
