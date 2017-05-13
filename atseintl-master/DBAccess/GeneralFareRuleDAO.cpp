//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/GeneralFareRuleDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DBHistoryServer.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/Queries/QueryGetNonCombCatCtrl.h"

namespace tse
{
namespace
{
void
init(std::vector<GeneralFareRuleInfo*>& vect)
{
  typedef std::vector<GeneralFareRuleInfo*>::const_iterator It;
  for (It it = vect.begin(); it != vect.end(); ++it)
  {
    (*it)->init();
  }
}

} // namespace

log4cxx::LoggerPtr
GeneralFareRuleDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.GeneralFareRuleDAO"));

GeneralFareRuleDAO&
GeneralFareRuleDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<GeneralFareRuleInfo*>&
getGeneralFareRuleData(const VendorCode vendor,
                       const CarrierCode carrier,
                       const TariffNumber ruleTariff,
                       const RuleNumber rule,
                       const CatNumber category)
{
  // currently only for non historical, can be extended when optimizing historical applications
  return GeneralFareRuleDAO::instance().getAll(vendor, carrier, ruleTariff, rule, category);
}

const std::vector<GeneralFareRuleInfo*>&
getGeneralFareRuleData(const VendorCode& vendor,
                       const CarrierCode& carrier,
                       const TariffNumber& ruleTariff,
                       const RuleNumber& rule,
                       const CatNumber& category,
                       const DateTime& date,
                       DeleteList& deleteList,
                       const DateTime& ticketDate,
                       bool isHistorical,
                       bool isFareDisplay)
{
  if (UNLIKELY(isHistorical))
  {
    GeneralFareRuleHistoricalDAO& dao = GeneralFareRuleHistoricalDAO::instance();

    const std::vector<GeneralFareRuleInfo*>& ret =
        (isFareDisplay
             ? dao.getForFD(
                   deleteList, vendor, carrier, ruleTariff, rule, category, date, ticketDate)
             : dao.get(deleteList, vendor, carrier, ruleTariff, rule, category, date, ticketDate));

    return ret;
  }
  else
  {
    GeneralFareRuleDAO& dao = GeneralFareRuleDAO::instance();

    const std::vector<GeneralFareRuleInfo*>& ret =
        (isFareDisplay
             ? dao.getForFD(
                   deleteList, vendor, carrier, ruleTariff, rule, category, date, ticketDate)
             : dao.get(deleteList, vendor, carrier, ruleTariff, rule, category, date, ticketDate));

    return ret;
  }
}

const std::vector<GeneralFareRuleInfo*>&
GeneralFareRuleDAO::get(DeleteList& del,
                        const VendorCode& vendor,
                        const CarrierCode& carrier,
                        const TariffNumber& ruleTariff,
                        const RuleNumber& rule,
                        const CatNumber& category,
                        const DateTime& date,
                        const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  GeneralFareRuleKey key(vendor, carrier, ruleTariff, rule, category);
  DAOCache::pointer_type ptr = getFromCache(key);
  return *applyFilter(del, ptr, IsNotEffectiveG<GeneralFareRuleInfo>(date, ticketDate));
}

const std::vector<GeneralFareRuleInfo*>&
GeneralFareRuleDAO::getForFD(DeleteList& del,
                             const VendorCode& vendor,
                             const CarrierCode& carrier,
                             const TariffNumber& ruleTariff,
                             const RuleNumber& rule,
                             const CatNumber& category,
                             const DateTime& date,
                             const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetFDCallCount;

  GeneralFareRuleKey key(vendor, carrier, ruleTariff, rule, category);
  DAOCache::pointer_type ptr = getFromCache(key);
  std::vector<GeneralFareRuleInfo*>* ret = new std::vector<GeneralFareRuleInfo*>;
  del.adopt(ret);

  if (ptr.get() != nullptr)
  {
    del.copy(ptr);
    remove_copy_if(ptr->begin(),
                   ptr->end(),
                   back_inserter(*ret),
                   IsNotEffectiveG<GeneralFareRuleInfo>(date, ticketDate));
  }

  ret->erase(std::remove_if(ret->begin(), ret->end(), InhibitForFD<GeneralFareRuleInfo>),
             ret->end());
  return *ret;
}

GeneralFareRuleKey
GeneralFareRuleDAO::createKey(GeneralFareRuleInfo* info)
{
  return GeneralFareRuleKey(info->vendorCode(),
                            info->carrierCode(),
                            info->tariffNumber(),
                            info->ruleNumber(),
                            info->categoryNumber());
}

std::vector<GeneralFareRuleInfo*>*
GeneralFareRuleDAO::create(GeneralFareRuleKey key)
{
  std::vector<GeneralFareRuleInfo*>* ret = new std::vector<GeneralFareRuleInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetNonCombCatCtrl fncc(dbAdapter->getAdapter());
    fncc.findGeneralFareRule(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in GeneralFareRuleDAO::create");
    destroyContainer(ret);
    throw;
  }
  init(*ret);
  return ret;
}

void
GeneralFareRuleDAO::destroy(GeneralFareRuleKey key, std::vector<GeneralFareRuleInfo*>* recs)
{
  destroyContainer(recs);
}

void
GeneralFareRuleDAO::load()
{
  StartupLoader<QueryGetAllNonCombCatCtrl, GeneralFareRuleInfo, GeneralFareRuleDAO>();
  _loadedOnStartup = true;
}

size_t
GeneralFareRuleDAO::clear()
{
  size_t result(cache().clear());

  // if we clear the cache, then loading we did on startup
  // is removed.
  _loadedOnStartup = false;

  LOG4CXX_ERROR(_logger, "GeneralFareRule cache cleared");
  return result;
}

sfc::CompressedData*
GeneralFareRuleDAO::compress(const std::vector<GeneralFareRuleInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<GeneralFareRuleInfo*>*
GeneralFareRuleDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<GeneralFareRuleInfo>(compressed);
}

std::string
GeneralFareRuleDAO::_name("GeneralFareRule");
std::string
GeneralFareRuleDAO::_cacheClass("Rules");
DAOHelper<GeneralFareRuleDAO>
GeneralFareRuleDAO::_helper(_name);
GeneralFareRuleDAO* GeneralFareRuleDAO::_instance = nullptr;

///////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////

log4cxx::LoggerPtr
GeneralFareRuleHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.GeneralFareRuleHistoricalDAO"));

GeneralFareRuleHistoricalDAO&
GeneralFareRuleHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<GeneralFareRuleInfo*>&
GeneralFareRuleHistoricalDAO::get(DeleteList& del,
                                  const VendorCode& vendor,
                                  const CarrierCode& carrier,
                                  const TariffNumber& ruleTariff,
                                  const RuleNumber& rule,
                                  const CatNumber& category,
                                  const DateTime& date,
                                  const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  GeneralFareRuleHistoricalKey key(vendor, carrier, ruleTariff, rule, category);
  DAOUtils::getDateRange(ticketDate, key._f, key._g, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<GeneralFareRuleInfo*>* ret = new std::vector<GeneralFareRuleInfo*>;
  del.adopt(ret);

  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveH<GeneralFareRuleInfo>(date, ticketDate));
  return *ret;
}

const std::vector<GeneralFareRuleInfo*>&
GeneralFareRuleHistoricalDAO::getForFD(DeleteList& del,
                                       const VendorCode& vendor,
                                       const CarrierCode& carrier,
                                       const TariffNumber& ruleTariff,
                                       const RuleNumber& rule,
                                       const CatNumber& category,
                                       const DateTime& date,
                                       const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetFDCallCount;

  GeneralFareRuleHistoricalKey key(vendor, carrier, ruleTariff, rule, category);
  DAOUtils::getDateRange(ticketDate, key._f, key._g, _cacheBy);
  DAOCache::pointer_type ptr = getFromCache(key);
  std::vector<GeneralFareRuleInfo*>* ret = new std::vector<GeneralFareRuleInfo*>;
  del.adopt(ret);

  if (ptr.get() != nullptr)
  {
    del.copy(ptr);
    remove_copy_if(ptr->begin(),
                   ptr->end(),
                   back_inserter(*ret),
                   IsNotEffectiveH<GeneralFareRuleInfo>(date, ticketDate));
  }

  ret->erase(std::remove_if(ret->begin(), ret->end(), InhibitForFD<GeneralFareRuleInfo>),
             ret->end());
  return *ret;
}

std::vector<GeneralFareRuleInfo*>*
GeneralFareRuleHistoricalDAO::create(GeneralFareRuleHistoricalKey key)
{
  std::vector<GeneralFareRuleInfo*>* ret = new std::vector<GeneralFareRuleInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetNonCombCatCtrlHistorical ncc(dbAdapter->getAdapter());
    ncc.findGeneralFareRule(*ret, key._a, key._b, key._c, key._d, key._e, key._f, key._g);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in GeneralFareRuleHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  init(*ret);
  return ret;
}

GeneralFareRuleHistoricalKey
GeneralFareRuleHistoricalDAO::createKey(const GeneralFareRuleInfo* info,
                                        const DateTime& startDate,
                                        const DateTime& endDate)
{
  return GeneralFareRuleHistoricalKey(info->vendorCode(),
                                      info->carrierCode(),
                                      info->tariffNumber(),
                                      info->ruleNumber(),
                                      info->categoryNumber(),
                                      startDate,
                                      endDate);
}

void
GeneralFareRuleHistoricalDAO::destroy(GeneralFareRuleHistoricalKey key,
                                      std::vector<GeneralFareRuleInfo*>* recs)
{
  if (recs)
  {
    for (GeneralFareRuleInfo* gfri : *recs)
      delete gfri;
    delete recs;
  }
}

void
GeneralFareRuleHistoricalDAO::load()
{
  StartupLoaderNoDB<GeneralFareRuleInfo, GeneralFareRuleHistoricalDAO>();
  _loadedOnStartup = true;
}

size_t
GeneralFareRuleHistoricalDAO::clear()
{
  size_t result(cache().clear());

  // if we clear the cache, then loading we did on startup
  // is removed.
  _loadedOnStartup = false;

  LOG4CXX_ERROR(_logger, "GeneralFareRuleHistorical cache cleared");
  return result;
}

sfc::CompressedData*
GeneralFareRuleHistoricalDAO::compress(const std::vector<GeneralFareRuleInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<GeneralFareRuleInfo*>*
GeneralFareRuleHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<GeneralFareRuleInfo>(compressed);
}

std::string
GeneralFareRuleHistoricalDAO::_name("GeneralFareRuleHistorical");
std::string
GeneralFareRuleHistoricalDAO::_cacheClass("Rules");
DAOHelper<GeneralFareRuleHistoricalDAO>
GeneralFareRuleHistoricalDAO::_helper(_name);
GeneralFareRuleHistoricalDAO* GeneralFareRuleHistoricalDAO::_instance = nullptr;

} // namespace tse
