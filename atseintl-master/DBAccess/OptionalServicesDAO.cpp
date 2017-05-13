//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/OptionalServicesDAO.h"

#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/Queries/QueryGetOptionalServices.h"

namespace
{

template <typename T>
struct IsSame
{
  IsSame<T>(T& data) : _data(data) {}
  template <typename S>
  bool operator()(S* obj, T& (S::*mhod)() const) const
  {
    return (obj->*mhod)() == _data;
  }

private:
  T& _data;
};
}

namespace tse
{
log4cxx::LoggerPtr
OptionalServicesDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.OptionalServicesDAO"));

std::string
OptionalServicesDAO::_name("OptionalServices");
std::string
OptionalServicesDAO::_cacheClass("Rules");
DAOHelper<OptionalServicesDAO>
OptionalServicesDAO::_helper(_name);
OptionalServicesDAO* OptionalServicesDAO::_instance = nullptr;

OptionalServicesDAO&
OptionalServicesDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<OptionalServicesInfo*>&
getOptionalServicesData(const VendorCode& vendor,
                        const CarrierCode& carrier,
                        const ServiceTypeCode& serviceTypeCode,
                        const ServiceSubTypeCode& serviceSubTypeCode,
                        Indicator fltTktMerchInd,
                        const DateTime& date,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    OptionalServicesHistoricalDAO& dao = OptionalServicesHistoricalDAO::instance();
    return dao.get(deleteList,
                   vendor,
                   carrier,
                   serviceTypeCode,
                   serviceSubTypeCode,
                   fltTktMerchInd,
                   date,
                   ticketDate);
  }
  else
  {
    OptionalServicesDAO& dao = OptionalServicesDAO::instance();
    return dao.get(deleteList,
                   vendor,
                   carrier,
                   serviceTypeCode,
                   serviceSubTypeCode,
                   fltTktMerchInd,
                   date,
                   ticketDate);
  }
}

const std::vector<OptionalServicesInfo*>&
OptionalServicesDAO::get(DeleteList& del,
                         const VendorCode& vendor,
                         const CarrierCode& carrier,
                         const ServiceTypeCode& serviceTypeCode,
                         const ServiceSubTypeCode& serviceSubTypeCode,
                         Indicator fltTktMerchInd,
                         const DateTime& date,
                         const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  OptionalServicesKey key(vendor, carrier, fltTktMerchInd);
  DAOCache::pointer_type ptr = cache().get(key);

  return *applyFilter(
             del,
             ptr,
             boost::bind<bool>(IsNotEffectiveG<OptionalServicesInfo>(date, ticketDate), _1) ||
                 !boost::bind<bool>(
                     IsSame<const ServiceTypeCode>(serviceTypeCode),
                     _1,
                     static_cast<const ServiceTypeCode& (OptionalServicesInfo::*)(void) const>(
                         &OptionalServicesInfo::serviceTypeCode)) ||
                 !boost::bind<bool>(
                     IsSame<const ServiceSubTypeCode>(serviceSubTypeCode),
                     _1,
                     static_cast<const ServiceSubTypeCode& (OptionalServicesInfo::*)(void) const>(
                         &OptionalServicesInfo::serviceSubTypeCode)));
}

std::vector<OptionalServicesInfo*>*
OptionalServicesDAO::create(OptionalServicesKey key)
{
  std::vector<OptionalServicesInfo*>* ret = new std::vector<OptionalServicesInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetOptionalServices fnc(dbAdapter->getAdapter());
    fnc.findOptionalServicesInfo(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in OptionalServicesDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
OptionalServicesDAO::destroy(OptionalServicesKey key, std::vector<OptionalServicesInfo*>* recs)
{
  destroyContainer(recs);
}

OptionalServicesKey
OptionalServicesDAO::createKey(OptionalServicesInfo* info)
{
  return OptionalServicesKey(info->vendor(), info->carrier(), info->fltTktMerchInd());
}

void
OptionalServicesDAO::load()
{
  // not pre_loading
  StartupLoaderNoDB<OptionalServicesInfo, OptionalServicesDAO>();
}

size_t
OptionalServicesDAO::invalidate(const ObjectKey& objectKey)
{
  OptionalServicesKey invKey;

  if (!translateKey(objectKey, invKey))
  {
    LOG4CXX_ERROR(_logger, "Cache notification key translation error");
    return 0;
  }

  // Flush all possible OptionalServicesInfo variations of ObjectKey
  std::shared_ptr<std::vector<OptionalServicesKey>> cacheKeys = cache().keys();
  std::vector<OptionalServicesKey>* pCacheKeys = cacheKeys.get();

  if (pCacheKeys == nullptr)
  {
    LOG4CXX_DEBUG(_logger, "No cache entries to iterate");
    return 0;
  }

  uint32_t nKeys = pCacheKeys->size();
  size_t result(0);
  for (uint32_t i = 0; i < nKeys; ++i)
  {
    OptionalServicesKey cacheKey = (*pCacheKeys)[i];

    if (invKey._a == cacheKey._a && invKey._b == cacheKey._b)
    {
      result += DataAccessObject<OptionalServicesKey, std::vector<OptionalServicesInfo*> >::invalidate(
          cacheKey);
    }
  }
  return result;
}

sfc::CompressedData*
OptionalServicesDAO::compress(const std::vector<OptionalServicesInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<OptionalServicesInfo*>*
OptionalServicesDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<OptionalServicesInfo>(compressed);
}

///////////////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////////////
log4cxx::LoggerPtr
OptionalServicesHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.OptionalServicesHistoricalDAO"));

std::string
OptionalServicesHistoricalDAO::_name("OptionalServicesHistorical");
std::string
OptionalServicesHistoricalDAO::_cacheClass("Rules");
DAOHelper<OptionalServicesHistoricalDAO>
OptionalServicesHistoricalDAO::_helper(_name);
OptionalServicesHistoricalDAO* OptionalServicesHistoricalDAO::_instance = nullptr;

OptionalServicesHistoricalDAO&
OptionalServicesHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<OptionalServicesInfo*>&
OptionalServicesHistoricalDAO::get(DeleteList& del,
                                   const VendorCode& vendor,
                                   const CarrierCode& carrier,
                                   const ServiceTypeCode& serviceTypeCode,
                                   const ServiceSubTypeCode& serviceSubTypeCode,
                                   Indicator fltTktMerchInd,
                                   const DateTime& date,
                                   const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  OptionalServicesHistoricalKey key(vendor, carrier, fltTktMerchInd);
  DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<OptionalServicesInfo*>* ret = new std::vector<OptionalServicesInfo*>;
  del.adopt(ret);
  remove_copy_if(
      ptr->begin(),
      ptr->end(),
      back_inserter(*ret),
      boost::bind<bool>(IsNotEffectiveH<OptionalServicesInfo>(date, ticketDate), _1) ||
          !boost::bind<bool>(
              IsSame<const ServiceTypeCode>(serviceTypeCode),
              _1,
              static_cast<const ServiceTypeCode& (OptionalServicesInfo::*)(void) const>(
                  &OptionalServicesInfo::serviceTypeCode)) ||
          !boost::bind<bool>(
              IsSame<const ServiceSubTypeCode>(serviceSubTypeCode),
              _1,
              static_cast<const ServiceSubTypeCode& (OptionalServicesInfo::*)(void) const>(
                  &OptionalServicesInfo::serviceSubTypeCode)));

  return *ret;
}

std::vector<OptionalServicesInfo*>*
OptionalServicesHistoricalDAO::create(OptionalServicesHistoricalKey key)
{
  std::vector<OptionalServicesInfo*>* ret = new std::vector<OptionalServicesInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetOptionalServicesHistorical fnc(dbAdapter->getAdapter());
    fnc.findOptionalServicesInfo(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in OptionalServicesHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

size_t
OptionalServicesHistoricalDAO::invalidate(const ObjectKey& objectKey)
{
  OptionalServicesHistoricalKey invKey;

  if (!translateKey(objectKey, invKey))
  {
    LOG4CXX_ERROR(_logger, "Cache notification key translation error");
    return 0;
  }

  // Flush all possible OptionalServicesInfo variations of ObjectKey
  std::shared_ptr<std::vector<OptionalServicesHistoricalKey>> cacheKeys = cache().keys();
  std::vector<OptionalServicesHistoricalKey>* pCacheKeys = cacheKeys.get();

  if (pCacheKeys == nullptr)
  {
    LOG4CXX_DEBUG(_logger, "No cache entries to iterate");
    return 0;
  }

  uint32_t nKeys = pCacheKeys->size();
  size_t result(0);
  for (uint32_t i = 0; i < nKeys; ++i)
  {
    OptionalServicesHistoricalKey cacheKey = (*pCacheKeys)[i];

    if (invKey._a == cacheKey._a && invKey._b == cacheKey._b)
    {
      result += HistoricalDataAccessObject<OptionalServicesHistoricalKey,
                                 std::vector<OptionalServicesInfo*> >::invalidate(cacheKey);
    }
  }
  return result;
}

void
OptionalServicesHistoricalDAO::destroy(OptionalServicesHistoricalKey key,
                                       std::vector<OptionalServicesInfo*>* recs)
{
  std::vector<OptionalServicesInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
OptionalServicesHistoricalDAO::compress(const std::vector<OptionalServicesInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<OptionalServicesInfo*>*
OptionalServicesHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<OptionalServicesInfo>(compressed);
}

} // tse
