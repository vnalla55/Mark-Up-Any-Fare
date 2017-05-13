//----------------------------------------------------------------------------
//
//  Copyright (c) Sabre 2013
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//
//----------------------------------------------------------------------------
#include "DBAccess/GenSalesAgentDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DAOUtils.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/GenSalesAgentInfo.h"
#include "DBAccess/Queries/QueryGetGenSalesAgent.h"

#include <algorithm>

namespace tse
{
log4cxx::LoggerPtr
GenSalesAgentDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.GenSalesAgentDAO"));

GenSalesAgentDAO&
GenSalesAgentDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
    _helper.init();
  return *_instance;
}

bool
GenSalesAgentDAO::translateKey(const ObjectKey& objKey, GenSalesAgentKey& key) const
{
  return (key.initialized = objKey.getValue("GDS", key._a) &&
                            objKey.getValue("NATION", key._b) &&
                            objKey.getValue("SETTLEMENTPLAN", key._c) &&
                            objKey.getValue("NONPARTICIPATINGCARRIER", key._d));
}

void
GenSalesAgentDAO::translateKey(const GenSalesAgentKey& key, ObjectKey& objKey) const
{
  objKey.setValue("GDS", key._a);
  objKey.setValue("NATION", key._b);
  objKey.setValue("SETTLEMENTPLAN", key._c);
  objKey.setValue("NONPARTICIPATINGCARRIER", key._d);
}

const std::vector<GenSalesAgentInfo*>&
getGenSalesAgentData(DeleteList& deleteList,
                     const CrsCode& gds,
                     const NationCode& country,
                     const SettlementPlanType& settlementPlan,
                     const DateTime& date)
{
  GenSalesAgentDAO& dao = GenSalesAgentDAO::instance();
  return dao.get(deleteList, gds, country, settlementPlan, date);
}

const std::vector<GenSalesAgentInfo*>&
getGenSalesAgentData(DeleteList& deleteList,
                     const CrsCode& gds,
                     const NationCode& country,
                     const SettlementPlanType& settlementPlan,
                     const CarrierCode& validatingCxr,
                     const DateTime& ticketDate,
                     bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    GenSalesAgentHistoricalDAO& dao = GenSalesAgentHistoricalDAO::instance();
    return dao.get(deleteList, gds, country, settlementPlan, validatingCxr, ticketDate);
  }
  else
  {
    GenSalesAgentDAO& dao = GenSalesAgentDAO::instance();
    return dao.get(deleteList, gds, country, settlementPlan, validatingCxr, ticketDate);
  }
}

const std::vector<GenSalesAgentInfo*>&
GenSalesAgentDAO::get(DeleteList& del,
                      const CrsCode& gds,
                      const NationCode& country,
                      const SettlementPlanType& settlementPlan,
                      const CarrierCode& nonParticipatingCxr,
                      const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  GenSalesAgentKey key(gds, country, settlementPlan, nonParticipatingCxr);
  DAOCache::pointer_type ptr = cache().get(key);
  return *applyFilter(del,
                      ptr,
                      IsNotEffectiveG<GenSalesAgentInfo>(ticketDate));
}

const std::vector<GenSalesAgentInfo*>&
GenSalesAgentDAO::get(DeleteList& del,
                      const CrsCode& gds,
                      const NationCode& country,
                      const SettlementPlanType& settlementPlan,
                      const DateTime& date)
{
  _codeCoverageGetCallCount++;

  std::vector<GenSalesAgentInfo*>* gsaList =
      new std::vector<GenSalesAgentInfo*>;

  std::shared_ptr<std::vector<GenSalesAgentKey>> keys = cache().keys();

  for (std::vector<GenSalesAgentKey>::const_iterator itKey = keys->begin();
       itKey != keys->end();
       ++itKey)
  {
    const GenSalesAgentKey& key = (*itKey);
    if ( (key._a == gds) && (key._b == country) && (key._c == settlementPlan) )
    {
      DAOCache::pointer_type ptr = cache().getIfResident(key);
      if (ptr)
      {
        del.copy(ptr);
        remove_copy_if(ptr->begin(),
                       ptr->end(),
                       back_inserter(*gsaList),
                       IsNotEffectiveG<GenSalesAgentInfo>(date));
      }
    }
  }

  return *gsaList;
}

std::vector<GenSalesAgentInfo*>*
GenSalesAgentDAO::create(GenSalesAgentKey key)
{
  std::vector<GenSalesAgentInfo*>* ret = new std::vector<GenSalesAgentInfo*>;
  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());

  try
  {
    QueryGetGenSalesAgent gsa(dbAdapter->getAdapter());
    gsa.findGenSalesAgents(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in GenSalesAgentDAO::create()");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

void
GenSalesAgentDAO::destroy(GenSalesAgentKey key, std::vector<GenSalesAgentInfo*>* recs)
{
  destroyContainer(recs);
}

GenSalesAgentKey
GenSalesAgentDAO::createKey(GenSalesAgentInfo* info)
{
  return GenSalesAgentKey(info->getGDSCode(),
                          info->getCountryCode(),
                          info->getSettlementPlanCode(),
                          info->getNonParticipatingCxr());
}

size_t
GenSalesAgentDAO::clear()
{
  size_t result(cache().clear());
  load();
  LOG4CXX_ERROR(_logger, "GenSalesAgent cache cleared");
  return result;
}

void
GenSalesAgentDAO::load()
{
  StartupLoader<QueryGetAllGenSalesAgents, GenSalesAgentInfo, GenSalesAgentDAO>();
}

std::string
GenSalesAgentDAO::_name("GenSalesAgent");
std::string
GenSalesAgentDAO::_cacheClass("Common");
DAOHelper<GenSalesAgentDAO>
GenSalesAgentDAO::_helper(_name);
GenSalesAgentDAO* GenSalesAgentDAO::_instance = nullptr;

//-----------------------------------------------------------------------------
// HISTORICAL
//-----------------------------------------------------------------------------

log4cxx::LoggerPtr
GenSalesAgentHistoricalDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.GenSalesAgentHistoricalDAO"));

GenSalesAgentHistoricalDAO&
GenSalesAgentHistoricalDAO::instance()
{
  if (_instance == nullptr)
    _helper.init();
  return *_instance;
}

const std::vector<GenSalesAgentInfo*>&
GenSalesAgentHistoricalDAO::get(DeleteList& del,
                                const CrsCode& gds,
                                const NationCode& country,
                                const SettlementPlanType& settlementPlan,
                                const CarrierCode& nonParticipatingCxr,
                                const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  GenSalesAgentHistoricalKey gsaKey(gds, country, settlementPlan, nonParticipatingCxr);
  DAOUtils::getDateRange(ticketDate, gsaKey._e, gsaKey._f, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(gsaKey);
  del.copy(ptr);
  std::vector<GenSalesAgentInfo*>* gsaList = new std::vector<GenSalesAgentInfo*>;
  del.adopt(gsaList);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*gsaList),
                 IsNotEffectiveH<GenSalesAgentInfo>(ticketDate, ticketDate));
  return *gsaList;
}

std::vector<GenSalesAgentInfo*>*
GenSalesAgentHistoricalDAO::create(GenSalesAgentHistoricalKey gsaKey)
{
  std::vector<GenSalesAgentInfo*>* gsaList = new std::vector<GenSalesAgentInfo*>;
  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());

  try
  {
    QueryGetGenSalesAgentHistorical gsa(dbAdapter->getAdapter());
    gsa.findGenSalesAgents(*gsaList, gsaKey._a, gsaKey._b, gsaKey._c, gsaKey._d, gsaKey._e, gsaKey._f);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in GenSalesAgentHistoricalDAO::create()");
    destroyContainer(gsaList);
    throw;
  }
  return gsaList;
}

void
GenSalesAgentHistoricalDAO::destroy(GenSalesAgentHistoricalKey key, std::vector<GenSalesAgentInfo*>* gsaList)
{
  destroyContainer(gsaList);
}

bool
GenSalesAgentHistoricalDAO::translateKey(const ObjectKey& objKey, GenSalesAgentHistoricalKey& gsaKey) const
{
  return (gsaKey.initialized = objKey.getValue("GDS", gsaKey._a) &&
                               objKey.getValue("NATION", gsaKey._b) &&
                               objKey.getValue("SETTLEMENTPLAN", gsaKey._c) &&
                               objKey.getValue("NONPARTICIPATINGCARRIER", gsaKey._d));
}

void
GenSalesAgentHistoricalDAO::translateKey(const GenSalesAgentHistoricalKey& gsaKey, ObjectKey& objKey) const
{
  objKey.setValue("GDS", gsaKey._a);
  objKey.setValue("NATION", gsaKey._b);
  objKey.setValue("SETTLEMENTPLAN", gsaKey._c);
  objKey.setValue("NONPARTICIPATINGCARRIER", gsaKey._d);
}

bool
GenSalesAgentHistoricalDAO::translateKey(const ObjectKey& objKey,
                                         GenSalesAgentHistoricalKey& gsaKey,
                                         const DateTime ticketDate) const
{
  DAOUtils::getDateRange(ticketDate, gsaKey._e, gsaKey._f, _cacheBy);
  return translateKey(objKey, gsaKey);
}

std::string
GenSalesAgentHistoricalDAO::_name("GenSalesAgentHistorical");
std::string
GenSalesAgentHistoricalDAO::_cacheClass("Common");
DAOHelper<GenSalesAgentHistoricalDAO>
GenSalesAgentHistoricalDAO::_helper(_name);
GenSalesAgentHistoricalDAO* GenSalesAgentHistoricalDAO::_instance = nullptr;

} // namespace tse
