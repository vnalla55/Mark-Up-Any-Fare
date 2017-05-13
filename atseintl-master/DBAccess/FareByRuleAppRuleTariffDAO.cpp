//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/FareByRuleAppRuleTariffDAO.h"

#include "Common/Logger.h"
#include "DBAccess/CorpId.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/Queries/QueryGetFareByRuleApp.h"

#include <boost/algorithm/string.hpp>

namespace tse
{

log4cxx::LoggerPtr
FareByRuleAppRuleTariffDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareByRuleAppRuleTariffDAO"));

struct FareByRuleAppRuleTariffDAO::FilterPaxType
{
public:
  FilterPaxType(const TktDesignator& tktDesignator,
                const std::vector<PaxTypeCode>& paxTypes,
                const std::vector<CorpId*>& corpIds,
                const DateTime& date,
                const DateTime& ticketDate,
                bool isFareDisplay)
    : _tktDesignator(tktDesignator),
      _paxTypes(paxTypes),
      _corpIds(corpIds),
      _isNotEffective(date, ticketDate),
      _isFareDisplay(isFareDisplay)
  {
  }

  bool operator()(const FareByRuleApp* rec)
  {
    if (_isFareDisplay)
    {
      if (InhibitForFD<FareByRuleApp>(rec))
        return true;
    }
    else
    {
      if (Inhibit<FareByRuleApp>(rec))
        return true;
    }

    if (_isNotEffective(rec))
      return true;

    if (!rec->tktDesignator().empty() && rec->tktDesignator() != _tktDesignator)
      return true;

    if (std::find(_paxTypes.begin(), _paxTypes.end(), rec->primePaxType()) == _paxTypes.end() &&
        std::find_first_of(_paxTypes.begin(),
                           _paxTypes.end(),
                           rec->secondaryPaxTypes().begin(),
                           rec->secondaryPaxTypes().end()) == _paxTypes.end())
      return true;

    std::vector<CorpId*>::const_iterator i = _corpIds.begin();
    for (; i != _corpIds.end(); ++i)
    {
      CorpId& corpId = **i;
      if (LIKELY(corpId.vendor() != rec->vendor()))
        continue;
      if (corpId.ruleTariff() != rec->ruleTariff())
        continue;
      if (!corpId.rule().empty() && corpId.rule() != rec->ruleNo())
        continue;
      return false;
    }

    return true;
  }

private:
  const TktDesignator& _tktDesignator;
  const std::vector<PaxTypeCode>& _paxTypes;
  const std::vector<CorpId*>& _corpIds;
  IsNotEffectiveG<FareByRuleApp> _isNotEffective;
  bool _isFareDisplay;
};

FareByRuleAppRuleTariffDAO&
FareByRuleAppRuleTariffDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareByRuleApp*>&
getFareByRuleAppRuleTariffData(const CarrierCode& carrier,
                               const TktDesignator& tktDesignator,
                               const DateTime& tvlDate,
                               std::vector<PaxTypeCode>& paxTypes,
                               const std::vector<CorpId*>& corpIds,
                               DeleteList& deleteList,
                               const DateTime& ticketDate,
                               bool isHistorical,
                               bool isFareDisplay)
{
  if (isHistorical)
  {
    FareByRuleAppRuleTariffHistoricalDAO& dao = FareByRuleAppRuleTariffHistoricalDAO::instance();
    return dao.get(
        deleteList, carrier, tktDesignator, paxTypes, corpIds, tvlDate, ticketDate, isFareDisplay);
  }
  else
  {
    FareByRuleAppRuleTariffDAO& dao = FareByRuleAppRuleTariffDAO::instance();
    return dao.get(
        deleteList, carrier, tktDesignator, paxTypes, corpIds, tvlDate, ticketDate, isFareDisplay);
  }
}

const std::vector<FareByRuleApp*>&
FareByRuleAppRuleTariffDAO::get(DeleteList& del,
                                const CarrierCode& carrier,
                                const TktDesignator& tktDesignator,
                                const std::vector<PaxTypeCode>& paxTypes,
                                const std::vector<CorpId*>& corpIds,
                                const DateTime& tvlDate,
                                const DateTime& ticketDate,
                                bool isFareDisplay)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  std::vector<FareByRuleApp*>* ret = new std::vector<FareByRuleApp*>;
  del.adopt(ret);

  FilterPaxType filter(tktDesignator, paxTypes, corpIds, tvlDate, ticketDate, isFareDisplay);

  TariffNumber ruleTariff = -1;
  std::vector<CorpId*>::const_iterator i = corpIds.begin();
  for (; i != corpIds.end(); ++i)
  {
    if (ruleTariff == (*i)->ruleTariff())
      continue;
    ruleTariff = (*i)->ruleTariff();

    FareByRuleAppRuleTariffKey key(carrier, ruleTariff);
    DAOCache::pointer_type ptr = cache().get(key);
    del.copy(ptr);
    remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), filter);
    if (carrier != "YY")
    {
      FareByRuleAppRuleTariffKey key2("YY", ruleTariff);
      ptr = cache().get(key2);
      del.copy(ptr);
      remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), filter);
    }
  }
  return *ret;
}

std::vector<FareByRuleApp*>*
FareByRuleAppRuleTariffDAO::create(FareByRuleAppRuleTariffKey key)
{
  std::vector<FareByRuleApp*>* ret = new std::vector<FareByRuleApp*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareByRuleAppRuleTariff fbr(dbAdapter->getAdapter());
    fbr.findFareByRuleApp(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareByRuleAppRuleTariffDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FareByRuleAppRuleTariffDAO::destroy(FareByRuleAppRuleTariffKey key,
                                    std::vector<FareByRuleApp*>* recs)
{
  destroyContainer(recs);
}

std::string
FareByRuleAppRuleTariffDAO::_name("FareByRuleAppRuleTariff");
std::string
FareByRuleAppRuleTariffDAO::_cacheClass("Fares");
DAOHelper<FareByRuleAppRuleTariffDAO>
FareByRuleAppRuleTariffDAO::_helper(_name);
FareByRuleAppRuleTariffDAO* FareByRuleAppRuleTariffDAO::_instance = nullptr;

FareByRuleAppRuleTariffKey
FareByRuleAppRuleTariffDAO::createKey(const FareByRuleApp* info)
{
  return FareByRuleAppRuleTariffKey(info->carrier(), info->ruleTariff());
}

void
FareByRuleAppRuleTariffDAO::load()
{
  StartupLoaderNoDB<FareByRuleApp, FareByRuleAppRuleTariffDAO>();
}

size_t FareByRuleAppRuleTariffDAO::invalidate(const ObjectKey& objectKey)
{
  CarrierCode carrierNotification;
  objectKey.getValue("CARRIER", carrierNotification);
/*////test
FareByRuleAppRuleTariffKey dummyKey(carrierNotification, 1);
std::vector<FareByRuleApp*>* dummyValue(new std::vector<FareByRuleApp*>);
this->cache().put(dummyKey, dummyValue);
*///////
  std::string tariffStr;
  objectKey.getValue("RULETARIFF", tariffStr);
  boost::trim(tariffStr);
  if (!tariffStr.empty())
  {
    return DataAccessObject<FareByRuleAppRuleTariffKey, std::vector<FareByRuleApp*>>::invalidate(objectKey);
  }
  size_t result(0);
  std::shared_ptr<std::vector<FareByRuleAppRuleTariffKey>> cacheKeys(cache().keys());
  if (cacheKeys)
  {
    for (auto& elem : *cacheKeys)
    {
      if (elem._a == carrierNotification)
      {
        result +=
            DataAccessObject<FareByRuleAppRuleTariffKey, std::vector<FareByRuleApp*>>::invalidate(
                elem);
      }
    }
  }
  return result;
}

sfc::CompressedData*
FareByRuleAppRuleTariffDAO::compress(const std::vector<FareByRuleApp*>* vect) const
{
  return compressVector(vect);
}

std::vector<FareByRuleApp*>*
FareByRuleAppRuleTariffDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<FareByRuleApp>(compressed);
}

///////////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////////
log4cxx::LoggerPtr
FareByRuleAppRuleTariffHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareByRuleAppRuleTariffHistoricalDAO"));

struct FareByRuleAppRuleTariffHistoricalDAO::FilterPaxType
{
public:
  FilterPaxType(const TktDesignator& tktDesignator,
                const std::vector<PaxTypeCode>& paxTypes,
                const std::vector<CorpId*>& corpIds,
                const DateTime& date,
                const DateTime& ticketDate,
                bool isFareDisplay)
    : _tktDesignator(tktDesignator),
      _paxTypes(paxTypes),
      _corpIds(corpIds),
      _isNotEffective(date, ticketDate),
      _isFareDisplay(isFareDisplay)
  {
  }

  FilterPaxType(const TktDesignator& tktDesignator,
                const std::vector<PaxTypeCode>& paxTypes,
                const std::vector<CorpId*>& corpIds,
                const DateTime& startDate,
                const DateTime& endDate,
                const DateTime& ticketDate,
                bool isFareDisplay)
    : _tktDesignator(tktDesignator),
      _paxTypes(paxTypes),
      _corpIds(corpIds),
      _isNotEffective(startDate, endDate, ticketDate),
      _isFareDisplay(isFareDisplay)
  {
  }

  bool operator()(const FareByRuleApp* rec)
  {
    if (_isFareDisplay)
    {
      if (InhibitForFD<FareByRuleApp>(rec))
        return true;
    }
    else
    {
      if (Inhibit<FareByRuleApp>(rec))
        return true;
    }

    if (_isNotEffective(rec))
      return true;

    if (!rec->tktDesignator().empty() && rec->tktDesignator() != _tktDesignator)
      return true;

    if (std::find(_paxTypes.begin(), _paxTypes.end(), rec->primePaxType()) == _paxTypes.end() &&
        std::find_first_of(_paxTypes.begin(),
                           _paxTypes.end(),
                           rec->secondaryPaxTypes().begin(),
                           rec->secondaryPaxTypes().end()) == _paxTypes.end())
      return true;

    std::vector<CorpId*>::const_iterator i = _corpIds.begin();
    for (; i != _corpIds.end(); ++i)
    {
      CorpId& corpId = **i;
      if (corpId.vendor() != rec->vendor())
        continue;
      if (corpId.ruleTariff() != rec->ruleTariff())
        continue;
      if (!corpId.rule().empty() && corpId.rule() != rec->ruleNo())
        continue;
      return false;
    }

    return true;
  }

private:
  const TktDesignator& _tktDesignator;
  const std::vector<PaxTypeCode>& _paxTypes;
  const std::vector<CorpId*>& _corpIds;
  IsNotEffectiveH<FareByRuleApp> _isNotEffective;
  bool _isFareDisplay;
};

FareByRuleAppRuleTariffHistoricalDAO&
FareByRuleAppRuleTariffHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareByRuleApp*>&
FareByRuleAppRuleTariffHistoricalDAO::get(DeleteList& del,
                                          const CarrierCode& carrier,
                                          const TktDesignator& tktDesignator,
                                          const std::vector<PaxTypeCode>& paxTypes,
                                          const std::vector<CorpId*>& corpIds,
                                          const DateTime& tvlDate,
                                          const DateTime& ticketDate,
                                          bool isFareDisplay)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  std::vector<FareByRuleApp*>* ret = new std::vector<FareByRuleApp*>;
  del.adopt(ret);

  FilterPaxType filter(tktDesignator, paxTypes, corpIds, tvlDate, ticketDate, isFareDisplay);

  TariffNumber ruleTariff = -1;
  std::vector<CorpId*>::const_iterator i = corpIds.begin();
  for (; i != corpIds.end(); ++i)
  {
    if (ruleTariff == (*i)->ruleTariff())
      continue;
    ruleTariff = (*i)->ruleTariff();

    FareByRuleAppRuleTariffHistoricalKey key(carrier, ruleTariff);
    DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
    DAOCache::pointer_type ptr = cache().get(key);
    del.copy(ptr);
    remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret), filter);
  }
  return *ret;
}

std::vector<FareByRuleApp*>*
FareByRuleAppRuleTariffHistoricalDAO::create(FareByRuleAppRuleTariffHistoricalKey key)
{
  std::vector<FareByRuleApp*>* ret = new std::vector<FareByRuleApp*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetFareByRuleAppRuleTariffHistorical fbrart(dbAdapter->getAdapter());
    fbrart.findFareByRuleApp(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareByRuleAppRuleTariffHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FareByRuleAppRuleTariffHistoricalDAO::destroy(FareByRuleAppRuleTariffHistoricalKey key,
                                              std::vector<FareByRuleApp*>* recs)
{
  destroyContainer(recs);
}

size_t FareByRuleAppRuleTariffHistoricalDAO::invalidate(const ObjectKey& objectKey)
{
  CarrierCode carrierNotification;
  objectKey.getValue("CARRIER", carrierNotification);
/*///test
DateTime startDate;
DateTime endDate;
FareByRuleAppRuleTariffHistoricalKey dummyKey(carrierNotification, 1, startDate, endDate);
std::vector<FareByRuleApp*>* dummyValue(new std::vector<FareByRuleApp*>);
this->cache().put(dummyKey, dummyValue);
*///////
  std::string tariffStr;
  objectKey.getValue("RULETARIFF", tariffStr);
  boost::trim(tariffStr);
  if (!tariffStr.empty())
  {
    return HistoricalDataAccessObject<FareByRuleAppRuleTariffHistoricalKey,
                                      std::vector<FareByRuleApp*>>::invalidate(objectKey);
  }
  size_t result(0);
  std::shared_ptr<std::vector<FareByRuleAppRuleTariffHistoricalKey>> cacheKeys(cache().keys());
  if (cacheKeys)
  {
    for (auto& elem : *cacheKeys)
    {
      if (elem._a == carrierNotification)
      {
        result += HistoricalDataAccessObject<FareByRuleAppRuleTariffHistoricalKey,
                                             std::vector<FareByRuleApp*>>::invalidate(elem);
      }
    }
  }
  return result;
}

sfc::CompressedData*
FareByRuleAppRuleTariffHistoricalDAO::compress(const std::vector<FareByRuleApp*>* vect) const
{
  return compressVector(vect);
}

std::vector<FareByRuleApp*>*
FareByRuleAppRuleTariffHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<FareByRuleApp>(compressed);
}

std::string
FareByRuleAppRuleTariffHistoricalDAO::_name("FareByRuleAppRuleTariffHistorical");
std::string
FareByRuleAppRuleTariffHistoricalDAO::_cacheClass("Fares");
DAOHelper<FareByRuleAppRuleTariffHistoricalDAO>
FareByRuleAppRuleTariffHistoricalDAO::_helper(_name);
FareByRuleAppRuleTariffHistoricalDAO* FareByRuleAppRuleTariffHistoricalDAO::_instance = nullptr;

} // namespace tse
