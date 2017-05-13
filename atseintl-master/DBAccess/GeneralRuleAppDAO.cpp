//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/GeneralRuleAppDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/GeneralRuleApp.h"
#include "DBAccess/Queries/QueryGetGeneralRule.h"

namespace tse
{
log4cxx::LoggerPtr
GeneralRuleAppDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.GeneralRuleAppDAO"));

GeneralRuleAppDAO&
GeneralRuleAppDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<GeneralRuleApp*>&
getGeneralRuleAppByTvlDateData(const VendorCode& vendor,
                               const CarrierCode& carrier,
                               const TariffNumber& tariffNumber,
                               const RuleNumber& ruleNumber,
                               DeleteList& deleteList,
                               const DateTime& ticketDate,
                               bool isHistorical,
                               DateTime tvlDate)
{
  if (isHistorical)
  {
    GeneralRuleAppHistoricalDAO& dao = GeneralRuleAppHistoricalDAO::instance();
    return dao.getByTvlDate(deleteList, vendor, carrier, tariffNumber, ruleNumber, ticketDate, tvlDate);
  }
  else
  {
    GeneralRuleAppDAO& dao = GeneralRuleAppDAO::instance();
    return dao.getByTvlDate(deleteList, vendor, carrier, tariffNumber, ruleNumber, ticketDate, tvlDate);
  }
}

const std::vector<GeneralRuleApp*>&
GeneralRuleAppDAO::get(DeleteList& del,
                       const VendorCode& vendor,
                       const CarrierCode& carrier,
                       const TariffNumber& tariffNumber,
                       const RuleNumber& ruleNumber,
                       const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  GeneralRuleAppKey key(vendor, carrier, tariffNumber, ruleNumber);
  DAOCache::pointer_type ptr = cache().get(key);

  return *(applyFilter(del, ptr, IsNotCurrentG<GeneralRuleApp>(ticketDate)));
}

const std::vector<GeneralRuleApp*>&
GeneralRuleAppDAO::getByTvlDate(DeleteList& del,
                       const VendorCode& vendor,
                       const CarrierCode& carrier,
                       const TariffNumber& tariffNumber,
                       const RuleNumber& ruleNumber,
                       const DateTime& ticketDate,
                       DateTime tvlDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  GeneralRuleAppKey key(vendor, carrier, tariffNumber, ruleNumber);
  DAOCache::pointer_type ptr = cache().get(key);

  return *(applyFilter(del, ptr, IsNotEffectiveG<GeneralRuleApp>(tvlDate, ticketDate)));
}

GeneralRuleApp*
getGeneralRuleAppData(const VendorCode& vendor,
                      const CarrierCode& carrier,
                      const TariffNumber& tariffNumber,
                      const RuleNumber& ruleNumber,
                      CatNumber catNum,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    GeneralRuleAppHistoricalDAO& dao = GeneralRuleAppHistoricalDAO::instance();
    return dao.get(deleteList, vendor, carrier, tariffNumber, ruleNumber, ticketDate, catNum);
  }
  else
  {
    GeneralRuleAppDAO& dao = GeneralRuleAppDAO::instance();
    return dao.get(deleteList, vendor, carrier, tariffNumber, ruleNumber, ticketDate, catNum);
  }
}

GeneralRuleApp*
getGeneralRuleAppByTvlDateData(const VendorCode& vendor,
                               const CarrierCode& carrier,
                               const TariffNumber& tariffNumber,
                               const RuleNumber& ruleNumber,
                               CatNumber catNum,
                               DeleteList& deleteList,
                               const DateTime& ticketDate,
                               bool isHistorical,
                               DateTime tvlDate)
{
  if (isHistorical)
  {
    GeneralRuleAppHistoricalDAO& dao = GeneralRuleAppHistoricalDAO::instance();
    return dao.getByTvlDate(deleteList, vendor, carrier, tariffNumber, ruleNumber, ticketDate, catNum, tvlDate);
  }
  else
  {
    GeneralRuleAppDAO& dao = GeneralRuleAppDAO::instance();
    return dao.getByTvlDate(deleteList, vendor, carrier, tariffNumber, ruleNumber, ticketDate, catNum, tvlDate);
  }
}

GeneralRuleApp*
GeneralRuleAppDAO::get(DeleteList& del,
                       const VendorCode& vendor,
                       const CarrierCode& carrier,
                       const TariffNumber& tariffNumber,
                       const RuleNumber& ruleNumber,
                       const DateTime& ticketDate,
                       CatNumber catNum)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  GeneralRuleAppKey key(vendor, carrier, tariffNumber, ruleNumber);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  IsNotCurrentG<GeneralRuleApp> notCurrent(ticketDate);
  std::vector<GeneralRuleApp*>::const_iterator i1 = ptr->begin();
  std::vector<GeneralRuleApp*>::const_iterator i2 = ptr->end();
  for (; i1 != i2; ++i1)
  {
    if (!notCurrent(*i1) && (*i1)->category() == catNum)
    {
      return *i1;
    }
  }

  return nullptr;
}

GeneralRuleApp*
GeneralRuleAppDAO::getByTvlDate(DeleteList& del,
                       const VendorCode& vendor,
                       const CarrierCode& carrier,
                       const TariffNumber& tariffNumber,
                       const RuleNumber& ruleNumber,
                       const DateTime& ticketDate,
                       CatNumber catNum,
                       DateTime tvlDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  GeneralRuleAppKey key(vendor, carrier, tariffNumber, ruleNumber);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  IsNotEffectiveG<GeneralRuleApp> notCurrent(tvlDate);
  std::vector<GeneralRuleApp*>::const_iterator i1 = ptr->begin();
  std::vector<GeneralRuleApp*>::const_iterator i2 = ptr->end();
  for (; i1 != i2; ++i1)
  {
    if (!notCurrent(*i1) && (*i1)->category() == catNum)
    {
      return *i1;
    }
  }

  return nullptr;
}

bool
getGeneralRuleAppTariffRuleData(const VendorCode& vendor,
                                const CarrierCode& carrier,
                                const TariffNumber& tariffNumber,
                                const RuleNumber& ruleNumber,
                                CatNumber catNum,
                                RuleNumber& ruleNumOut,
                                TariffNumber& tariffNumOut,
                                DeleteList& deleteList,
                                const DateTime& ticketDate,
                                bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    GeneralRuleAppHistoricalDAO& dao = GeneralRuleAppHistoricalDAO::instance();
    return dao.getTariffRule(deleteList,
                             vendor,
                             carrier,
                             tariffNumber,
                             ruleNumber,
                             ticketDate,
                             catNum,
                             ruleNumOut,
                             tariffNumOut);
  }
  else
  {
    GeneralRuleAppDAO& dao = GeneralRuleAppDAO::instance();
    return dao.getTariffRule(deleteList,
                             vendor,
                             carrier,
                             tariffNumber,
                             ruleNumber,
                             ticketDate,
                             catNum,
                             ruleNumOut,
                             tariffNumOut);
  }
}

bool
getGeneralRuleAppTariffRuleByTvlDateData(const VendorCode& vendor,
                                         const CarrierCode& carrier,
                                         const TariffNumber& tariffNumber,
                                         const RuleNumber& ruleNumber,
                                         CatNumber catNum,
                                         RuleNumber& ruleNumOut,
                                         TariffNumber& tariffNumOut,
                                         DeleteList& deleteList,
                                         const DateTime& ticketDate,
                                         bool isHistorical,
                                         DateTime tvlDate)
{
  if (isHistorical)
  {
    GeneralRuleAppHistoricalDAO& dao = GeneralRuleAppHistoricalDAO::instance();
    return dao.getTariffRuleByTvlDate(deleteList,
                                      vendor,
                                      carrier,
                                      tariffNumber,
                                      ruleNumber,
                                      ticketDate,
                                      catNum,
                                      ruleNumOut,
                                      tariffNumOut,
                                      tvlDate);
  }
  else
  {
    GeneralRuleAppDAO& dao = GeneralRuleAppDAO::instance();
    return dao.getTariffRuleByTvlDate(deleteList,
                                      vendor,
                                      carrier,
                                      tariffNumber,
                                      ruleNumber,
                                      ticketDate,
                                      catNum,
                                      ruleNumOut,
                                      tariffNumOut,
                                      tvlDate);
  }
}

bool
GeneralRuleAppDAO::getTariffRule(DeleteList& del,
                                 const VendorCode& vendor,
                                 const CarrierCode& carrier,
                                 const TariffNumber& tariffNumber,
                                 const RuleNumber& ruleNumber,
                                 const DateTime& ticketDate,
                                 CatNumber catNum,
                                 RuleNumber& ruleNumOut,
                                 TariffNumber& tariffNumOut)
{
  GeneralRuleAppKey key(vendor, carrier, tariffNumber, ruleNumber);
  DAOCache::pointer_type ptr = cache().get(key);

  IsNotCurrentG<GeneralRuleApp> notCurrent(ticketDate);
  std::vector<GeneralRuleApp*>::const_iterator i1 = ptr->begin();
  std::vector<GeneralRuleApp*>::const_iterator i2 = ptr->end();
  for (; i1 != i2; ++i1)
  {
    if (!notCurrent(*i1) && (*i1)->category() == catNum)
    {
      ruleNumOut = (*i1)->generalRule();
      tariffNumOut = (*i1)->generalRuleTariff();
      return true;
    }
  }

  return false;
}

bool
GeneralRuleAppDAO::getTariffRuleByTvlDate(DeleteList& del,
                                          const VendorCode& vendor,
                                          const CarrierCode& carrier,
                                          const TariffNumber& tariffNumber,
                                          const RuleNumber& ruleNumber,
                                          const DateTime& ticketDate,
                                          CatNumber catNum,
                                          RuleNumber& ruleNumOut,
                                          TariffNumber& tariffNumOut,
                                          DateTime tvlDate)
{
  GeneralRuleAppKey key(vendor, carrier, tariffNumber, ruleNumber);
  DAOCache::pointer_type ptr = cache().get(key);

  IsNotEffectiveG<GeneralRuleApp> notCurrent(tvlDate, ticketDate);
  std::vector<GeneralRuleApp*>::const_iterator i1 = ptr->begin();
  std::vector<GeneralRuleApp*>::const_iterator i2 = ptr->end();
  for (; i1 != i2; ++i1)
  {
    if (!notCurrent(*i1) && (*i1)->category() == catNum)
    {

      ruleNumOut = (*i1)->generalRule();
      tariffNumOut = (*i1)->generalRuleTariff();
      return true;
    }
  }

  return false;
}

std::vector<GeneralRuleApp*>*
GeneralRuleAppDAO::create(GeneralRuleAppKey key)
{
  std::vector<GeneralRuleApp*>* ret = new std::vector<GeneralRuleApp*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetGeneralRule gr(dbAdapter->getAdapter());
    gr.findGeneralRuleApp(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in GeneralRuleAppDAO::create()");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
GeneralRuleAppDAO::destroy(GeneralRuleAppKey key, std::vector<GeneralRuleApp*>* recs)
{
  std::vector<GeneralRuleApp*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

GeneralRuleAppKey
GeneralRuleAppDAO::createKey(GeneralRuleApp* info)
{
  return GeneralRuleAppKey(info->vendor(), info->carrier(), info->ruleTariff(), info->rule());
}

void
GeneralRuleAppDAO::load()
{
  StartupLoaderNoDB<GeneralRuleApp, GeneralRuleAppDAO>();
}

sfc::CompressedData*
GeneralRuleAppDAO::compress(const std::vector<GeneralRuleApp*>* vect) const
{
  return compressVector(vect);
}

std::vector<GeneralRuleApp*>*
GeneralRuleAppDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<GeneralRuleApp>(compressed);
}

std::string
GeneralRuleAppDAO::_name("GeneralRuleApp");
std::string
GeneralRuleAppDAO::_cacheClass("Rules");
DAOHelper<GeneralRuleAppDAO>
GeneralRuleAppDAO::_helper(_name);
GeneralRuleAppDAO* GeneralRuleAppDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: GeneralRuleAppHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
GeneralRuleAppHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.GeneralRuleAppHistoricalDAO"));
GeneralRuleAppHistoricalDAO&
GeneralRuleAppHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<GeneralRuleApp*>&
GeneralRuleAppHistoricalDAO::get(DeleteList& del,
                                 const VendorCode& vendor,
                                 const CarrierCode& carrier,
                                 const TariffNumber& tariffNumber,
                                 const RuleNumber& ruleNumber,
                                 const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  GeneralRuleAppHistoricalKey key(vendor, carrier, tariffNumber, ruleNumber);
  DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<GeneralRuleApp*>* ret = new std::vector<GeneralRuleApp*>;
  del.adopt(ret);

  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotCurrentH<GeneralRuleApp>(ticketDate));

  return *ret;
}

const std::vector<GeneralRuleApp*>&
GeneralRuleAppHistoricalDAO::getByTvlDate(DeleteList& del,
                                          const VendorCode& vendor,
                                          const CarrierCode& carrier,
                                          const TariffNumber& tariffNumber,
                                          const RuleNumber& ruleNumber,
                                          const DateTime& ticketDate,
                                          DateTime tvlDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  GeneralRuleAppHistoricalKey key(vendor, carrier, tariffNumber, ruleNumber);
  DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  std::vector<GeneralRuleApp*>* ret = new std::vector<GeneralRuleApp*>;
  del.adopt(ret);

  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotEffectiveH<GeneralRuleApp>(tvlDate, ticketDate));

  return *ret;
}


GeneralRuleApp*
GeneralRuleAppHistoricalDAO::get(DeleteList& del,
                                 const VendorCode& vendor,
                                 const CarrierCode& carrier,
                                 const TariffNumber& tariffNumber,
                                 const RuleNumber& ruleNumber,
                                 const DateTime& ticketDate,
                                 CatNumber catNum)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  GeneralRuleAppHistoricalKey key(vendor, carrier, tariffNumber, ruleNumber);
  DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  IsNotCurrentH<GeneralRuleApp> notCurrent(ticketDate);
  std::vector<GeneralRuleApp*>::const_iterator i1 = ptr->begin();
  std::vector<GeneralRuleApp*>::const_iterator i2 = ptr->end();
  for (; i1 != i2; ++i1)
  {
    if (!notCurrent(*i1) && (*i1)->category() == catNum)
    {
      return *i1;
    }
  }

  return nullptr;
}

GeneralRuleApp*
GeneralRuleAppHistoricalDAO::getByTvlDate(DeleteList& del,
                                          const VendorCode& vendor,
                                          const CarrierCode& carrier,
                                          const TariffNumber& tariffNumber,
                                          const RuleNumber& ruleNumber,
                                          const DateTime& ticketDate,
                                          CatNumber catNum,
                                          DateTime tvlDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  GeneralRuleAppHistoricalKey key(vendor, carrier, tariffNumber, ruleNumber);
  DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);

  IsNotCurrentH<GeneralRuleApp> notCurrent(ticketDate);
  std::vector<GeneralRuleApp*>::const_iterator i1 = ptr->begin();
  std::vector<GeneralRuleApp*>::const_iterator i2 = ptr->end();
  for (; i1 != i2; ++i1)
  {
    if (!notCurrent(*i1) && (*i1)->category() == catNum)
    {
      return *i1;
    }
  }

  return nullptr;
}

bool
GeneralRuleAppHistoricalDAO::getTariffRule(DeleteList& del,
                                           const VendorCode& vendor,
                                           const CarrierCode& carrier,
                                           const TariffNumber& tariffNumber,
                                           const RuleNumber& ruleNumber,
                                           const DateTime& ticketDate,
                                           CatNumber catNum,
                                           RuleNumber& ruleNumOut,
                                           TariffNumber& tariffNumOut)
{
  GeneralRuleAppHistoricalKey key(vendor, carrier, tariffNumber, ruleNumber);
  DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);

  IsNotCurrentH<GeneralRuleApp> notCurrent(ticketDate);
  std::vector<GeneralRuleApp*>::const_iterator i1 = ptr->begin();
  std::vector<GeneralRuleApp*>::const_iterator i2 = ptr->end();
  for (; i1 != i2; ++i1)
  {
    if (!notCurrent(*i1) && (*i1)->category() == catNum)
    {
      ruleNumOut = (*i1)->generalRule();
      tariffNumOut = (*i1)->generalRuleTariff();
      return true;
    }
  }

  return false;
}

bool
GeneralRuleAppHistoricalDAO::getTariffRuleByTvlDate(DeleteList& del,
                                                    const VendorCode& vendor,
                                                    const CarrierCode& carrier,
                                                    const TariffNumber& tariffNumber,
                                                    const RuleNumber& ruleNumber,
                                                    const DateTime& ticketDate,
                                                    CatNumber catNum,
                                                    RuleNumber& ruleNumOut,
                                                    TariffNumber& tariffNumOut,
                                                    DateTime tvlDate)
{
  GeneralRuleAppHistoricalKey key(vendor, carrier, tariffNumber, ruleNumber);
  DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);

  IsNotCurrentH<GeneralRuleApp> notCurrent(ticketDate);
  std::vector<GeneralRuleApp*>::const_iterator i1 = ptr->begin();
  std::vector<GeneralRuleApp*>::const_iterator i2 = ptr->end();
  for (; i1 != i2; ++i1)
  {
    if (!notCurrent(*i1) && (*i1)->category() == catNum)
    {
      ruleNumOut = (*i1)->generalRule();
      tariffNumOut = (*i1)->generalRuleTariff();
      return true;
    }
  }

  return false;
}

std::vector<GeneralRuleApp*>*
GeneralRuleAppHistoricalDAO::create(GeneralRuleAppHistoricalKey key)
{
  std::vector<GeneralRuleApp*>* ret = new std::vector<GeneralRuleApp*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetGeneralRuleHistorical gr(dbAdapter->getAdapter());
    gr.findGeneralRuleApp(*ret, key._a, key._b, key._c, key._d, key._e, key._f);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in GeneralRuleAppHistoricalDAO::create()");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
GeneralRuleAppHistoricalDAO::destroy(GeneralRuleAppHistoricalKey key,
                                     std::vector<GeneralRuleApp*>* recs)
{
  std::vector<GeneralRuleApp*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

GeneralRuleAppHistoricalKey
GeneralRuleAppHistoricalDAO::createKey(const GeneralRuleApp* info,
                                       const DateTime& startDate,
                                       const DateTime& endDate)
{
  return GeneralRuleAppHistoricalKey(
      info->vendor(), info->carrier(), info->ruleTariff(), info->rule(), startDate, endDate);
}

void
GeneralRuleAppHistoricalDAO::load()
{
  StartupLoaderNoDB<GeneralRuleApp, GeneralRuleAppHistoricalDAO>();
}

sfc::CompressedData*
GeneralRuleAppHistoricalDAO::compress(const std::vector<GeneralRuleApp*>* vect) const
{
  return compressVector(vect);
}

std::vector<GeneralRuleApp*>*
GeneralRuleAppHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<GeneralRuleApp>(compressed);
}

std::string
GeneralRuleAppHistoricalDAO::_name("GeneralRuleAppHistorical");
std::string
GeneralRuleAppHistoricalDAO::_cacheClass("Rules");
DAOHelper<GeneralRuleAppHistoricalDAO>
GeneralRuleAppHistoricalDAO::_helper(_name);
GeneralRuleAppHistoricalDAO* GeneralRuleAppHistoricalDAO::_instance = nullptr;

} // namespace tse
