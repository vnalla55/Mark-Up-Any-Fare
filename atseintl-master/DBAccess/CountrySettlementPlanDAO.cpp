//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/CountrySettlementPlanDAO.h"

#include "Common/Logger.h"
#include "DBAccess/CountrySettlementPlanInfo.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DAOUtils.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/HistoricalDataAccessObject.h"
#include "DBAccess/Queries/QueryGetCountrySettlementPlan.h"

#include <algorithm>

namespace tse
{

log4cxx::LoggerPtr
CountrySettlementPlanDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.CountrySettlementPlanDAO"));

CountrySettlementPlanDAO&
CountrySettlementPlanDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<CountrySettlementPlanInfo*>&
getCountrySettlementPlanData(const NationCode& countryCode,
                             DeleteList& deleteList,
                             const DateTime& ticketDate,
                             bool isHistorical)
{
  if (isHistorical)
  {
    CountrySettlementPlanHistoricalDAO& dao = CountrySettlementPlanHistoricalDAO::instance();
    return dao.get(deleteList, countryCode, ticketDate);
  }
  else
  {
    CountrySettlementPlanDAO& dao = CountrySettlementPlanDAO::instance();
    return dao.get(deleteList, countryCode, ticketDate);
  }

}

const std::vector<CountrySettlementPlanInfo*>&
CountrySettlementPlanDAO::get(DeleteList& del,
                              const NationCode& countryCode,
                              const DateTime& ticketDate)
{
  _codeCoverageGetCallCount++;

  CountrySettlementPlanKey key(countryCode);
  DAOCache::pointer_type ptr = cache().get(key);
  return *applyFilter(del,
                      ptr,
                      IsNotEffectiveG<CountrySettlementPlanInfo>(ticketDate));
}

std::vector<CountrySettlementPlanInfo*>*
CountrySettlementPlanDAO::create(CountrySettlementPlanKey cspKey)
{
  std::vector<CountrySettlementPlanInfo*>* cspList =
      new std::vector<CountrySettlementPlanInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCountrySettlementPlan csp(dbAdapter->getAdapter());
    csp.findCountrySettlementPlan(*cspList, cspKey._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CountrySettlementPlanDAO::create");
    destroyContainer(cspList);
    throw;
  }

  return cspList;
}

CountrySettlementPlanKey
CountrySettlementPlanDAO::createKey(const CountrySettlementPlanInfo* info)
{
  return CountrySettlementPlanKey(info->getCountryCode());
}

bool
CountrySettlementPlanDAO::translateKey(const ObjectKey& objectKey, CountrySettlementPlanKey& key)
    const
{
  return key.initialized = objectKey.getValue("NATION", key._a);
}

void
CountrySettlementPlanDAO::translateKey(const CountrySettlementPlanKey& key, ObjectKey& objectKey)
    const
{
  objectKey.setValue("NATION", key._a);
}

bool
CountrySettlementPlanDAO::insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
{
  return DummyObjectInserter<CountrySettlementPlanInfo, CountrySettlementPlanDAO>(
             flatKey, objectKey).success();
}

void
CountrySettlementPlanDAO::destroy(CountrySettlementPlanKey key,
                                  std::vector<CountrySettlementPlanInfo*>* cspList)
{
  destroyContainer(cspList);
}

void
CountrySettlementPlanDAO::load()
{
  StartupLoaderNoDB<CountrySettlementPlanInfo, CountrySettlementPlanDAO>();
}

size_t
CountrySettlementPlanDAO::clear()
{
  size_t result(cache().clear());
  LOG4CXX_ERROR(_logger, "CountrySettlementPlan cache cleared");
  return result;
}

std::string
CountrySettlementPlanDAO::_name("CountrySettlementPlan");
std::string
CountrySettlementPlanDAO::_cacheClass("Common");
DAOHelper<CountrySettlementPlanDAO>
CountrySettlementPlanDAO::_helper(_name);
CountrySettlementPlanDAO* CountrySettlementPlanDAO::_instance = nullptr;

//-----------------------------------------------------------------------------
// HISTORICAL
//-----------------------------------------------------------------------------

log4cxx::LoggerPtr
CountrySettlementPlanHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.CountrySettlementPlanHistoricalDAO"));

CountrySettlementPlanHistoricalDAO&
CountrySettlementPlanHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<CountrySettlementPlanInfo*>&
CountrySettlementPlanHistoricalDAO::get(DeleteList& del,
                                        const NationCode& countryCode,
                                        const DateTime& ticketDate)
{
  _codeCoverageGetCallCount++;

  CountrySettlementPlanHistoricalKey cspKey(countryCode);
  DAOUtils::getDateRange(ticketDate, cspKey._b, cspKey._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(cspKey);
  del.copy(ptr);

  std::vector<CountrySettlementPlanInfo*>* cspList = new std::vector<CountrySettlementPlanInfo*>;
  del.adopt(cspList);

  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*cspList),
                 IsNotEffectiveH<CountrySettlementPlanInfo>(ticketDate, ticketDate));
  return *cspList;
}

std::vector<CountrySettlementPlanInfo*>*
CountrySettlementPlanHistoricalDAO::create(CountrySettlementPlanHistoricalKey cspKey)
{
  std::vector<CountrySettlementPlanInfo*>* cspList =
      new std::vector<CountrySettlementPlanInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetCountrySettlementPlanHistorical cspQuery(dbAdapter->getAdapter());
    cspQuery.findCountrySettlementPlan(*cspList, cspKey._a, cspKey._b, cspKey._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in CountrySettlementPlanHistoricalDAO::create");
    destroyContainer(cspList);
    throw;
  }

  return cspList;
}

bool
CountrySettlementPlanHistoricalDAO::translateKey(const ObjectKey& objectKey,
                                                 CountrySettlementPlanHistoricalKey& key) const
{
  return key.initialized = objectKey.getValue("NATION", key._a);
}

void
CountrySettlementPlanHistoricalDAO::translateKey(const CountrySettlementPlanHistoricalKey& key,
                                                 ObjectKey& objectKey) const
{
  objectKey.setValue("NATION", key._a);
}

bool
CountrySettlementPlanHistoricalDAO::translateKey(const ObjectKey& objectKey,
                                                 CountrySettlementPlanHistoricalKey& cspKey,
                                                 const DateTime ticketDate) const
{
  DAOUtils::getDateRange(ticketDate, cspKey._b, cspKey._c, _cacheBy);
  return translateKey(objectKey, cspKey);
}

void
CountrySettlementPlanHistoricalDAO::destroy(CountrySettlementPlanHistoricalKey key,
                                            std::vector<CountrySettlementPlanInfo*>* cspList)
{
  destroyContainer(cspList);
}

std::string
CountrySettlementPlanHistoricalDAO::_name("CountrySettlementPlanHistorical");
std::string
CountrySettlementPlanHistoricalDAO::_cacheClass("Common");
DAOHelper<CountrySettlementPlanHistoricalDAO>
CountrySettlementPlanHistoricalDAO::_helper(_name);
CountrySettlementPlanHistoricalDAO* CountrySettlementPlanHistoricalDAO::_instance = nullptr;

}
