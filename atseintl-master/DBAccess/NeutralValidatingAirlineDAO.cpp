//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/NeutralValidatingAirlineDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DAOUtils.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/HistoricalDataAccessObject.h"
#include "DBAccess/NeutralValidatingAirlineInfo.h"
#include "DBAccess/Queries/QueryGetNeutralValidatingAirline.h"

#include <algorithm>

namespace tse
{

log4cxx::LoggerPtr
NeutralValidatingAirlineDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.NeutralValidatingAirlineDAO"));

NeutralValidatingAirlineDAO&
NeutralValidatingAirlineDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<NeutralValidatingAirlineInfo*>&
getNeutralValidatingAirlineData(const NationCode& country,
                                const CrsCode& gds,
                                const SettlementPlanType& spType,
                                DeleteList& deleteList,
                                const DateTime& ticketDate,
                                bool isHistorical)
{
  if (isHistorical)
  {
    NeutralValidatingAirlineHistoricalDAO& dao = NeutralValidatingAirlineHistoricalDAO::instance();
    return dao.get(deleteList, country, gds, spType, ticketDate);
  }
  else
  {
    NeutralValidatingAirlineDAO& dao = NeutralValidatingAirlineDAO::instance();
    return dao.get(deleteList, country, gds, spType, ticketDate);
  }

}

const std::vector<NeutralValidatingAirlineInfo*>&
NeutralValidatingAirlineDAO::get(DeleteList& del,
                                 const NationCode& country,
                                 const CrsCode& gds,
                                 const SettlementPlanType& spType,
                                 const DateTime& ticketDate)
{
  _codeCoverageGetCallCount++;

  NeutralValidatingAirlineKey nvaKey(country, gds, spType);
  DAOCache::pointer_type ptr = cache().get(nvaKey);
  return *applyFilter(del,
                      ptr,
                      IsNotEffectiveG<NeutralValidatingAirlineInfo>(ticketDate));
}

std::vector<NeutralValidatingAirlineInfo*>*
NeutralValidatingAirlineDAO::create(NeutralValidatingAirlineKey nvaKey)
{
  std::vector<NeutralValidatingAirlineInfo*>* nvaList =
      new std::vector<NeutralValidatingAirlineInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetNeutralValidatingAirline nvaQuery(dbAdapter->getAdapter());
    nvaQuery.findNeutralValidatingAirlineInfo(*nvaList, nvaKey._a, nvaKey._b, nvaKey._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in NeutralValidatingAirlineDAO::create");
    destroyContainer(nvaList);
    throw;
  }

  return nvaList;
}

NeutralValidatingAirlineKey
NeutralValidatingAirlineDAO::createKey(const NeutralValidatingAirlineInfo* nva)
{
  return NeutralValidatingAirlineKey(
      nva->getCountryCode(), nva->getGds(), nva->getSettlementPlanType());
}

bool
NeutralValidatingAirlineDAO::translateKey(const ObjectKey& objectKey,
                                          NeutralValidatingAirlineKey& nvaKey) const
{
  nvaKey.initialized = objectKey.getValue("NATION", nvaKey._a) &&
                       objectKey.getValue("GDS", nvaKey._b) &&
                       objectKey.getValue("SETTLEMENTPLAN", nvaKey._c);

  return nvaKey.initialized;
}

void
NeutralValidatingAirlineDAO::translateKey(const NeutralValidatingAirlineKey& nvaKey,
                                          ObjectKey& objectKey) const
{
  objectKey.setValue("NATION", nvaKey._a);
  objectKey.setValue("GDS", nvaKey._b);
  objectKey.setValue("SETTLEMENTPLAN", nvaKey._c);
}

bool
NeutralValidatingAirlineDAO::insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
{
  return DummyObjectInserter<NeutralValidatingAirlineInfo, NeutralValidatingAirlineDAO>(
             flatKey, objectKey).success();
}

void
NeutralValidatingAirlineDAO::destroy(NeutralValidatingAirlineKey key,
                                     std::vector<NeutralValidatingAirlineInfo*>* nvaList)
{
  destroyContainer(nvaList);
}

void
NeutralValidatingAirlineDAO::load()
{
  StartupLoaderNoDB<NeutralValidatingAirlineInfo, NeutralValidatingAirlineDAO>();
}

size_t
NeutralValidatingAirlineDAO::clear()
{
  size_t result(cache().clear());
  LOG4CXX_ERROR(_logger, "NeutralValidatingAirline cache cleared");
  return result;
}

std::string
NeutralValidatingAirlineDAO::_name("NeutralValidatingAirline");
std::string
NeutralValidatingAirlineDAO::_cacheClass("Common");
DAOHelper<NeutralValidatingAirlineDAO>
NeutralValidatingAirlineDAO::_helper(_name);
NeutralValidatingAirlineDAO* NeutralValidatingAirlineDAO::_instance = nullptr;

//-----------------------------------------------------------------------------
// HISTORICAL
//-----------------------------------------------------------------------------

log4cxx::LoggerPtr
NeutralValidatingAirlineHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.NeutralValidatingAirlineHistoricalDAO"));

NeutralValidatingAirlineHistoricalDAO&
NeutralValidatingAirlineHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<NeutralValidatingAirlineInfo*>&
NeutralValidatingAirlineHistoricalDAO::get(DeleteList& del,
                                       const NationCode& country,
                                       const CrsCode& gds,
                                       const SettlementPlanType& spType,
                                       const DateTime& ticketDate)
{
  _codeCoverageGetCallCount++;

  NeutralValidatingAirlineHistoricalKey nvaKey(country, gds, spType);
  DAOUtils::getDateRange(ticketDate, nvaKey._d, nvaKey._e, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(nvaKey);
  del.copy(ptr);

  std::vector<NeutralValidatingAirlineInfo*>* nvaList =
      new std::vector<NeutralValidatingAirlineInfo*>;
  del.adopt(nvaList);

  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*nvaList),
                 IsNotEffectiveH<NeutralValidatingAirlineInfo>(ticketDate, ticketDate));
  return *nvaList;
}

std::vector<NeutralValidatingAirlineInfo*>*
NeutralValidatingAirlineHistoricalDAO::create(NeutralValidatingAirlineHistoricalKey nvaKey)
{
  std::vector<NeutralValidatingAirlineInfo*>* nvaList =
      new std::vector<NeutralValidatingAirlineInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetNeutralValidatingAirlineHistorical nvaQuery(dbAdapter->getAdapter());
    nvaQuery.findNeutralValidatingAirlineInfo(*nvaList, nvaKey._a, nvaKey._b, nvaKey._c, nvaKey._d, nvaKey._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in NeutralValidatingAirlineHistoricalDAO::create");
    destroyContainer(nvaList);
    throw;
  }

  return nvaList;
}

bool
NeutralValidatingAirlineHistoricalDAO::translateKey(const ObjectKey& objectKey,
                                                    NeutralValidatingAirlineHistoricalKey& nvaKey) const
{
  nvaKey.initialized = objectKey.getValue("NATION", nvaKey._a) &&
                       objectKey.getValue("GDS", nvaKey._b) &&
                       objectKey.getValue("SETTLEMENTPLAN", nvaKey._c);

  return nvaKey.initialized;
}

void
NeutralValidatingAirlineHistoricalDAO::translateKey(const NeutralValidatingAirlineHistoricalKey& nvaKey,
                                                    ObjectKey& objectKey) const
{
  objectKey.setValue("NATION", nvaKey._a);
  objectKey.setValue("GDS", nvaKey._b);
  objectKey.setValue("SETTLEMENTPLAN", nvaKey._c);
}

bool
NeutralValidatingAirlineHistoricalDAO::translateKey(const ObjectKey& objectKey,
                                                    NeutralValidatingAirlineHistoricalKey& nvaKey,
                                                    const DateTime ticketDate) const
{
  DAOUtils::getDateRange(ticketDate, nvaKey._d, nvaKey._e, _cacheBy);
  return translateKey(objectKey, nvaKey);
}

void
NeutralValidatingAirlineHistoricalDAO::destroy(NeutralValidatingAirlineHistoricalKey key,
                                               std::vector<NeutralValidatingAirlineInfo*>* nvaList)
{
  destroyContainer(nvaList);
}

std::string
NeutralValidatingAirlineHistoricalDAO::_name("NeutralValidatingAirlineHistorical");
std::string
NeutralValidatingAirlineHistoricalDAO::_cacheClass("Common");
DAOHelper<NeutralValidatingAirlineHistoricalDAO>
NeutralValidatingAirlineHistoricalDAO::_helper(_name);
NeutralValidatingAirlineHistoricalDAO* NeutralValidatingAirlineHistoricalDAO::_instance = nullptr;

}

