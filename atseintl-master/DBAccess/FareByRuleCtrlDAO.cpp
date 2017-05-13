//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/FareByRuleCtrlDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DAOUtils.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DBHistoryServer.h"
#include "DBAccess/FareByRuleCtrlInfo.h"
#include "DBAccess/Queries/QueryGetNonCombCatCtrl.h"

namespace tse
{
log4cxx::LoggerPtr
FareByRuleCtrlDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FareByRuleCtrlDAO"));

FareByRuleCtrlDAO&
FareByRuleCtrlDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareByRuleCtrlInfo*>&
getFareByRuleCtrlData(const VendorCode& vendor,
                      const CarrierCode& carrier,
                      const TariffNumber& tariffNumber,
                      const RuleNumber& ruleNumber,
                      const DateTime& tvlDate,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical,
                      bool isFareDisplay)
{
  if (UNLIKELY(isHistorical))
  {
    FareByRuleCtrlHistoricalDAO& dao = FareByRuleCtrlHistoricalDAO::instance();

    const std::vector<FareByRuleCtrlInfo*>& ret =
        isFareDisplay
            ? dao.getForFD(
                  deleteList, vendor, carrier, tariffNumber, ruleNumber, tvlDate, ticketDate)
            : dao.get(deleteList, vendor, carrier, tariffNumber, ruleNumber, tvlDate, ticketDate);

    return ret;
  }
  else
  {
    FareByRuleCtrlDAO& dao = FareByRuleCtrlDAO::instance();

    const std::vector<FareByRuleCtrlInfo*>& ret =
        isFareDisplay
            ? dao.getForFD(
                  deleteList, vendor, carrier, tariffNumber, ruleNumber, tvlDate, ticketDate)
            : dao.get(deleteList, vendor, carrier, tariffNumber, ruleNumber, tvlDate, ticketDate);

    return ret;
  }
}

const std::vector<FareByRuleCtrlInfo*>&
getFareByRuleCtrlData(const VendorCode& vendor,
                      const CarrierCode& carrier,
                      const TariffNumber& tariffNumber,
                      const RuleNumber& ruleNumber)
{
  // currently only for non historical, can be extended when optimizing historical applications
  return FareByRuleCtrlDAO::instance().getAll(vendor, carrier, tariffNumber, ruleNumber);
}

const std::vector<FareByRuleCtrlInfo*>&
FareByRuleCtrlDAO::get(DeleteList& del,
                       const VendorCode& vendor,
                       const CarrierCode& carrier,
                       const TariffNumber& tariffNumber,
                       const RuleNumber& ruleNumber,
                       const DateTime& tvlDate,
                       const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareByRuleCtrlKey key(vendor, carrier, tariffNumber, ruleNumber);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<FareByRuleCtrlInfo*>* ret = new std::vector<FareByRuleCtrlInfo*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveG<FareByRuleCtrlInfo>(tvlDate, ticketDate));
  ret->erase(std::remove_if(ret->begin(), ret->end(), Inhibit<FareByRuleCtrlInfo>), ret->end());
  return *ret;
}

const std::vector<FareByRuleCtrlInfo*>&
FareByRuleCtrlDAO::getForFD(DeleteList& del,
                            const VendorCode& vendor,
                            const CarrierCode& carrier,
                            const TariffNumber& tariffNumber,
                            const RuleNumber& rule,
                            const DateTime& date,
                            const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetFDCallCount++;

  FareByRuleCtrlKey key(vendor, carrier, tariffNumber, rule);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<FareByRuleCtrlInfo*>* ret = new std::vector<FareByRuleCtrlInfo*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveG<FareByRuleCtrlInfo>(date, ticketDate));
  ret->erase(std::remove_if(ret->begin(), ret->end(), InhibitForFD<FareByRuleCtrlInfo>),
             ret->end());
  return *ret;
}

std::vector<FareByRuleCtrlInfo*>*
FareByRuleCtrlDAO::create(FareByRuleCtrlKey key)
{
  std::vector<FareByRuleCtrlInfo*>* ret = new std::vector<FareByRuleCtrlInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFBRCtrl fbrc(dbAdapter->getAdapter());
    fbrc.findFareByRuleCtrlInfo(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareByRuleCtrlDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FareByRuleCtrlDAO::destroy(FareByRuleCtrlKey key, std::vector<FareByRuleCtrlInfo*>* recs)
{
  std::vector<FareByRuleCtrlInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

FareByRuleCtrlKey
FareByRuleCtrlDAO::createKey(FareByRuleCtrlInfo* info)
{
  return FareByRuleCtrlKey(
      info->vendorCode(), info->carrierCode(), info->tariffNumber(), info->ruleNumber());
}

void
FareByRuleCtrlDAO::load()
{
  StartupLoaderNoDB<FareByRuleCtrlInfo, FareByRuleCtrlDAO>();
}

sfc::CompressedData*
FareByRuleCtrlDAO::compress(const std::vector<FareByRuleCtrlInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<FareByRuleCtrlInfo*>*
FareByRuleCtrlDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<FareByRuleCtrlInfo>(compressed);
}

std::string
FareByRuleCtrlDAO::_name("FareByRuleCtrl");
std::string
FareByRuleCtrlDAO::_cacheClass("Fares");
DAOHelper<FareByRuleCtrlDAO>
FareByRuleCtrlDAO::_helper(_name);
FareByRuleCtrlDAO* FareByRuleCtrlDAO::_instance = nullptr;

////////////////////////////////////////////
// Historical DAO Object
////////////////////////////////////////////
log4cxx::LoggerPtr
FareByRuleCtrlHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareByRuleCtrlHistoricalDAO"));
FareByRuleCtrlHistoricalDAO&
FareByRuleCtrlHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareByRuleCtrlInfo*>&
FareByRuleCtrlHistoricalDAO::get(DeleteList& del,
                                 const VendorCode& vendor,
                                 const CarrierCode& carrier,
                                 const TariffNumber& tariffNumber,
                                 const RuleNumber& rule,
                                 const DateTime& tvlDate,
                                 const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  FareByRuleCtrlHistoricalKey key(vendor, carrier, tariffNumber, rule);
  DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<FareByRuleCtrlInfo*>* ret = new std::vector<FareByRuleCtrlInfo*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveHist<FareByRuleCtrlInfo>(tvlDate, ticketDate));
  ret->erase(std::remove_if(ret->begin(), ret->end(), Inhibit<FareByRuleCtrlInfo>), ret->end());
  return *ret;
}

const std::vector<FareByRuleCtrlInfo*>&
FareByRuleCtrlHistoricalDAO::getForFD(DeleteList& del,
                                      const VendorCode& vendor,
                                      const CarrierCode& carrier,
                                      const TariffNumber& tariffNumber,
                                      const RuleNumber& rule,
                                      const DateTime& date,
                                      const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetFDCallCount++;

  FareByRuleCtrlHistoricalKey key(vendor, carrier, tariffNumber, rule);
  DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<FareByRuleCtrlInfo*>* ret = new std::vector<FareByRuleCtrlInfo*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveHist<FareByRuleCtrlInfo>(date, ticketDate));
  ret->erase(std::remove_if(ret->begin(), ret->end(), InhibitForFD<FareByRuleCtrlInfo>),
             ret->end());
  return *ret;
}

std::vector<FareByRuleCtrlInfo*>*
FareByRuleCtrlHistoricalDAO::create(FareByRuleCtrlHistoricalKey key)
{
  std::vector<FareByRuleCtrlInfo*>* ret = new std::vector<FareByRuleCtrlInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetFBRCtrlHistorical fbrc(dbAdapter->getAdapter());
    fbrc.findFareByRuleCtrlInfo(*ret, key._a, key._b, key._c, key._d, key._e, key._f);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareByRuleCtrlHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FareByRuleCtrlHistoricalDAO::destroy(FareByRuleCtrlHistoricalKey key,
                                     std::vector<FareByRuleCtrlInfo*>* recs)
{
  std::vector<FareByRuleCtrlInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

FareByRuleCtrlHistoricalKey
FareByRuleCtrlHistoricalDAO::createKey(FareByRuleCtrlInfo* info,
                                       const DateTime& startDate,
                                       const DateTime& endDate)
{
  return FareByRuleCtrlHistoricalKey(info->vendorCode(),
                                     info->carrierCode(),
                                     info->tariffNumber(),
                                     info->ruleNumber(),
                                     startDate,
                                     endDate);
}

void
FareByRuleCtrlHistoricalDAO::load()
{
  StartupLoaderNoDB<FareByRuleCtrlInfo, FareByRuleCtrlHistoricalDAO>();
}

sfc::CompressedData*
FareByRuleCtrlHistoricalDAO::compress(const std::vector<FareByRuleCtrlInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<FareByRuleCtrlInfo*>*
FareByRuleCtrlHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<FareByRuleCtrlInfo>(compressed);
}

std::string
FareByRuleCtrlHistoricalDAO::_name("FareByRuleCtrlHistorical");
std::string
FareByRuleCtrlHistoricalDAO::_cacheClass("Rules");
DAOHelper<FareByRuleCtrlHistoricalDAO>
FareByRuleCtrlHistoricalDAO::_helper(_name);
FareByRuleCtrlHistoricalDAO* FareByRuleCtrlHistoricalDAO::_instance = nullptr;

} // namespace tse
