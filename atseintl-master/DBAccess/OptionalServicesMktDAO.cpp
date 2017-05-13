//-------------------------------------------------------------------------------
// Copyright 2011, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/OptionalServicesMktDAO.h"

#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/Queries/QueryGetOptionalServicesMkt.h"

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

struct CompareSeq
{
  bool
  operator()(const tse::OptionalServicesInfo* rec1, const tse::OptionalServicesInfo* rec2) const
  {
    return rec1->seqNo() < rec2->seqNo();
  }
};
}

namespace tse
{
log4cxx::LoggerPtr
OptionalServicesMktDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.OptionalServicesMktDAO"));

std::string
OptionalServicesMktDAO::_name("OptionalServicesMkt");
std::string
OptionalServicesMktDAO::_cacheClass("Rules");
DAOHelper<OptionalServicesMktDAO>
OptionalServicesMktDAO::_helper(_name);
OptionalServicesMktDAO* OptionalServicesMktDAO::_instance = nullptr;

OptionalServicesMktDAO&
OptionalServicesMktDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<OptionalServicesInfo*>&
getOptionalServicesMktData(const VendorCode& vendor,
                           const CarrierCode& carrier,
                           const LocCode& loc1,
                           const LocCode& loc2,
                           const ServiceTypeCode& serviceTypeCode,
                           const ServiceSubTypeCode& serviceSubTypeCode,
                           Indicator fltTktMerchInd,
                           const DateTime& date,
                           DeleteList& deleteList,
                           const DateTime& ticketDate,
                           bool isHistorical)
{
  if (isHistorical)
  {
    OptionalServicesMktHistoricalDAO& dao = OptionalServicesMktHistoricalDAO::instance();
    return dao.get(deleteList,
                   vendor,
                   carrier,
                   loc1,
                   loc2,
                   serviceTypeCode,
                   serviceSubTypeCode,
                   fltTktMerchInd,
                   date,
                   ticketDate);
  }
  else
  {
    OptionalServicesMktDAO& dao = OptionalServicesMktDAO::instance();
    return dao.get(deleteList,
                   vendor,
                   carrier,
                   loc1,
                   loc2,
                   serviceTypeCode,
                   serviceSubTypeCode,
                   fltTktMerchInd,
                   date,
                   ticketDate);
  }
}

const std::vector<OptionalServicesInfo*>&
OptionalServicesMktDAO::get(DeleteList& del,
                            const VendorCode& vendor,
                            const CarrierCode& carrier,
                            const LocCode& loc1,
                            const LocCode& loc2,
                            const ServiceTypeCode& serviceTypeCode,
                            const ServiceSubTypeCode& serviceSubTypeCode,
                            Indicator fltTktMerchInd,
                            const DateTime& date,
                            const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  OptionalServicesMktKey key(vendor, carrier, loc1, loc2);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<OptionalServicesInfo*>* ret = new std::vector<OptionalServicesInfo*>;
  del.adopt(ret);

  remove_copy_if(
      ptr->begin(),
      ptr->end(),
      back_inserter(*ret),
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
                  &OptionalServicesInfo::serviceSubTypeCode)) ||
          !boost::bind<bool>(IsSame<const Indicator>(fltTktMerchInd),
                             _1,
                             static_cast<const Indicator& (OptionalServicesInfo::*)(void) const>(
                                 &OptionalServicesInfo::fltTktMerchInd)));

  OptionalServicesMktKey emptyMktKey(vendor, carrier, "", "");
  ptr = cache().get(emptyMktKey);
  del.copy(ptr);

  remove_copy_if(
      ptr->begin(),
      ptr->end(),
      back_inserter(*ret),
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
                  &OptionalServicesInfo::serviceSubTypeCode)) ||
          !boost::bind<bool>(IsSame<const Indicator>(fltTktMerchInd),
                             _1,
                             static_cast<const Indicator& (OptionalServicesInfo::*)(void) const>(
                                 &OptionalServicesInfo::fltTktMerchInd)));

  std::stable_sort(ret->begin(), ret->end(), CompareSeq());

  return *ret;
}

std::vector<OptionalServicesInfo*>*
OptionalServicesMktDAO::create(OptionalServicesMktKey key)
{
  std::vector<OptionalServicesInfo*>* ret = new std::vector<OptionalServicesInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetOptionalServicesMkt fnc(dbAdapter->getAdapter());
    fnc.findOptionalServicesMktInfo(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in OptionalServicesMktDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
OptionalServicesMktDAO::destroy(OptionalServicesMktKey key,
                                std::vector<OptionalServicesInfo*>* recs)
{
  destroyContainer(recs);
}

OptionalServicesMktKey
OptionalServicesMktDAO::createKey(OptionalServicesInfo* info)
{
  return OptionalServicesMktKey(
      info->vendor(), info->carrier(), info->loc1().loc(), info->loc2().loc());
}

void
OptionalServicesMktDAO::load()
{
  // not pre_loading
  StartupLoaderNoDB<OptionalServicesInfo, OptionalServicesMktDAO>();
}

size_t
OptionalServicesMktDAO::invalidate(const ObjectKey& objectKey)
{
  OptionalServicesMktKey invKey;
  if (!translateKey(objectKey, invKey))
  {
    LOG4CXX_ERROR(_logger, "Cache notification key translation error");
    return 0;
  }

  // Flush all possible OptionalServicesInfo variations of ObjectKey
  std::shared_ptr<std::vector<OptionalServicesMktKey>> cacheKeys = cache().keys();
  std::vector<OptionalServicesMktKey>* pCacheKeys = cacheKeys.get();

  if (pCacheKeys == nullptr)
  {
    LOG4CXX_DEBUG(_logger, "No cache entries to iterate");
    return 0;
  }

  uint32_t nKeys = pCacheKeys->size();
  size_t result(0);
  for (uint32_t i = 0; (i < nKeys); ++i)
  {
    OptionalServicesMktKey cacheKey = (*pCacheKeys)[i];
    if (invKey._a == cacheKey._a && invKey._b == cacheKey._b)
    {
      result += DataAccessObject<OptionalServicesMktKey, std::vector<OptionalServicesInfo*> >::invalidate(
          cacheKey);
    }
  }
  return result;
}

///////////////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////////////
log4cxx::LoggerPtr
OptionalServicesMktHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.OptionalServicesMktHistoricalDAO"));

std::string
OptionalServicesMktHistoricalDAO::_name("OptionalServicesMktHistorical");
std::string
OptionalServicesMktHistoricalDAO::_cacheClass("Rules");
DAOHelper<OptionalServicesMktHistoricalDAO>
OptionalServicesMktHistoricalDAO::_helper(_name);
OptionalServicesMktHistoricalDAO* OptionalServicesMktHistoricalDAO::_instance = nullptr;

OptionalServicesMktHistoricalDAO&
OptionalServicesMktHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<OptionalServicesInfo*>&
OptionalServicesMktHistoricalDAO::get(DeleteList& del,
                                      const VendorCode& vendor,
                                      const CarrierCode& carrier,
                                      const LocCode& loc1,
                                      const LocCode& loc2,
                                      const ServiceTypeCode& serviceTypeCode,
                                      const ServiceSubTypeCode& serviceSubTypeCode,
                                      Indicator fltTktMerchInd,
                                      const DateTime& date,
                                      const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  OptionalServicesMktHistoricalKey key(vendor, carrier, loc1, loc2);
  DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
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
                  &OptionalServicesInfo::serviceSubTypeCode)) ||
          !boost::bind<bool>(IsSame<const Indicator>(fltTktMerchInd),
                             _1,
                             static_cast<const Indicator& (OptionalServicesInfo::*)(void) const>(
                                 &OptionalServicesInfo::fltTktMerchInd)));

  OptionalServicesMktHistoricalKey emptyMktKey(vendor, carrier, "", "");
  DAOUtils::getDateRange(ticketDate, emptyMktKey._e, emptyMktKey._f, _cacheBy);
  ptr = cache().get(emptyMktKey);
  del.copy(ptr);

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
                  &OptionalServicesInfo::serviceSubTypeCode)) ||
          !boost::bind<bool>(IsSame<const Indicator>(fltTktMerchInd),
                             _1,
                             static_cast<const Indicator& (OptionalServicesInfo::*)(void) const>(
                                 &OptionalServicesInfo::fltTktMerchInd)));

  std::stable_sort(ret->begin(), ret->end(), CompareSeq());

  return *ret;
}

std::vector<OptionalServicesInfo*>*
OptionalServicesMktHistoricalDAO::create(OptionalServicesMktHistoricalKey key)
{
  std::vector<OptionalServicesInfo*>* ret = new std::vector<OptionalServicesInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetOptionalServicesMktHistorical fnc(dbAdapter->getAdapter());
    fnc.findOptionalServicesMktInfo(*ret, key._a, key._b, key._c, key._d, key._e, key._f);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in OptionalServicesMktHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
OptionalServicesMktHistoricalDAO::destroy(OptionalServicesMktHistoricalKey key,
                                          std::vector<OptionalServicesInfo*>* recs)
{
  std::vector<OptionalServicesInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}
} // tse
