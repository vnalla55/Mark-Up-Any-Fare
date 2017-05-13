//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/AirlineCountrySettlementPlanDAO.h"

#include "Common/Logger.h"
#include "DBAccess/AirlineCountrySettlementPlanInfo.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DAOUtils.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/HistoricalDataAccessObject.h"
#include "DBAccess/Queries/QueryGetAirlineCountrySettlementPlan.h"

#include <algorithm>

namespace tse
{

log4cxx::LoggerPtr
AirlineCountrySettlementPlanDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.AirlineCountrySettlementPlanDAO"));

AirlineCountrySettlementPlanDAO&
AirlineCountrySettlementPlanDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<AirlineCountrySettlementPlanInfo*>&
getAirlineCountrySettlementPlanData(const CrsCode& gds,
                                    const NationCode& country,
                                    const CarrierCode& airline,
                                    DeleteList& deleteList,
                                    const DateTime& date)
{
  AirlineCountrySettlementPlanDAO& dao = AirlineCountrySettlementPlanDAO::instance();
  return dao.get(deleteList, gds, country, airline, date);
}

const std::vector<AirlineCountrySettlementPlanInfo*>&
getAirlineCountrySettlementPlanData(const NationCode& country,
                                    const CrsCode& gds,
                                    const SettlementPlanType& settlementPlan,
                                    DeleteList& deleteList,
                                    const DateTime& date)
{
  AirlineCountrySettlementPlanDAO& dao = AirlineCountrySettlementPlanDAO::instance();
  return dao.get(deleteList, country, gds, settlementPlan, date);
}

const std::vector<AirlineCountrySettlementPlanInfo*>&
getAirlineCountrySettlementPlanData(const NationCode& country,
                                    const CrsCode& gds,
                                    const CarrierCode& airline,
                                    const SettlementPlanType& spType,
                                    DeleteList& deleteList,
                                    const DateTime& ticketDate,
                                    bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    AirlineCountrySettlementPlanHistoricalDAO& dao =
        AirlineCountrySettlementPlanHistoricalDAO::instance();
    return dao.get(deleteList, country, gds, airline, spType, ticketDate);
  }
  else
  {
    AirlineCountrySettlementPlanDAO& dao = AirlineCountrySettlementPlanDAO::instance();
    return dao.get(deleteList, country, gds, airline, spType, ticketDate);
  }
}

const std::vector<AirlineCountrySettlementPlanInfo*>&
AirlineCountrySettlementPlanDAO::get(DeleteList& del,
                                     const NationCode& country,
                                     const CrsCode& gds,
                                     const CarrierCode& airline,
                                     const SettlementPlanType& spType,
                                     const DateTime& ticketDate)
{
  _codeCoverageGetCallCount++;

  AirlineCountrySettlementPlanKey acspKey(country, gds, airline, spType);
  DAOCache::pointer_type ptr = cache().get(acspKey);
  return *applyFilter(del,
                      ptr,
                      IsNotEffectiveG<AirlineCountrySettlementPlanInfo>(ticketDate));
}

const std::vector<AirlineCountrySettlementPlanInfo*>&
AirlineCountrySettlementPlanDAO::get(DeleteList& del,
                                     const NationCode& country,
                                     const CrsCode& gds,
                                     const SettlementPlanType& settlementPlan,
                                     const DateTime& date)
{
  _codeCoverageGetCallCount++;

  std::vector<AirlineCountrySettlementPlanInfo*>* acspList =
      new std::vector<AirlineCountrySettlementPlanInfo*>;

  std::shared_ptr<std::vector<AirlineCountrySettlementPlanKey>> keys = cache().keys();

  for (std::vector<AirlineCountrySettlementPlanKey>::const_iterator itKey = keys->begin();
       itKey != keys->end();
       ++itKey)
  {
    const AirlineCountrySettlementPlanKey& key = (*itKey);
    if ( (key._a == country) && (key._b == gds) && (key._d == settlementPlan) )
    {
      DAOCache::pointer_type ptr = cache().getIfResident(key);
      if (ptr)
      {
        del.copy(ptr);
        remove_copy_if(ptr->begin(),
                       ptr->end(),
                       back_inserter(*acspList),
                       IsNotEffectiveG<AirlineCountrySettlementPlanInfo>(date));
      }
    }
  }

  return *acspList;
}

const std::vector<AirlineCountrySettlementPlanInfo*>&
AirlineCountrySettlementPlanDAO::get(DeleteList& del,
                                     const CrsCode& gds,
                                     const NationCode& country,
                                     const CarrierCode& airline,
                                     const DateTime& date)
{
  _codeCoverageGetCallCount++;

  std::vector<AirlineCountrySettlementPlanInfo*>* acspList =
      new std::vector<AirlineCountrySettlementPlanInfo*>;

  std::shared_ptr<std::vector<AirlineCountrySettlementPlanKey>> keys = cache().keys();

  for (std::vector<AirlineCountrySettlementPlanKey>::const_iterator itKey = keys->begin();
       itKey != keys->end();
       ++itKey)
  {
    const AirlineCountrySettlementPlanKey& key = (*itKey);
    if ( (key._a == country) && (key._b == gds) && (key._c == airline) )
    {
      DAOCache::pointer_type ptr = cache().getIfResident(key);
      if (ptr)
      {
        del.copy(ptr);
        remove_copy_if(ptr->begin(),
                       ptr->end(),
                       back_inserter(*acspList),
                       IsNotEffectiveG<AirlineCountrySettlementPlanInfo>(date));
      }
    }
  }

  return *acspList;
}

std::vector<AirlineCountrySettlementPlanInfo*>*
AirlineCountrySettlementPlanDAO::create(AirlineCountrySettlementPlanKey acspKey)
{
  std::vector<AirlineCountrySettlementPlanInfo*>* acspList =
      new std::vector<AirlineCountrySettlementPlanInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAirlineCountrySettlementPlan acspQuery(dbAdapter->getAdapter());
    acspQuery.findAirlineCountrySettlementPlans(
        *acspList, acspKey._a, acspKey._b, acspKey._c, acspKey._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in AirlineCountrySettlementPlanDAO::create");
    destroyContainer(acspList);
    throw;
  }

  return acspList;
}

AirlineCountrySettlementPlanKey
AirlineCountrySettlementPlanDAO::createKey(const AirlineCountrySettlementPlanInfo* acsp)
{
  return AirlineCountrySettlementPlanKey(
      acsp->getCountryCode(), acsp->getGds(), acsp->getAirline(), acsp->getSettlementPlanType());
}

bool
AirlineCountrySettlementPlanDAO::translateKey(const ObjectKey& objectKey,
                                              AirlineCountrySettlementPlanKey& acspKey) const
{
  acspKey.initialized = objectKey.getValue("NATION", acspKey._a) &&
                        objectKey.getValue("GDS", acspKey._b) &&
                        objectKey.getValue("AIRLINE", acspKey._c) &&
                        objectKey.getValue("SETTLEMENTPLAN", acspKey._d);

  return acspKey.initialized;
}

void
AirlineCountrySettlementPlanDAO::translateKey(const AirlineCountrySettlementPlanKey& acspKey,
                                              ObjectKey& objectKey) const
{
  objectKey.setValue("NATION", acspKey._a);
  objectKey.setValue("GDS", acspKey._b);
  objectKey.setValue("AIRLINE", acspKey._c);
  objectKey.setValue("SETTLEMENTPLAN", acspKey._d);
}

bool
AirlineCountrySettlementPlanDAO::insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
{
  return DummyObjectInserter<AirlineCountrySettlementPlanInfo, AirlineCountrySettlementPlanDAO>(
             flatKey, objectKey).success();
}

void
AirlineCountrySettlementPlanDAO::destroy(AirlineCountrySettlementPlanKey key,
                                         std::vector<AirlineCountrySettlementPlanInfo*>* acspList)
{
  destroyContainer(acspList);
}

void
AirlineCountrySettlementPlanDAO::load()
{
  StartupLoader<QueryGetAllAirlineCountrySettlementPlans,
                AirlineCountrySettlementPlanInfo,
                AirlineCountrySettlementPlanDAO>();
}

size_t
AirlineCountrySettlementPlanDAO::clear()
{
  size_t result(cache().clear());
  load();
  LOG4CXX_ERROR(_logger, "AirlineCountrySettlementPlan cache cleared");
  return result;
}

std::string
AirlineCountrySettlementPlanDAO::_name("AirlineCountrySettlementPlan");
std::string
AirlineCountrySettlementPlanDAO::_cacheClass("Common");
DAOHelper<AirlineCountrySettlementPlanDAO>
AirlineCountrySettlementPlanDAO::_helper(_name);
AirlineCountrySettlementPlanDAO* AirlineCountrySettlementPlanDAO::_instance = nullptr;

// ----------------------------------------------------------------------------
// Historical DAO: AirlineCountrySettlementPlanHistoricalDAO
// ----------------------------------------------------------------------------

log4cxx::LoggerPtr
AirlineCountrySettlementPlanHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.AirlineCountrySettlementPlanHistoricalDAO"));

AirlineCountrySettlementPlanHistoricalDAO&
AirlineCountrySettlementPlanHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<AirlineCountrySettlementPlanInfo*>&
AirlineCountrySettlementPlanHistoricalDAO::get(DeleteList& del,
                                               const NationCode& country,
                                               const CrsCode& gds,
                                               const CarrierCode& airline,
                                               const SettlementPlanType& spType,
                                               const DateTime& ticketDate)
{
  _codeCoverageGetCallCount++;

  AirlineCountrySettlementPlanHistoricalKey acspKey(country, gds, airline, spType);
  DAOUtils::getDateRange(ticketDate, acspKey._e, acspKey._f, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(acspKey);
  del.copy(ptr);
  std::vector<AirlineCountrySettlementPlanInfo*>* acspList =
      new std::vector<AirlineCountrySettlementPlanInfo*>;
  del.adopt(acspList);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*acspList),
                 IsNotEffectiveH<AirlineCountrySettlementPlanInfo>(ticketDate, ticketDate));
  return *acspList;
}

std::vector<AirlineCountrySettlementPlanInfo*>*
AirlineCountrySettlementPlanHistoricalDAO::create(AirlineCountrySettlementPlanHistoricalKey acspKey)
{
  std::vector<AirlineCountrySettlementPlanInfo*>* acspList =
      new std::vector<AirlineCountrySettlementPlanInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAirlineCountrySettlementPlanHistorical acspQuery(dbAdapter->getAdapter());
    acspQuery.findAirlineCountrySettlementPlans(
        *acspList, acspKey._a, acspKey._b, acspKey._c, acspKey._d, acspKey._e, acspKey._f);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in AirlineCountrySettlementPlanistoricalDAO::create");
    destroyContainer(acspList);
    throw;
  }

  return acspList;
}

bool
AirlineCountrySettlementPlanHistoricalDAO::translateKey(
    const ObjectKey& objectKey,
    AirlineCountrySettlementPlanHistoricalKey& acspKey) const
{
  acspKey.initialized = objectKey.getValue("NATION", acspKey._a) &&
                        objectKey.getValue("GDS", acspKey._b) &&
                        objectKey.getValue("AIRLINE", acspKey._c) &&
                        objectKey.getValue("SETTLEMENTPLAN", acspKey._d);

  return acspKey.initialized;
}

void
AirlineCountrySettlementPlanHistoricalDAO::translateKey(
    const AirlineCountrySettlementPlanHistoricalKey& acspKey,
    ObjectKey& objectKey) const
{
  objectKey.setValue("NATION", acspKey._a);
  objectKey.setValue("GDS", acspKey._b);
  objectKey.setValue("AIRLINE", acspKey._c);
  objectKey.setValue("SETTLEMENTPLAN", acspKey._d);
}

bool
AirlineCountrySettlementPlanHistoricalDAO::translateKey(
    const ObjectKey& objectKey,
    AirlineCountrySettlementPlanHistoricalKey& acspKey,
    const DateTime ticketDate) const
{
  DAOUtils::getDateRange(ticketDate, acspKey._e, acspKey._f, _cacheBy);
  return translateKey(objectKey, acspKey);
}

void
AirlineCountrySettlementPlanHistoricalDAO::destroy(
    AirlineCountrySettlementPlanHistoricalKey key,
    std::vector<AirlineCountrySettlementPlanInfo*>* acspList)
{
  destroyContainer(acspList);
}


std::string AirlineCountrySettlementPlanHistoricalDAO::_name("AirlineCountrySettlementPlanHistorical");
std::string AirlineCountrySettlementPlanHistoricalDAO::_cacheClass("Common");
DAOHelper<AirlineCountrySettlementPlanHistoricalDAO>
AirlineCountrySettlementPlanHistoricalDAO::_helper(_name);
AirlineCountrySettlementPlanHistoricalDAO* AirlineCountrySettlementPlanHistoricalDAO::_instance = nullptr;

}
