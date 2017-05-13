//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/MinFareRuleLevelExclDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/MinFareRuleLevelExcl.h"
#include "DBAccess/Queries/QueryGetMinFareRule.h"

namespace tse
{
log4cxx::LoggerPtr
MinFareRuleLevelExclDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.MinFareRuleLevelExclDAO"));

MinFareRuleLevelExclDAO&
MinFareRuleLevelExclDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<MinFareRuleLevelExcl*>&
getMinFareRuleLevelExclData(const VendorCode& vendor,
                            int textTblItemNo,
                            const CarrierCode& governingCarrier,
                            const TariffNumber& ruleTariff,
                            const DateTime& date,
                            DeleteList& deleteList,
                            const DateTime& ticketDate,
                            bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    MinFareRuleLevelExclHistoricalDAO& dao = MinFareRuleLevelExclHistoricalDAO::instance();
    return dao.get(
        deleteList, vendor, textTblItemNo, governingCarrier, ruleTariff, date, ticketDate);
  }
  else
  {
    MinFareRuleLevelExclDAO& dao = MinFareRuleLevelExclDAO::instance();
    return dao.get(
        deleteList, vendor, textTblItemNo, governingCarrier, ruleTariff, date, ticketDate);
  }
}

const std::vector<MinFareRuleLevelExcl*>&
MinFareRuleLevelExclDAO::get(DeleteList& del,
                             const VendorCode& vendor,
                             int textTblItemNo,
                             const CarrierCode& governingCarrier,
                             const TariffNumber& ruleTariff,
                             const DateTime& date,
                             const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  MinFareRuleLevelExclKey key(vendor, textTblItemNo, governingCarrier, ruleTariff);
  DAOCache::pointer_type ptr = cache().get(key);
  /*
  std::vector<MinFareRuleLevelExcl*>* ret = new std::vector<MinFareRuleLevelExcl*>;
  del.adopt(ret);
  if (!ptr->empty()) {
      del.copy(ptr);
      remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret),
                     IsNotEffectiveG<MinFareRuleLevelExcl>(date, ticketDate));
  }

  return *ret;
  */
  return *(applyFilter(del, ptr, IsNotEffectiveG<MinFareRuleLevelExcl>(date, ticketDate)));
}

std::vector<MinFareRuleLevelExcl*>*
MinFareRuleLevelExclDAO::create(MinFareRuleLevelExclKey key)
{
  std::vector<MinFareRuleLevelExcl*>* ret = new std::vector<MinFareRuleLevelExcl*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMinFareRuleLevExclBase rleb(dbAdapter->getAdapter());
    rleb.findMinFareRuleLevelExcl(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MinFareRuleLevelExclDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MinFareRuleLevelExclDAO::destroy(MinFareRuleLevelExclKey key,
                                 std::vector<MinFareRuleLevelExcl*>* recs)
{
  std::vector<MinFareRuleLevelExcl*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

size_t
MinFareRuleLevelExclDAO::invalidate(const ObjectKey& objectKey)
{
  MinFareRuleLevelExclKey invKey;
  if (!translateKey(objectKey, invKey))
  {
    LOG4CXX_ERROR(_logger, "Cache notification key translation error");
    return 0;
  }
  // If the query OR condition default values are not in the key, no overriding
  //_b = texttblitemno; _c = governingcarrier; _d = ruletariff
  if ((invKey._b != 0) && (invKey._c != "") && (invKey._d != -1))
  {
    return DataAccessObject<MinFareRuleLevelExclKey, std::vector<MinFareRuleLevelExcl*> >::invalidate(
        invKey);
  }

  // Flush all possible MINFARERULELEVELEXCL variations of ObjectKey
  std::shared_ptr<std::vector<MinFareRuleLevelExclKey>> cacheKeys = cache().keys();

  std::vector<MinFareRuleLevelExclKey>* pCacheKeys = cacheKeys.get();

  if (pCacheKeys == nullptr)
  {
    LOG4CXX_DEBUG(_logger, "No cache entries to iterate");
    return 0;
  }

  uint32_t nKeys = pCacheKeys->size();
  size_t result(0);
  for (uint32_t i = 0; (i < nKeys); i++)
  {
    MinFareRuleLevelExclKey cacheKey = (*pCacheKeys)[i];
    if (((invKey._a == cacheKey._a)) && ((invKey._b == 0) || (invKey._b == cacheKey._b)) &&
        ((invKey._c.empty()) || (invKey._c == cacheKey._c)) &&
        ((invKey._d == -1) || (invKey._d == cacheKey._d)))
    {
      result += DataAccessObject<MinFareRuleLevelExclKey, std::vector<MinFareRuleLevelExcl*> >::invalidate(
          cacheKey);
    }
  }
  return result;
}

MinFareRuleLevelExclKey
MinFareRuleLevelExclDAO::createKey(MinFareRuleLevelExcl* info)
{
  return MinFareRuleLevelExclKey(
      info->vendor(), info->textTblItemNo(), info->governingCarrier(), info->ruleTariff());
}

void
MinFareRuleLevelExclDAO::load()
{
  StartupLoaderNoDB<MinFareRuleLevelExcl, MinFareRuleLevelExclDAO>();
}

sfc::CompressedData*
MinFareRuleLevelExclDAO::compress(const std::vector<MinFareRuleLevelExcl*>* vect) const
{
  return compressVector(vect);
}

std::vector<MinFareRuleLevelExcl*>*
MinFareRuleLevelExclDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MinFareRuleLevelExcl>(compressed);
}

std::string
MinFareRuleLevelExclDAO::_name("MinFareRuleLevelExcl");
std::string
MinFareRuleLevelExclDAO::_cacheClass("MinFares");
DAOHelper<MinFareRuleLevelExclDAO>
MinFareRuleLevelExclDAO::_helper(_name);
MinFareRuleLevelExclDAO* MinFareRuleLevelExclDAO::_instance = nullptr;

//////////////////////////////////////////////////////
// Historical DAO Object
//////////////////////////////////////////////////////
log4cxx::LoggerPtr
MinFareRuleLevelExclHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.MinFareRuleLevelExclHistoricalDAO"));

MinFareRuleLevelExclHistoricalDAO&
MinFareRuleLevelExclHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<MinFareRuleLevelExcl*>&
MinFareRuleLevelExclHistoricalDAO::get(DeleteList& del,
                                       const VendorCode& vendor,
                                       int textTblItemNo,
                                       const CarrierCode& governingCarrier,
                                       const TariffNumber& ruleTariff,
                                       const DateTime& date,
                                       const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  MinFareRuleLevelExclHistoricalKey key(vendor, textTblItemNo, governingCarrier, ruleTariff);
  DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<MinFareRuleLevelExcl*>* ret = new std::vector<MinFareRuleLevelExcl*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveHist<MinFareRuleLevelExcl>(date, ticketDate));
  return *ret;
}

std::vector<MinFareRuleLevelExcl*>*
MinFareRuleLevelExclHistoricalDAO::create(MinFareRuleLevelExclHistoricalKey key)
{
  std::vector<MinFareRuleLevelExcl*>* ret = new std::vector<MinFareRuleLevelExcl*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get("Historical", true);
  try
  {
    QueryGetMinFareRuleLevExclBaseHistorical mfrle(dbAdapter->getAdapter());
    mfrle.findMinFareRuleLevelExcl(*ret, key._a, key._b, key._c, key._d, key._e, key._f);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MinFareRuleLevelExclHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MinFareRuleLevelExclHistoricalDAO::destroy(MinFareRuleLevelExclHistoricalKey key,
                                           std::vector<MinFareRuleLevelExcl*>* recs)
{
  std::vector<MinFareRuleLevelExcl*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
MinFareRuleLevelExclHistoricalDAO::compress(const std::vector<MinFareRuleLevelExcl*>* vect) const
{
  return compressVector(vect);
}

std::vector<MinFareRuleLevelExcl*>*
MinFareRuleLevelExclHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MinFareRuleLevelExcl>(compressed);
}

std::string
MinFareRuleLevelExclHistoricalDAO::_name("MinFareRuleLevelExclHistorical");
std::string
MinFareRuleLevelExclHistoricalDAO::_cacheClass("MinFares");
DAOHelper<MinFareRuleLevelExclHistoricalDAO>
MinFareRuleLevelExclHistoricalDAO::_helper(_name);
MinFareRuleLevelExclHistoricalDAO* MinFareRuleLevelExclHistoricalDAO::_instance = nullptr;

} // namespace tse
