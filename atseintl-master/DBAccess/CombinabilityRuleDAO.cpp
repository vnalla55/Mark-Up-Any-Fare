//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/CombinabilityRuleDAO.h"

#include "Common/Logger.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DBHistoryServer.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetCombCtrl.h"

namespace tse
{
log4cxx::LoggerPtr
CombinabilityRuleDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.CombinabilityRuleDAO"));

CombinabilityRuleDAO&
CombinabilityRuleDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<CombinabilityRuleInfo*>&
getCombinabilityRuleData(const VendorCode& vendor,
                         const CarrierCode& carrier,
                         const TariffNumber& ruleTariff,
                         const RuleNumber& rule,
                         const DateTime& date,
                         DeleteList& deleteList,
                         const DateTime& ticketDate,
                         bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    CombinabilityRuleHistoricalDAO& dao = CombinabilityRuleHistoricalDAO::instance();
    return dao.get(deleteList, vendor, carrier, ruleTariff, rule, date, ticketDate);
  }
  else
  {
    CombinabilityRuleDAO& dao = CombinabilityRuleDAO::instance();
    return dao.get(deleteList, vendor, carrier, ruleTariff, rule, date, ticketDate);
  }
}

const std::vector<CombinabilityRuleInfo*>&
CombinabilityRuleDAO::get(DeleteList& del,
                          const VendorCode& vendor,
                          const CarrierCode& carrier,
                          const TariffNumber& ruleTariff,
                          const RuleNumber& rule,
                          const DateTime& date,
                          const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  CombinabilityRuleKey key(vendor, carrier, ruleTariff, rule);
  DAOCache::pointer_type ptr = cache().get(key);
  return *(applyFilter(del, ptr, IsNotEffectiveG<CombinabilityRuleInfo>(date, ticketDate)));
}

std::vector<CombinabilityRuleInfo*>*
CombinabilityRuleDAO::create(CombinabilityRuleKey key)
{
  std::vector<CombinabilityRuleInfo*>* ret = new std::vector<CombinabilityRuleInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCombCtrl cr(dbAdapter->getAdapter());
    cr.findCombinabilityRule(*ret, key._a, key._b, key._c, key._d, 10);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CombinabilityRuleDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CombinabilityRuleDAO::destroy(CombinabilityRuleKey key, std::vector<CombinabilityRuleInfo*>* recs)
{
  std::vector<CombinabilityRuleInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
CombinabilityRuleDAO::compress(const std::vector<CombinabilityRuleInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<CombinabilityRuleInfo*>*
CombinabilityRuleDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<CombinabilityRuleInfo>(compressed);
}

CombinabilityRuleKey
CombinabilityRuleDAO::createKey(CombinabilityRuleInfo* info)
{
  return CombinabilityRuleKey(
      info->vendorCode(), info->carrierCode(), info->tariffNumber(), info->ruleNumber());
}

void
CombinabilityRuleDAO::load()
{
  StartupLoaderNoDB<CombinabilityRuleInfo, CombinabilityRuleDAO>();
}

std::string
CombinabilityRuleDAO::_name("CombinabilityRule");
std::string
CombinabilityRuleDAO::_cacheClass("Rules");
DAOHelper<CombinabilityRuleDAO>
CombinabilityRuleDAO::_helper(_name);
CombinabilityRuleDAO* CombinabilityRuleDAO::_instance = nullptr;

////////////////////////////////////////////
// Historical DAO Object
////////////////////////////////////////////
log4cxx::LoggerPtr
CombinabilityRuleHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.CombinabilityRuleHistoricalDAO"));

CombinabilityRuleHistoricalDAO&
CombinabilityRuleHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<CombinabilityRuleInfo*>&
CombinabilityRuleHistoricalDAO::get(DeleteList& del,
                                    const VendorCode& vendor,
                                    const CarrierCode& carrier,
                                    const TariffNumber& ruleTariff,
                                    const RuleNumber& rule,
                                    const DateTime& date,
                                    const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  CombinabilityRuleHistoricalKey key(vendor, carrier, ruleTariff, rule);
  DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<CombinabilityRuleInfo*>* ret = new std::vector<CombinabilityRuleInfo*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveH<CombinabilityRuleInfo>(date, ticketDate));
  return *ret;
}

std::vector<CombinabilityRuleInfo*>*
CombinabilityRuleHistoricalDAO::create(CombinabilityRuleHistoricalKey key)
{
  std::vector<CombinabilityRuleInfo*>* ret = new std::vector<CombinabilityRuleInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetCombCtrlHistorical cc(dbAdapter->getAdapter());
    cc.findCombinabilityRule(*ret, key._a, key._b, key._c, key._d, 10, key._e, key._f);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CombinabilityRuleHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
CombinabilityRuleHistoricalDAO::destroy(CombinabilityRuleHistoricalKey key,
                                        std::vector<CombinabilityRuleInfo*>* recs)
{
  std::vector<CombinabilityRuleInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

CombinabilityRuleHistoricalKey
CombinabilityRuleHistoricalDAO::createKey(const CombinabilityRuleInfo* info,
                                          const DateTime& startDate,
                                          const DateTime& endDate)
{
  return CombinabilityRuleHistoricalKey(info->vendorCode(),
                                        info->carrierCode(),
                                        info->tariffNumber(),
                                        info->ruleNumber(),
                                        startDate,
                                        endDate);
}

sfc::CompressedData*
CombinabilityRuleHistoricalDAO::compress(const std::vector<CombinabilityRuleInfo*>* vect) const
{
  return compressVector(vect);
}

std::vector<CombinabilityRuleInfo*>*
CombinabilityRuleHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<CombinabilityRuleInfo>(compressed);
}

void
CombinabilityRuleHistoricalDAO::load()
{
  StartupLoaderNoDB<CombinabilityRuleInfo, CombinabilityRuleHistoricalDAO>();
}

std::string
CombinabilityRuleHistoricalDAO::_name("CombinabilityRuleHistorical");
std::string
CombinabilityRuleHistoricalDAO::_cacheClass("Rules");
DAOHelper<CombinabilityRuleHistoricalDAO>
CombinabilityRuleHistoricalDAO::_helper(_name);
CombinabilityRuleHistoricalDAO* CombinabilityRuleHistoricalDAO::_instance = nullptr;

} // namespace tse
