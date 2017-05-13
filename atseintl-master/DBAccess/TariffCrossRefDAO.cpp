//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/TariffCrossRefDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetTrfXRefByCxr.h"
#include "DBAccess/TariffCrossRefInfo.h"
#include "DBAccess/VendorCrossRef.h"
#include "DBAccess/VendorCrossRefDAO.h"

namespace tse
{
log4cxx::LoggerPtr
TariffCrossRefDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.TariffCrossRefDAO"));

TariffCrossRefDAO&
TariffCrossRefDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

struct TariffCrossRefDAO::matchFareTariff
    : public std::unary_function<const TariffCrossRefInfo*, bool>
{
public:
  const TariffNumber& _fareTariff;
  const IsEffectiveG<TariffCrossRefInfo> _isEffective;

  matchFareTariff(const TariffNumber& fareTariff, const DateTime& date, const DateTime& ticketDate)
    : _fareTariff(fareTariff), _isEffective(date, ticketDate)
  {
  }

  bool operator()(const TariffCrossRefInfo* rec) const
  {
    return rec->_fareTariff == _fareTariff && _isEffective(rec);
  }
};

struct TariffCrossRefDAO::matchRuleTariff
    : public std::unary_function<const TariffCrossRefInfo*, bool>
{
public:
  const TariffNumber& _ruleTariff;
  const IsEffectiveG<TariffCrossRefInfo> _isEffective;

  matchRuleTariff(const TariffNumber& ruleTariff, const DateTime& date, const DateTime& ticketDate)
    : _ruleTariff(ruleTariff), _isEffective(date, ticketDate)
  {
  }

  bool operator()(const TariffCrossRefInfo* rec) const
  {
    return rec->_ruleTariff == _ruleTariff && _isEffective(rec);
  }
};

struct TariffCrossRefDAO::matchGenRuleTariff
    : public std::unary_function<const TariffCrossRefInfo*, bool>
{
public:
  const TariffNumber& _ruleTariff;
  const IsEffectiveG<TariffCrossRefInfo> _isEffective;

  matchGenRuleTariff(const TariffNumber& ruleTariff,
                     const DateTime& date,
                     const DateTime& ticketDate)
    : _ruleTariff(ruleTariff), _isEffective(date, ticketDate)
  {
  }

  bool operator()(const TariffCrossRefInfo* rec) const
  {
    return rec->_governingTariff == _ruleTariff && _isEffective(rec);
  }
};

struct TariffCrossRefDAO::matchAddonTariff
    : public std::unary_function<const TariffCrossRefInfo*, bool>
{
public:
  const TariffNumber& _addonTariff;
  const IsEffectiveG<TariffCrossRefInfo> _isEffective;

  matchAddonTariff(const TariffNumber& addonTariff,
                   const DateTime& date,
                   const DateTime& ticketDate)
    : _addonTariff(addonTariff), _isEffective(date, ticketDate)
  {
  }

  bool operator()(const TariffCrossRefInfo* rec) const
  {
    return ((rec->_addonTariff1 == _addonTariff || rec->_addonTariff2 == _addonTariff) &&
            _isEffective(rec));
  }
};

const std::vector<TariffCrossRefInfo*>&
getTariffXRefData(const VendorCode& vendor,
                  const CarrierCode& carrier,
                  const RecordScope& crossRefType,
                  DeleteList& deleteList,
                  const DateTime& ticketDate,
                  bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    TariffCrossRefHistoricalDAO& dao = TariffCrossRefHistoricalDAO::instance();
    return dao.get(deleteList, vendor, carrier, crossRefType, ticketDate);
  }
  else
  {
    TariffCrossRefDAO& dao = TariffCrossRefDAO::instance();
    return dao.get(deleteList, vendor, carrier, crossRefType, ticketDate);
  }
}

const std::vector<TariffCrossRefInfo*>&
TariffCrossRefDAO::get(DeleteList& del,
                       const VendorCode& vendor,
                       const CarrierCode& carrier,
                       const RecordScope& crossRefType,
                       const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode tcrVendor = getTCRVendorData(vendor, del, ticketDate, false);
  TariffCrossRefKey key(tcrVendor, carrier, crossRefType);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  return ptr->tariffInfo();
}

const std::vector<TariffCrossRefInfo*>&
getTariffXRefByFareTariffData(const VendorCode& vendor,
                              const CarrierCode& carrier,
                              const RecordScope& crossRefType,
                              const TariffNumber& fareTariff,
                              const DateTime& date,
                              DeleteList& deleteList,
                              const DateTime& ticketDate,
                              bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    TariffCrossRefHistoricalDAO& dao = TariffCrossRefHistoricalDAO::instance();
    return dao.getByFareTariff(
        deleteList, vendor, carrier, crossRefType, fareTariff, date, ticketDate);
  }
  else
  {
    TariffCrossRefDAO& dao = TariffCrossRefDAO::instance();
    return dao.getByFareTariff(
        deleteList, vendor, carrier, crossRefType, fareTariff, date, ticketDate);
  }
}

const std::vector<TariffCrossRefInfo*>&
TariffCrossRefDAO::getByFareTariff(DeleteList& del,
                                   const VendorCode& vendor,
                                   const CarrierCode& carrier,
                                   const RecordScope& crossRefType,
                                   const TariffNumber& fareTariff,
                                   const DateTime& date,
                                   const DateTime& ticketDate)
{
  VendorCode tcrVendor = getTCRVendorData(vendor, del, ticketDate, false);
  TariffCrossRefKey key(tcrVendor, carrier, crossRefType);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<TariffCrossRefInfo*>* ret = nullptr;
  // std::vector<TariffCrossRefInfo*>* ret = new std::vector<TariffCrossRefInfo*>;
  // del.adopt(ret);
  // remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), not1(matchFareTariff(fareTariff,
  // date, ticketDate)));
  std::vector<TariffCrossRefInfo*>* tariffs = ptr->fareTariffs(fareTariff);
  if (tariffs != nullptr)
  {
    // remove_copy_if(tariffs->begin(), tariffs->end(), back_inserter(*ret),
    // IsNotEffectiveG<TariffCrossRefInfo>(date, ticketDate));
    ret = tariffs;
  }
  else
  {
    ret = new std::vector<TariffCrossRefInfo*>;
    del.adopt(ret);
  }
  return *ret;
}

const std::vector<TariffCrossRefInfo*>&
getTariffXRefByRuleTariffData(const VendorCode& vendor,
                              const CarrierCode& carrier,
                              const RecordScope& crossRefType,
                              const TariffNumber& ruleTariff,
                              const DateTime& date,
                              DeleteList& deleteList,
                              const DateTime& ticketDate,
                              bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    TariffCrossRefHistoricalDAO& dao = TariffCrossRefHistoricalDAO::instance();
    return dao.getByRuleTariff(
        deleteList, vendor, carrier, crossRefType, ruleTariff, date, ticketDate);
  }
  else
  {
    TariffCrossRefDAO& dao = TariffCrossRefDAO::instance();
    return dao.getByRuleTariff(
        deleteList, vendor, carrier, crossRefType, ruleTariff, date, ticketDate);
  }
}

const std::vector<TariffCrossRefInfo*>&
TariffCrossRefDAO::getByRuleTariff(DeleteList& del,
                                   const VendorCode& vendor,
                                   const CarrierCode& carrier,
                                   const RecordScope& crossRefType,
                                   const TariffNumber& ruleTariff,
                                   const DateTime& date,
                                   const DateTime& ticketDate)
{
  VendorCode tcrVendor = getTCRVendorData(vendor, del, ticketDate, false);
  TariffCrossRefKey key(tcrVendor, carrier, crossRefType);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<TariffCrossRefInfo*>* ret = nullptr;
  // std::vector<TariffCrossRefInfo*>* ret = new std::vector<TariffCrossRefInfo*>;
  // del.adopt(ret);
  // remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), not1(matchRuleTariff(ruleTariff,
  // date, ticketDate)));
  std::vector<TariffCrossRefInfo*>* tariffs = ptr->ruleTariffs(ruleTariff);
  if (tariffs != nullptr)
  {
    // remove_copy_if(tariffs->begin(), tariffs->end(), back_inserter(*ret),
    // IsNotEffectiveG<TariffCrossRefInfo>(date, ticketDate));
    ret = tariffs;
  }
  else
  {
    ret = new std::vector<TariffCrossRefInfo*>;
    del.adopt(ret);
  }
  return *ret;
}

const std::vector<TariffCrossRefInfo*>&
getTariffXRefByGenRuleTariffData(const VendorCode& vendor,
                                 const CarrierCode& carrier,
                                 const RecordScope& crossRefType,
                                 const TariffNumber& ruleTariff,
                                 const DateTime& date,
                                 DeleteList& deleteList,
                                 const DateTime& ticketDate,
                                 bool isHistorical)
{
  if (isHistorical)
  {
    TariffCrossRefHistoricalDAO& dao = TariffCrossRefHistoricalDAO::instance();
    return dao.getByGenRuleTariff(
        deleteList, vendor, carrier, crossRefType, ruleTariff, date, ticketDate);
  }
  else
  {
    TariffCrossRefDAO& dao = TariffCrossRefDAO::instance();
    return dao.getByGenRuleTariff(
        deleteList, vendor, carrier, crossRefType, ruleTariff, date, ticketDate);
  }
}

const std::vector<TariffCrossRefInfo*>&
TariffCrossRefDAO::getByGenRuleTariff(DeleteList& del,
                                      const VendorCode& vendor,
                                      const CarrierCode& carrier,
                                      const RecordScope& crossRefType,
                                      const TariffNumber& ruleTariff,
                                      const DateTime& date,
                                      const DateTime& ticketDate)
{
  VendorCode tcrVendor = getTCRVendorData(vendor, del, ticketDate, false);
  TariffCrossRefKey key(tcrVendor, carrier, crossRefType);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<TariffCrossRefInfo*>* ret = nullptr;
  // std::vector<TariffCrossRefInfo*>* ret = new std::vector<TariffCrossRefInfo*>;
  // del.adopt(ret);
  // remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret),
  // not1(matchGenRuleTariff(ruleTariff, date, ticketDate)));
  std::vector<TariffCrossRefInfo*>* tariffs = ptr->govRuleTariffs(ruleTariff);
  if (tariffs != nullptr)
  {
    // remove_copy_if(tariffs->begin(), tariffs->end(), back_inserter(*ret),
    // IsNotEffectiveG<TariffCrossRefInfo>(date, ticketDate));
    ret = tariffs;
  }
  else
  {
    ret = new std::vector<TariffCrossRefInfo*>;
    del.adopt(ret);
  }
  return *ret;
}

const std::vector<TariffCrossRefInfo*>&
getTariffXRefByAddonTariffData(const VendorCode& vendor,
                               const CarrierCode& carrier,
                               const RecordScope& crossRefType,
                               const TariffNumber& addonTariff,
                               const DateTime& date,
                               DeleteList& deleteList,
                               const DateTime& ticketDate,
                               bool isHistorical)
{
  if (isHistorical)
  {
    TariffCrossRefHistoricalDAO& dao = TariffCrossRefHistoricalDAO::instance();
    return dao.getByAddonTariff(
        deleteList, vendor, carrier, crossRefType, addonTariff, date, ticketDate);
  }
  else
  {
    TariffCrossRefDAO& dao = TariffCrossRefDAO::instance();
    return dao.getByAddonTariff(
        deleteList, vendor, carrier, crossRefType, addonTariff, date, ticketDate);
  }
}

const std::vector<TariffCrossRefInfo*>&
TariffCrossRefDAO::getByAddonTariff(DeleteList& del,
                                    const VendorCode& vendor,
                                    const CarrierCode& carrier,
                                    const RecordScope& crossRefType,
                                    const TariffNumber& addonTariff,
                                    const DateTime& date,
                                    const DateTime& ticketDate)
{
  VendorCode tcrVendor = getTCRVendorData(vendor, del, ticketDate, false);
  TariffCrossRefKey key(tcrVendor, carrier, crossRefType);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<TariffCrossRefInfo*>* ret = nullptr;
  // std::vector<TariffCrossRefInfo*>* ret = new std::vector<TariffCrossRefInfo*>;
  // del.adopt(ret);
  // remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret),
  // not1(matchAddonTariff(addonTariff, date, ticketDate)));
  std::vector<TariffCrossRefInfo*>* tariffs = ptr->addonTariffs(addonTariff);
  if (tariffs != nullptr)
  {
    // remove_copy_if(tariffs->begin(), tariffs->end(), back_inserter(*ret),
    // IsNotEffectiveG<TariffCrossRefInfo>(date, ticketDate));
    ret = tariffs;
  }
  else
  {
    ret = new std::vector<TariffCrossRefInfo*>;
    del.adopt(ret);
  }
  return *ret;
}

TariffCrossRefInfoContainer*
TariffCrossRefDAO::create(TariffCrossRefKey key)
{
  TariffCrossRefInfoContainer* ret = new TariffCrossRefInfoContainer;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  std::string crossRefType = (key._c == DOMESTIC ? "D" : "I");
  try
  {
    QueryGetTrfXRefByCxr txr(dbAdapter->getAdapter());
    txr.findTariffXRefByCarrier(ret->tariffInfo(), key._b, key._a, crossRefType);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TariffCrossRefDAO::create");
    delete ret;
    throw;
  }
  ret->initializeTariffMaps();

  return ret;
}

void
TariffCrossRefDAO::destroy(TariffCrossRefKey key, TariffCrossRefInfoContainer* recs)
{
  // std::vector<TariffCrossRefInfo*>::iterator i;
  // for (i = recs->begin(); i != recs->end(); i++) delete *i;
  //
  // if(recs->size() > 0)
  //{
  //    TariffCrossRefInfo* array = recs->front();
  //    delete[] array;
  //}
  delete recs;
}

TariffCrossRefKey
TariffCrossRefDAO::createKey(TariffCrossRefInfoContainer* info)
{
  if (info == nullptr || info->tariffInfo().size() == 0 || info->tariffInfo().front() == nullptr)
  {
    LOG4CXX_ERROR(_logger, "SOMETHING IS TERRIBLY WRONG.... TariffCrossRefInfoContainer");
    return TariffCrossRefKey("NULL", "NL", DOMESTIC);
  }
  TariffCrossRefInfo* tcr = info->tariffInfo().front();
  return TariffCrossRefKey(tcr->vendor(), tcr->carrier(), tcr->crossRefType());
}

namespace
{
struct NoQuery
{
  NoQuery(tse::DBAdapter* dummy) {}
  void execute(TariffCrossRefInfoContainer& data) {}
};
}

void
TariffCrossRefDAO::load()
{
  TariffCrossRefInfoContainer data;
  Loader<NoQuery, TariffCrossRefDAO, TariffCrossRefInfoContainer> loader(data, false);
}

std::string
TariffCrossRefDAO::_name("TariffCrossRef");
std::string
TariffCrossRefDAO::_cacheClass("Common");

DAOHelper<TariffCrossRefDAO>
TariffCrossRefDAO::_helper(_name);

TariffCrossRefDAO* TariffCrossRefDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: TariffCrossRefHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
TariffCrossRefHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.TariffCrossRefHistoricalDAO"));
TariffCrossRefHistoricalDAO&
TariffCrossRefHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

struct TariffCrossRefHistoricalDAO::matchFareTariff
    : public std::unary_function<const TariffCrossRefInfo*, bool>
{
public:
  const TariffNumber& _fareTariff;
  const IsEffectiveH<TariffCrossRefInfo> _isEffective;

  matchFareTariff(const TariffNumber& fareTariff, const DateTime& date, const DateTime& ticketDate)
    : _fareTariff(fareTariff), _isEffective(date, ticketDate)
  {
  }

  bool operator()(const TariffCrossRefInfo* rec) const
  {
    return rec->_fareTariff == _fareTariff && _isEffective(rec);
  }
};

struct TariffCrossRefHistoricalDAO::matchRuleTariff
    : public std::unary_function<const TariffCrossRefInfo*, bool>
{
public:
  const TariffNumber& _ruleTariff;
  const IsEffectiveH<TariffCrossRefInfo> _isEffective;

  matchRuleTariff(const TariffNumber& ruleTariff, const DateTime& date, const DateTime& ticketDate)
    : _ruleTariff(ruleTariff), _isEffective(date, ticketDate)
  {
  }

  bool operator()(const TariffCrossRefInfo* rec) const
  {
    return rec->_ruleTariff == _ruleTariff && _isEffective(rec);
  }
};

struct TariffCrossRefHistoricalDAO::matchGenRuleTariff
    : public std::unary_function<const TariffCrossRefInfo*, bool>
{
public:
  const TariffNumber& _ruleTariff;
  const IsEffectiveH<TariffCrossRefInfo> _isEffective;

  matchGenRuleTariff(const TariffNumber& ruleTariff,
                     const DateTime& date,
                     const DateTime& ticketDate)
    : _ruleTariff(ruleTariff), _isEffective(date, ticketDate)
  {
  }

  bool operator()(const TariffCrossRefInfo* rec) const
  {
    return rec->_governingTariff == _ruleTariff && _isEffective(rec);
  }
};

struct TariffCrossRefHistoricalDAO::matchAddonTariff
    : public std::unary_function<const TariffCrossRefInfo*, bool>
{
public:
  const TariffNumber& _addonTariff;
  const IsEffectiveH<TariffCrossRefInfo> _isEffective;

  matchAddonTariff(const TariffNumber& addonTariff,
                   const DateTime& date,
                   const DateTime& ticketDate)
    : _addonTariff(addonTariff), _isEffective(date, ticketDate)
  {
  }

  bool operator()(const TariffCrossRefInfo* rec) const
  {
    return ((rec->_addonTariff1 == _addonTariff || rec->_addonTariff2 == _addonTariff) &&
            _isEffective(rec));
  }
};

const std::vector<TariffCrossRefInfo*>&
TariffCrossRefHistoricalDAO::get(DeleteList& del,
                                 const VendorCode& vendor,
                                 const CarrierCode& carrier,
                                 const RecordScope& crossRefType,
                                 const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  VendorCode tcrVendor = getTCRVendorData(vendor, del, ticketDate, true);
  TariffCrossRefHistoricalKey key(tcrVendor, carrier, crossRefType);
  DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<TariffCrossRefInfo*>* ret = new std::vector<TariffCrossRefInfo*>;
  del.adopt(ret);
  // remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret),
  // IsNotCurrentH<TariffCrossRefInfo>(ticketDate));
  std::vector<TariffCrossRefInfo*>& tariffs = ptr->tariffInfo();
  remove_copy_if(tariffs.begin(),
                 tariffs.end(),
                 back_inserter(*ret),
                 IsNotCurrentH<TariffCrossRefInfo>(ticketDate));
  return *ret;
}

const std::vector<TariffCrossRefInfo*>&
TariffCrossRefHistoricalDAO::getByFareTariff(DeleteList& del,
                                             const VendorCode& vendor,
                                             const CarrierCode& carrier,
                                             const RecordScope& crossRefType,
                                             const TariffNumber& fareTariff,
                                             const DateTime& date,
                                             const DateTime& ticketDate)
{
  VendorCode tcrVendor = getTCRVendorData(vendor, del, ticketDate, true);
  TariffCrossRefHistoricalKey key(tcrVendor, carrier, crossRefType);
  DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<TariffCrossRefInfo*>* ret = new std::vector<TariffCrossRefInfo*>;
  del.adopt(ret);
  // remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), not1(matchFareTariff(fareTariff,
  // date, ticketDate)));
  std::vector<TariffCrossRefInfo*>* tariffs = ptr->fareTariffs(fareTariff);
  if (tariffs != nullptr)
    remove_copy_if(tariffs->begin(),
                   tariffs->end(),
                   back_inserter(*ret),
                   IsNotEffectiveH<TariffCrossRefInfo>(date, ticketDate));
  return *ret;
}

const std::vector<TariffCrossRefInfo*>&
TariffCrossRefHistoricalDAO::getByRuleTariff(DeleteList& del,
                                             const VendorCode& vendor,
                                             const CarrierCode& carrier,
                                             const RecordScope& crossRefType,
                                             const TariffNumber& ruleTariff,
                                             const DateTime& date,
                                             const DateTime& ticketDate)
{
  VendorCode tcrVendor = getTCRVendorData(vendor, del, ticketDate, true);
  TariffCrossRefHistoricalKey key(tcrVendor, carrier, crossRefType);
  DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<TariffCrossRefInfo*>* ret = new std::vector<TariffCrossRefInfo*>;
  del.adopt(ret);
  // remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), not1(matchRuleTariff(ruleTariff,
  // date, ticketDate)));
  std::vector<TariffCrossRefInfo*>* tariffs = ptr->ruleTariffs(ruleTariff);
  if (tariffs != nullptr)
    remove_copy_if(tariffs->begin(),
                   tariffs->end(),
                   back_inserter(*ret),
                   IsNotEffectiveH<TariffCrossRefInfo>(date, ticketDate));
  return *ret;
}

const std::vector<TariffCrossRefInfo*>&
TariffCrossRefHistoricalDAO::getByGenRuleTariff(DeleteList& del,
                                                const VendorCode& vendor,
                                                const CarrierCode& carrier,
                                                const RecordScope& crossRefType,
                                                const TariffNumber& ruleTariff,
                                                const DateTime& date,
                                                const DateTime& ticketDate)
{
  VendorCode tcrVendor = getTCRVendorData(vendor, del, ticketDate, true);
  TariffCrossRefHistoricalKey key(tcrVendor, carrier, crossRefType);
  DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<TariffCrossRefInfo*>* ret = new std::vector<TariffCrossRefInfo*>;
  del.adopt(ret);
  // remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret),
  // not1(matchGenRuleTariff(ruleTariff, date, ticketDate)));
  std::vector<TariffCrossRefInfo*>* tariffs = ptr->govRuleTariffs(ruleTariff);
  if (tariffs != nullptr)
    remove_copy_if(tariffs->begin(),
                   tariffs->end(),
                   back_inserter(*ret),
                   IsNotEffectiveH<TariffCrossRefInfo>(date, ticketDate));
  return *ret;
}

const std::vector<TariffCrossRefInfo*>&
TariffCrossRefHistoricalDAO::getByAddonTariff(DeleteList& del,
                                              const VendorCode& vendor,
                                              const CarrierCode& carrier,
                                              const RecordScope& crossRefType,
                                              const TariffNumber& addonTariff,
                                              const DateTime& date,
                                              const DateTime& ticketDate)
{
  VendorCode tcrVendor = getTCRVendorData(vendor, del, ticketDate, true);
  TariffCrossRefHistoricalKey key(tcrVendor, carrier, crossRefType);
  DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<TariffCrossRefInfo*>* ret = new std::vector<TariffCrossRefInfo*>;
  del.adopt(ret);
  // remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret),
  // not1(matchAddonTariff(addonTariff, date, ticketDate)));
  std::vector<TariffCrossRefInfo*>* tariffs = ptr->addonTariffs(addonTariff);
  if (tariffs != nullptr)
    remove_copy_if(tariffs->begin(),
                   tariffs->end(),
                   back_inserter(*ret),
                   IsNotEffectiveH<TariffCrossRefInfo>(date, ticketDate));
  return *ret;
}

TariffCrossRefInfoContainer*
TariffCrossRefHistoricalDAO::create(TariffCrossRefHistoricalKey key)
{
  TariffCrossRefInfoContainer* ret = new TariffCrossRefInfoContainer;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  std::string crossRefType = (key._c == DOMESTIC ? "D" : "I");
  try
  {
    QueryGetTrfXRefByCxrHistorical txr(dbAdapter->getAdapter());
    txr.findTariffXRefByCarrier(ret->tariffInfo(), key._b, key._a, crossRefType, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in TariffCrossRefHistoricalDAO::create");
    delete ret;
    throw;
  }
  ret->initializeTariffMaps();

  return ret;
}

void
TariffCrossRefHistoricalDAO::destroy(TariffCrossRefHistoricalKey key,
                                     TariffCrossRefInfoContainer* recs)
{
  // std::vector<TariffCrossRefInfo*>::iterator i;
  // for (i = recs->begin(); i != recs->end(); i++) delete *i;
  delete recs;
}

std::string
TariffCrossRefHistoricalDAO::_name("TariffCrossRefHistorical");
std::string
TariffCrossRefHistoricalDAO::_cacheClass("Common");
DAOHelper<TariffCrossRefHistoricalDAO>
TariffCrossRefHistoricalDAO::_helper(_name);
TariffCrossRefHistoricalDAO* TariffCrossRefHistoricalDAO::_instance = nullptr;

} // namespace tse
