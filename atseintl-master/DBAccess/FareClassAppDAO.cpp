//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/FareClassAppDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareClassAppInfo.h"
#include "DBAccess/Queries/QueryGetFareClassApp.h"

namespace tse
{
log4cxx::LoggerPtr
FareClassAppDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FareClassAppDAO"));
FareClassAppDAO&
FareClassAppDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

struct FareClassAppDAO::isNotApplicable : public std::unary_function<const FareClassAppInfo*, bool>
{
public:
  const IsNotCurrentG<FareClassAppInfo> _isNotCurrentG;

  isNotApplicable(const DateTime& ticketDate) : _isNotCurrentG(ticketDate) {}

  bool operator()(const FareClassAppInfo* rec) const
  {
    return (_isNotCurrentG(rec) || Inhibit<FareClassAppInfo>(rec));
  }
};

struct FareClassAppDAO::isNotApplicableByTravelDT : public std::unary_function<const FareClassAppInfo*, bool>
{
public:
  const IsNotEffectiveG<FareClassAppInfo> _isNotEffectiveG;

  isNotApplicableByTravelDT(const DateTime& ticketDate, const DateTime& travelDate) :
                                          _isNotEffectiveG(travelDate, ticketDate) {}

  bool operator()(const FareClassAppInfo* rec) const
  {
    return ( _isNotEffectiveG(rec) || Inhibit<FareClassAppInfo>(rec));
  }
};

struct FareClassAppDAO::isNotApplicableForFD
    : public std::unary_function<const FareClassAppInfo*, bool>
{
public:
  const IsNotCurrentG<FareClassAppInfo> _isNotCurrentG;

  isNotApplicableForFD(const DateTime& ticketDate) : _isNotCurrentG(ticketDate) {}

  bool operator()(const FareClassAppInfo* rec) const
  {
    return (_isNotCurrentG(rec) || InhibitForFD<FareClassAppInfo>(rec));
  }
};

struct FareClassAppDAO::isNotApplicableForFDByTravelDT
    : public std::unary_function<const FareClassAppInfo*, bool>
{
public:
  const IsNotEffectiveG<FareClassAppInfo> _isNotEffectiveG;

  isNotApplicableForFDByTravelDT(const DateTime& ticketDate,const DateTime& travelDate) :
                                                 _isNotEffectiveG(travelDate,ticketDate){}

  bool operator()(const FareClassAppInfo* rec) const
  {
    return (  _isNotEffectiveG(rec) || InhibitForFD<FareClassAppInfo>(rec));
  }
};

const std::vector<const FareClassAppInfo*>&
getFareClassAppData(const VendorCode& vendor,
                    const CarrierCode& carrier,
                    const TariffNumber& ruleTariff,
                    const RuleNumber& ruleNumber,
                    const FareClassCode& fareClass,
                    DeleteList& deleteList,
                    const DateTime& ticketDate,
                    bool isHistorical,
                    bool isFareDisplay)
{
  if (UNLIKELY(isHistorical))
  {
    FareClassAppHistoricalDAO& dao = FareClassAppHistoricalDAO::instance();

    const std::vector<const FareClassAppInfo*>& ret =
        isFareDisplay
            ? dao.getForFD(
                  deleteList, vendor, carrier, ruleTariff, ruleNumber, fareClass, ticketDate)
            : dao.get(deleteList, vendor, carrier, ruleTariff, ruleNumber, fareClass, ticketDate);

    return ret;
  }
  else
  {
    FareClassAppDAO& dao = FareClassAppDAO::instance();

    const std::vector<const FareClassAppInfo*>& ret =
        isFareDisplay
            ? dao.getForFD(
                  deleteList, vendor, carrier, ruleTariff, ruleNumber, fareClass, ticketDate)
            : dao.get(deleteList, vendor, carrier, ruleTariff, ruleNumber, fareClass, ticketDate);

    return ret;
  }
}

/* APO-37838  read record 1 by checking the travel date against the effective date */
const std::vector<const FareClassAppInfo*>&
getFareClassAppDataByTravelDT(const VendorCode& vendor,
                    const CarrierCode& carrier,
                    const TariffNumber& ruleTariff,
                    const RuleNumber& ruleNumber,
                    const FareClassCode& fareClass,
                    DateTime& travelDate,
                    DeleteList& deleteList,
                    const DateTime& ticketDate,
                    bool isHistorical,
                    bool isFareDisplay)
{
  if (isHistorical)
  {
    FareClassAppHistoricalDAO& dao = FareClassAppHistoricalDAO::instance();

    const std::vector<const FareClassAppInfo*>& ret =
        isFareDisplay
            ? dao.getForFDByTravelDT(
                  deleteList, vendor, carrier, ruleTariff, ruleNumber, fareClass, ticketDate,travelDate)
            : dao.getByTravelDT(deleteList, vendor, carrier, ruleTariff, ruleNumber, fareClass, ticketDate,travelDate);

    return ret;
  }
  else
  {
    FareClassAppDAO& dao = FareClassAppDAO::instance();

    const std::vector<const FareClassAppInfo*>& ret =
        isFareDisplay
            ? dao.getForFDByTravelDT(
                  deleteList, vendor, carrier, ruleTariff, ruleNumber, fareClass, ticketDate, travelDate)
            : dao.getByTravelDT(deleteList, vendor, carrier, ruleTariff, ruleNumber, fareClass, ticketDate,travelDate);

    return ret;
  }
}
const std::vector<const FareClassAppInfo*>&
FareClassAppDAO::get(DeleteList& del,
                     const VendorCode& vendor,
                     const CarrierCode& carrier,
                     const TariffNumber& ruleTariff,
                     const RuleNumber& ruleNumber,
                     const FareClassCode& fareClass,
                     const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareClassAppKey key(vendor, carrier, ruleTariff, ruleNumber, fareClass);
  DAOCache::pointer_type ptr = cache().get(key);
  /*
  del.copy(ptr);
  std::vector<const FareClassAppInfo*>* ret = new std::vector<const FareClassAppInfo*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), isNotApplicable(ticketDate));
  return *ret;
  */
  return *(applyFilter(del, ptr, isNotApplicable(ticketDate)));
}

const std::vector<const FareClassAppInfo*>&
FareClassAppDAO::getByTravelDT(DeleteList& del,
                     const VendorCode& vendor,
                     const CarrierCode& carrier,
                     const TariffNumber& ruleTariff,
                     const RuleNumber& ruleNumber,
                     const FareClassCode& fareClass,
                     const DateTime& ticketDate,
                     const DateTime& travelDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareClassAppKey key(vendor, carrier, ruleTariff, ruleNumber, fareClass);
  DAOCache::pointer_type ptr = cache().get(key);
  return *(applyFilter(del, ptr, isNotApplicableByTravelDT(ticketDate,travelDate)));
}

const std::vector<const FareClassAppInfo*>&
FareClassAppDAO::getForFD(DeleteList& del,
                          const VendorCode& vendor,
                          const CarrierCode& carrier,
                          const TariffNumber& ruleTariff,
                          const RuleNumber& ruleNumber,
                          const FareClassCode& fareClass,
                          const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetFDCallCount++;

  FareClassAppKey key(vendor, carrier, ruleTariff, ruleNumber, fareClass);
  DAOCache::pointer_type ptr = cache().get(key);
  /*
  del.copy(ptr);
  std::vector<const FareClassAppInfo*>* ret = new std::vector<const FareClassAppInfo*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), isNotApplicableForFD(ticketDate));
  return *ret;
  */
  return *(applyFilter(del, ptr, isNotApplicableForFD(ticketDate)));
}

const std::vector<const FareClassAppInfo*>&
FareClassAppDAO::getForFDByTravelDT(DeleteList& del,
                          const VendorCode& vendor,
                          const CarrierCode& carrier,
                          const TariffNumber& ruleTariff,
                          const RuleNumber& ruleNumber,
                          const FareClassCode& fareClass,
                          const DateTime& ticketDate,
                          const DateTime& travelDate)
{
  // Track calls for code coverage
  _codeCoverageGetFDCallCount++;

  FareClassAppKey key(vendor, carrier, ruleTariff, ruleNumber, fareClass);
  DAOCache::pointer_type ptr = cache().get(key);
  return *(applyFilter(del, ptr, isNotApplicableForFDByTravelDT(ticketDate,travelDate)));
}
struct FareClassAppDAO::groupByKey
{
public:
  FareClassAppKey prevKey;

  DAOCache& cache;

  groupByKey() : cache(FareClassAppDAO::instance().cache()), ptr(nullptr) {}

  std::vector<const FareClassAppInfo*>* ptr;

  void operator()(const FareClassAppInfo* info)
  {
    FareClassAppKey key(
        info->_vendor, info->_carrier, info->_ruleTariff, info->_ruleNumber, info->_fareClass);
    if (!(key == prevKey))
    {
      ptr = new std::vector<const FareClassAppInfo*>;
      cache.put(key, ptr);
      prevKey = key;
    }

    ptr->push_back(info);
  }
};

void
FareClassAppDAO::put(const std::vector<const FareClassAppInfo*>& recs)
{
  std::for_each(recs.begin(), recs.end(), groupByKey());
}

std::vector<const FareClassAppInfo*>*
FareClassAppDAO::create(FareClassAppKey key)
{
  std::vector<const FareClassAppInfo*>* ret = new std::vector<const FareClassAppInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareClassApp fca(dbAdapter->getAdapter());
    fca.findFareClassApp(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareClassAppDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

FareClassAppKey
FareClassAppDAO::createKey(const FareClassAppInfo* info)
{
  return FareClassAppKey(
      info->_vendor, info->_carrier, info->_ruleTariff, info->_ruleNumber, info->_fareClass);
}

void
FareClassAppDAO::load()
{
  StartupLoader<QueryGetAllFareClassApp, FareClassAppInfo, FareClassAppDAO>();
}

void
FareClassAppDAO::destroy(FareClassAppKey key, std::vector<const FareClassAppInfo*>* recs)
{
  std::vector<const FareClassAppInfo*>::iterator i;
  FareClassAppSegInfoList::const_iterator j;
  for (i = recs->begin(); i != recs->end(); ++i)
  {
    const FareClassAppSegInfoList& segs = (*i)->_segs;
    for (j = segs.begin(); j != segs.end(); ++j)
      delete *j;
    delete *i;
  }
  delete recs;
}

size_t
FareClassAppDAO::clear()
{
  size_t result(cache().clear());
  LOG4CXX_ERROR(_logger, "FareClassApp cache cleared");
  return result;
}

sfc::CompressedData*
FareClassAppDAO::compress(const std::vector<const FareClassAppInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<const FareClassAppInfo*>*
FareClassAppDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<const FareClassAppInfo>(compressed);
}

std::string
FareClassAppDAO::_name("FareClassApp");
std::string
FareClassAppDAO::_cacheClass("Fares");
DAOHelper<FareClassAppDAO>
FareClassAppDAO::_helper(_name);
FareClassAppDAO* FareClassAppDAO::_instance = nullptr;

////////////////////////////////////////////////////////
// Historical DAO Object
////////////////////////////////////////////////////////
log4cxx::LoggerPtr
FareClassAppHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareClassAppHistoricalDAO"));
FareClassAppHistoricalDAO&
FareClassAppHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

struct FareClassAppHistoricalDAO::isNotApplicable
    : public std::unary_function<const FareClassAppInfo*, bool>
{
public:
  const IsNotCurrentH<FareClassAppInfo> _isNotCurrentH;

  isNotApplicable(const DateTime& ticketDate) : _isNotCurrentH(ticketDate) {}

  bool operator()(const FareClassAppInfo* rec) const
  {
    return ( // rec->_fareClass != _fareClassCode ||
        _isNotCurrentH(rec) || Inhibit<FareClassAppInfo>(rec));
  }
};

struct FareClassAppHistoricalDAO::isNotApplicableByTravelDT
    : public std::unary_function<const FareClassAppInfo*, bool>
{
public:
  const IsNotEffectiveH<FareClassAppInfo> _isNotEffectiveH;

  isNotApplicableByTravelDT(const DateTime& ticketDate,const DateTime& travelDate) :
                              _isNotEffectiveH(travelDate, ticketDate) {}

  bool operator()(const FareClassAppInfo* rec) const
  {
    return (_isNotEffectiveH(rec) ||  Inhibit<FareClassAppInfo>(rec));
  }
};

struct FareClassAppHistoricalDAO::isNotApplicableForFD
    : public std::unary_function<const FareClassAppInfo*, bool>
{
public:
  const IsNotCurrentH<FareClassAppInfo> _isNotCurrentH;

  isNotApplicableForFD(const DateTime& ticketDate) : _isNotCurrentH(ticketDate) {}

  bool operator()(const FareClassAppInfo* rec) const
  {
    return (_isNotCurrentH(rec) || InhibitForFD<FareClassAppInfo>(rec));
  }
};
struct FareClassAppHistoricalDAO::isNotApplicableForFDByTravelDT
    : public std::unary_function<const FareClassAppInfo*, bool>
{
public:
  const IsNotEffectiveH<FareClassAppInfo> _isNotEffectiveH;

  isNotApplicableForFDByTravelDT(const DateTime& ticketDate, const DateTime& travelDate) : _isNotEffectiveH(travelDate, ticketDate) {}

  bool operator()(const FareClassAppInfo* rec) const
  {
    return ( _isNotEffectiveH(rec) || InhibitForFD<FareClassAppInfo>(rec));
  }
};

const std::vector<const FareClassAppInfo*>&
FareClassAppHistoricalDAO::get(DeleteList& del,
                               const VendorCode& vendor,
                               const CarrierCode& carrier,
                               const TariffNumber& ruleTariff,
                               const RuleNumber& ruleNumber,
                               const FareClassCode& fareClass,
                               const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareClassAppHistoricalKey key(vendor, carrier, ruleTariff, ruleNumber, fareClass);
  DAOUtils::getDateRange(ticketDate, key._f, key._g, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<const FareClassAppInfo*>* ret = new std::vector<const FareClassAppInfo*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), isNotApplicable(ticketDate));
  return *ret;
}
const std::vector<const FareClassAppInfo*>&
FareClassAppHistoricalDAO::getByTravelDT(DeleteList& del,
                               const VendorCode& vendor,
                               const CarrierCode& carrier,
                               const TariffNumber& ruleTariff,
                               const RuleNumber& ruleNumber,
                               const FareClassCode& fareClass,
                               const DateTime& ticketDate,
                               const DateTime& travelDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareClassAppHistoricalKey key(vendor, carrier, ruleTariff, ruleNumber, fareClass);
  DAOUtils::getDateRange(ticketDate, key._f, key._g, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<const FareClassAppInfo*>* ret = new std::vector<const FareClassAppInfo*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), isNotApplicableByTravelDT(ticketDate,travelDate));
  return *ret;
}

const std::vector<const FareClassAppInfo*>&
FareClassAppHistoricalDAO::getForFD(DeleteList& del,
                                    const VendorCode& vendor,
                                    const CarrierCode& carrier,
                                    const TariffNumber& ruleTariff,
                                    const RuleNumber& ruleNumber,
                                    const FareClassCode& fareClass,
                                    const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetFDCallCount++;

  FareClassAppHistoricalKey key(vendor, carrier, ruleTariff, ruleNumber, fareClass);
  DAOUtils::getDateRange(ticketDate, key._f, key._g, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<const FareClassAppInfo*>* ret = new std::vector<const FareClassAppInfo*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), isNotApplicableForFD(ticketDate));
  return *ret;
}

const std::vector<const FareClassAppInfo*>&
FareClassAppHistoricalDAO::getForFDByTravelDT(DeleteList& del,
                                    const VendorCode& vendor,
                                    const CarrierCode& carrier,
                                    const TariffNumber& ruleTariff,
                                    const RuleNumber& ruleNumber,
                                    const FareClassCode& fareClass,
                                    const DateTime& ticketDate,
                                    const DateTime& travelDate)
{
  // Track calls for code coverage
  _codeCoverageGetFDCallCount++;

  FareClassAppHistoricalKey key(vendor, carrier, ruleTariff, ruleNumber, fareClass);
  DAOUtils::getDateRange(ticketDate, key._f, key._g, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<const FareClassAppInfo*>* ret = new std::vector<const FareClassAppInfo*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), isNotApplicableForFDByTravelDT(ticketDate,travelDate));
  return *ret;
}
std::vector<const FareClassAppInfo*>*
FareClassAppHistoricalDAO::create(FareClassAppHistoricalKey key)
{
  std::vector<const FareClassAppInfo*>* ret = new std::vector<const FareClassAppInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetFareClassAppHistorical fca(dbAdapter->getAdapter());
    fca.findFareClassApp(*ret, key._a, key._b, key._c, key._d, key._e, key._f, key._g);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareClassAppHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FareClassAppHistoricalDAO::load()
{
  StartupLoaderNoDB<FareClassAppInfo, FareClassAppHistoricalDAO>();
}

FareClassAppHistoricalKey
FareClassAppHistoricalDAO::createKey(const FareClassAppInfo* info,
                                     const DateTime& startDate,
                                     const DateTime& endDate)
{
  return FareClassAppHistoricalKey(info->_vendor,
                                   info->_carrier,
                                   info->_ruleTariff,
                                   info->_ruleNumber,
                                   info->_fareClass,
                                   startDate,
                                   endDate);
}

void
FareClassAppHistoricalDAO::destroy(FareClassAppHistoricalKey key,
                                   std::vector<const FareClassAppInfo*>* recs)
{
  std::vector<const FareClassAppInfo*>::const_iterator itr(recs->begin()), itrend(recs->end());
  for (; itr != itrend; ++itr)
  {
    const FareClassAppSegInfoList& segs((*itr)->_segs);
    FareClassAppSegInfoList::const_iterator its(segs.begin()), itsend(segs.end());
    for (; its != itsend; ++its)
    {
      delete *its;
    }
    delete *itr;
  }
  delete recs;
}

sfc::CompressedData*
FareClassAppHistoricalDAO::compress(const std::vector<const FareClassAppInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<const FareClassAppInfo*>*
FareClassAppHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<const FareClassAppInfo>(compressed);
}

std::string
FareClassAppHistoricalDAO::_name("FareClassAppHistorical");
std::string
FareClassAppHistoricalDAO::_cacheClass("Fares");
DAOHelper<FareClassAppHistoricalDAO>
FareClassAppHistoricalDAO::_helper(_name);
FareClassAppHistoricalDAO* FareClassAppHistoricalDAO::_instance = nullptr;

} // namespace tse
