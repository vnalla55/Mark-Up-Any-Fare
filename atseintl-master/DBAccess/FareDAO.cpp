//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/FareDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DBHistoryServer.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/Queries/QueryGetDomIntFares.h"
#include "DBAccess/SITAFareInfo.h"

#include <algorithm>
#include <functional>

namespace tse
{

log4cxx::LoggerPtr
FareDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FareDAO"));

FareDAO&
FareDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

struct FareDAO::isEffective : public std::unary_function<const FareInfo*, bool>
{
  const IsEffectiveG<FareInfo> _isEffectiveG;
  bool _isFareDisplay;

  isEffective(const DateTime& startDate,
              const DateTime& endDate,
              const DateTime& ticketDate,
              bool isFareDisplay)
    : _isEffectiveG(startDate, endDate, ticketDate), _isFareDisplay(isFareDisplay)
  {
  }

  bool operator()(const FareInfo* rec) const
  {
    if (UNLIKELY(_isFareDisplay))
    {
      if (InhibitForFD<FareInfo>(rec))
        return false;
    }
    else
    {
      if (Inhibit<FareInfo>(rec))
        return false;
    }
    return _isEffectiveG(rec);
  }
};

struct FareDAO::isVendor : public std::unary_function<const FareInfo*, bool>
{
  const DateTime _date;
  const VendorCode& _vendor;

  isVendor(const DateTime& ticketDate, const VendorCode& vendor)
    : _date(ticketDate), _vendor(vendor)
  {
  }

  bool operator()(const FareInfo* rec) const
  {
    if (InhibitForFD<FareInfo>(rec))
      return false;
    if (rec->_vendor != _vendor)
      return false;
    return (rec->createDate() <= _date) && (_date <= rec->expireDate());
  }
};

const std::vector<const FareInfo*>&
getFaresByMarketCxrData(const LocCode& market1,
                        const LocCode& market2,
                        const CarrierCode& cxr,
                        const DateTime& startDate,
                        const DateTime& endDate,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical,
                        bool isFareDisplay)
{
  const LocCode& mkt1 = (market1 < market2) ? market1 : market2;
  const LocCode& mkt2 = (market1 < market2) ? market2 : market1;

  if (UNLIKELY(isHistorical))
  {
    FareHistoricalDAO& dao = FareHistoricalDAO::instance();
    return dao.get(deleteList, mkt1, mkt2, cxr, startDate, endDate, ticketDate, isFareDisplay);
  }
  else
  {
    FareDAO& dao = FareDAO::instance();
    return dao.get(deleteList, mkt1, mkt2, cxr, startDate, endDate, ticketDate, isFareDisplay);
  }
}

const std::vector<const FareInfo*>&
FareDAO::get(DeleteList& del,
             const LocCode& market1,
             const LocCode& market2,
             const CarrierCode& cxr,
             const DateTime& startDate,
             const DateTime& endDate,
             const DateTime& ticketDate,
             bool isFareDisplay)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareKey key(market1, market2, cxr);
  DAOCache::pointer_type ptr = cache().get(key);
  /*
  del.copy(ptr);
  std::vector<const FareInfo*>* ret = new std::vector<const FareInfo*>;
  del.adopt(ret);

  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret),
                 not1(isEffective(startDate, endDate, ticketDate, isFareDisplay)));
  return *ret;
  */
  return *(applyFilter(del, ptr, not1(isEffective(startDate, endDate, ticketDate, isFareDisplay))));
}

const std::vector<const FareInfo*>&
getFaresByMarketCxrData(const LocCode& market1,
                        const LocCode& market2,
                        const CarrierCode& cxr,
                        const VendorCode& vendor,
                        DeleteList& deleteList,
                        const DateTime& ticketDate,
                        bool isHistorical)
{
  const LocCode& mkt1 = (market1 < market2) ? market1 : market2;
  const LocCode& mkt2 = (market1 < market2) ? market2 : market1;

  if (UNLIKELY(isHistorical))
  {
    FareHistoricalDAO& dao = FareHistoricalDAO::instance();
    return dao.get(deleteList, mkt1, mkt2, cxr, vendor, ticketDate);
  }
  else
  {
    FareDAO& dao = FareDAO::instance();
    return dao.get(deleteList, mkt1, mkt2, cxr, vendor, ticketDate);
  }
}

const std::vector<const FareInfo*>&
FareDAO::get(DeleteList& del,
             const LocCode& market1,
             const LocCode& market2,
             const CarrierCode& cxr,
             const VendorCode& vendor,
             const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareKey key(market1, market2, cxr);
  DAOCache::pointer_type ptr = cache().get(key);
  /*
  del.copy(ptr);
  std::vector<const FareInfo*>* ret = new std::vector<const FareInfo*>;
  del.adopt(ret);

  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), not1(isVendor(ticketDate, vendor)));
  return *ret;
  */
  return *(applyFilter(del, ptr, not1(isVendor(ticketDate, vendor))));
}

void
loadFaresForMarketData(const LocCode& market1,
                       const LocCode& market2,
                       const std::vector<CarrierCode>& cxr,
                       const DateTime& ticketDate,
                       bool isHistorical)
{
  const LocCode& mkt1 = (market1 < market2) ? market1 : market2;
  const LocCode& mkt2 = (market1 < market2) ? market2 : market1;

  if (isHistorical)
  {
    FareHistoricalDAO& dao = FareHistoricalDAO::instance();
    dao.loadFaresForMarket(mkt1, mkt2, cxr, ticketDate);
  }
  else
  {
    FareDAO& dao = FareDAO::instance();
    dao.loadFaresForMarket(mkt1, mkt2, cxr);
  }
}

void
FareDAO::loadFaresForMarket(const LocCode& market1,
                            const LocCode& market2,
                            const std::vector<CarrierCode>& carriersParm)
{
  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<CarrierCode> cxr(carriersParm);

  FareKey key(market1, market2);

  // remove any carriers that are already in the cache
  std::vector<CarrierCode>::iterator i = cxr.begin();
  while (i != cxr.end())
  {
    key._c = *i;
    if (cache().getIfResident(key).get() != nullptr)
    {
      i = cxr.erase(i);
    }
    else
    {
      ++i;
    }
  }

  if (cxr.empty())
  {
    return;
  }

  std::vector<const FareInfo*> fareInfo;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  QueryGetDomFares df(dbAdapter->getAdapter());
  df.findFares(fareInfo, market1, market2, cxr);
  std::map<FareKey, std::vector<const FareInfo*>*> m;

  // make sure that we insert an item for each vector in the
  // carrier list, so even if some items returned no results at
  // all, we will insert an empty list of fares, and avoid
  // having to do another query in the future
  for (std::vector<CarrierCode>::const_iterator i = cxr.begin(); i != cxr.end(); ++i)
  {
    key._c = *i;
    m.insert(
        std::pair<FareKey, std::vector<const FareInfo*>*>(key, new std::vector<const FareInfo*>));
  }

  for (std::vector<const FareInfo*>::const_iterator i = fareInfo.begin(); i != fareInfo.end(); ++i)
  {
    key._c = (*i)->_carrier;

    std::vector<const FareInfo*>*& ptr = m[key];

    if (ptr != nullptr)
    {
      ptr->push_back(*i);
    }
  }

  for (std::map<FareKey, std::vector<const FareInfo*>*>::const_iterator i = m.begin(); i != m.end();
       ++i)
  {
    cache().put(i->first, i->second);
  }
}

bool
isRuleInFareMarketData(const LocCode& market1,
                       const LocCode& market2,
                       const CarrierCode& cxr,
                       const RuleNumber ruleNumber,
                       const DateTime& ticketDate,
                       bool isHistorical)
{
  const LocCode& mkt1 = (market1 < market2) ? market1 : market2;
  const LocCode& mkt2 = (market1 < market2) ? market2 : market1;

  if (isHistorical)
  {
    FareHistoricalDAO& dao = FareHistoricalDAO::instance();
    return dao.isRuleInFareMarket(mkt1, mkt2, cxr, ruleNumber, ticketDate);
  }
  else
  {
    FareDAO& dao = FareDAO::instance();
    return dao.isRuleInFareMarket(mkt1, mkt2, cxr, ruleNumber);
  }
}

bool
FareDAO::isRuleInFareMarket(const LocCode& market1,
                            const LocCode& market2,
                            const CarrierCode& cxr,
                            const RuleNumber ruleNumber)
{
  FareKey key(market1, market2, cxr);
  DAOCache::pointer_type ptr = cache().get(key);

  DAOCache::value_type::const_iterator i = ptr->begin();
  for (; i != ptr->end(); i++)
  {
    if ((*i)->_ruleNumber == ruleNumber)
      return true;
  }
  return false;
}

std::vector<const FareInfo*>*
FareDAO::create(FareKey key)
{
  std::vector<const FareInfo*>* ret = new std::vector<const FareInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetDomFares df(dbAdapter->getAdapter());
    df.findFares(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FareDAO::destroy(FareKey key, std::vector<const FareInfo*>* recs)
{
  if (recs == nullptr)
    return;
  std::vector<const FareInfo*>::const_iterator it(recs->begin());
  std::vector<const FareInfo*>::const_iterator itend(recs->end());
  for (; it != itend; ++it)
    delete *it;

  delete recs;
}

FareKey
FareDAO::createKey(const FareInfo* info)
{
  return FareKey(info->market1(), info->market2(), info->carrier());
}

typedef std::vector<const tse::FareInfo*> MYFAREDATA;

void
FareDAO::load()
{
  StartupLoaderNoDB<FareInfo, FareDAO>();
}

sfc::CompressedData*
FareDAO::compress(const std::vector<const FareInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<const FareInfo*>*
FareDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<const FareInfo>(compressed);
}

std::string
FareDAO::_name("Fare");
std::string
FareDAO::_cacheClass("Fares");

DAOHelper<FareDAO>
FareDAO::_helper(_name);

FareDAO* FareDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: FareHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
FareHistoricalDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FareHistoricalDAO"));
FareHistoricalDAO&
FareHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

struct FareHistoricalDAO::isEffective : public std::unary_function<const FareInfo*, bool>
{
  const IsEffectiveH<FareInfo> _isEffectiveH;
  bool _isFareDisplay;

  isEffective(const DateTime& startDate,
              const DateTime& endDate,
              const DateTime& ticketDate,
              bool isFareDisplay)
    : _isEffectiveH(startDate, endDate, ticketDate), _isFareDisplay(isFareDisplay)
  {
  }

  bool operator()(const FareInfo* rec) const
  {
    if (_isFareDisplay)
    {
      if (InhibitForFD<FareInfo>(rec))
        return false;
    }
    else
    {
      if (Inhibit<FareInfo>(rec))
        return false;
    }
    return _isEffectiveH(rec);
  }
};

struct FareHistoricalDAO::isVendor : public std::unary_function<const FareInfo*, bool>
{
  const IsCurrentH<FareInfo> _isCurrentH;
  const VendorCode& _vendor;

  isVendor(const DateTime& ticketDate, const VendorCode& vendor)
    : _isCurrentH(ticketDate), _vendor(vendor)
  {
  }

  bool operator()(const FareInfo* rec) const
  {
    if (InhibitForFD<FareInfo>(rec))
      return false;
    if (rec->_vendor != _vendor)
      return false;
    return _isCurrentH(rec);
  }
};

const std::vector<const FareInfo*>&
FareHistoricalDAO::get(DeleteList& del,
                       const LocCode& market1,
                       const LocCode& market2,
                       const CarrierCode& cxr,
                       const DateTime& startDate,
                       const DateTime& endDate,
                       const DateTime& ticketDate,
                       bool isFareDisplay)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareHistoricalKey key(market1, market2, cxr);
  DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<const FareInfo*>* ret = new std::vector<const FareInfo*>;
  del.adopt(ret);

  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 not1(isEffective(startDate, endDate, ticketDate, isFareDisplay)));
  return *ret;
}

const std::vector<const FareInfo*>&
FareHistoricalDAO::get(DeleteList& del,
                       const LocCode& market1,
                       const LocCode& market2,
                       const CarrierCode& cxr,
                       const VendorCode& vendor,
                       const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareHistoricalKey key(market1, market2, cxr);
  DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<const FareInfo*>* ret = new std::vector<const FareInfo*>;
  del.adopt(ret);

  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), not1(isVendor(ticketDate, vendor)));
  return *ret;
}

void
FareHistoricalDAO::loadFaresForMarket(const LocCode& market1,
                                      const LocCode& market2,
                                      const std::vector<CarrierCode>& carriersParm,
                                      const DateTime& ticketDate)
{
  std::vector<CarrierCode> cxr(carriersParm);

  FareHistoricalKey key(market1, market2);
  DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);

  // remove any carriers that are already in the cache
  std::vector<CarrierCode>::iterator i = cxr.begin();
  while (i != cxr.end())
  {
    key._c = *i;
    if (cache().getIfResident(key).get() != nullptr)
    {
      i = cxr.erase(i);
    }
    else
    {
      ++i;
    }
  }

  if (cxr.empty())
  {
    return;
  }

  std::vector<const FareInfo*> fareInfo;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  QueryGetDomFaresHistorical df(dbAdapter->getAdapter());
  df.findFares(fareInfo, market1, market2, cxr, key._d, key._e);
  std::map<FareHistoricalKey, std::vector<const FareInfo*>*> m;

  // make sure that we insert an item for each vector in the
  // carrier list, so even if some items returned no results at
  // all, we will insert an empty list of fares, and avoid
  // having to do another query in the future
  for (std::vector<CarrierCode>::const_iterator i = cxr.begin(); i != cxr.end(); ++i)
  {
    key._c = *i;
    m.insert(std::pair<FareHistoricalKey, std::vector<const FareInfo*>*>(
        key, new std::vector<const FareInfo*>));
  }

  for (std::vector<const FareInfo*>::const_iterator i = fareInfo.begin(); i != fareInfo.end(); ++i)
  {
    key._c = (*i)->_carrier;

    std::vector<const FareInfo*>*& ptr = m[key];

    if (ptr != nullptr)
    {
      ptr->push_back(*i);
    }
  }

  for (std::map<FareHistoricalKey, std::vector<const FareInfo*>*>::const_iterator i = m.begin();
       i != m.end();
       ++i)
  {
    cache().put(i->first, i->second);
  }
}

bool
FareHistoricalDAO::isRuleInFareMarket(const LocCode& market1,
                                      const LocCode& market2,
                                      const CarrierCode& cxr,
                                      const RuleNumber ruleNumber,
                                      const DateTime& ticketDate)
{
  FareHistoricalKey key(market1, market2, cxr);
  DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);

  DAOCache::value_type::const_iterator i = ptr->begin();
  for (; i != ptr->end(); i++)
  {
    if ((*i)->_ruleNumber == ruleNumber && (*i)->createDate() <= ticketDate &&
        (*i)->expireDate() >= ticketDate)
      return true;
  }
  return false;
}

std::vector<const FareInfo*>*
FareHistoricalDAO::create(FareHistoricalKey key)
{
  std::vector<const FareInfo*>* ret = new std::vector<const FareInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetDomFaresHistorical df(dbAdapter->getAdapter());
    df.findFares(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FareHistoricalDAO::load()
{
  StartupLoaderNoDB<FareInfo, FareHistoricalDAO>();
}

FareHistoricalKey
FareHistoricalDAO::createKey(const FareInfo* info,
                             const DateTime& startDate,
                             const DateTime& endDate)
{
  return FareHistoricalKey(info->market1(), info->market2(), info->carrier(), startDate, endDate);
}

void
FareHistoricalDAO::destroy(FareHistoricalKey key, std::vector<const FareInfo*>* recs)
{
  if (recs == nullptr)
    return;
  std::vector<const FareInfo*>::const_iterator it(recs->begin());
  std::vector<const FareInfo*>::const_iterator itend(recs->end());
  for (; it != itend; ++it)
    delete *it;

  delete recs;
}

sfc::CompressedData*
FareHistoricalDAO::compress(const std::vector<const FareInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<const FareInfo*>*
FareHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<const FareInfo>(compressed);
}

std::string
FareHistoricalDAO::_name("FareHistorical");
std::string
FareHistoricalDAO::_cacheClass("Fares");
DAOHelper<FareHistoricalDAO>
FareHistoricalDAO::_helper(_name);

FareHistoricalDAO* FareHistoricalDAO::_instance = nullptr;

} // namespace tse
