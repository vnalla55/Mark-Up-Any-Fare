//-------------------------------------------------------------------------------
// (C) 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/AirlineInterlineAgreementDAO.h"

#include "Common/Logger.h"
#include "DBAccess/AirlineInterlineAgreementInfo.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DAOUtils.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/HistoricalDataAccessObject.h"
#include "DBAccess/Queries/QueryGetAirlineInterlineAgreement.h"

#include <algorithm>

namespace tse
{

log4cxx::LoggerPtr
AirlineInterlineAgreementDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.AirlineInterlineAgreementDAO"));

AirlineInterlineAgreementDAO&
AirlineInterlineAgreementDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<AirlineInterlineAgreementInfo*>&
getAirlineInterlineAgreementData(const NationCode& country,
                                 const CrsCode& gds,
                                 const CarrierCode& validatingCarrier,
                                 DeleteList& deleteList,
                                 const DateTime& ticketDate,
                                 bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    AirlineInterlineAgreementHistoricalDAO& dao = AirlineInterlineAgreementHistoricalDAO::instance();
    return dao.get(deleteList, country, gds, validatingCarrier, ticketDate);
  }
  else
  {
    AirlineInterlineAgreementDAO& dao = AirlineInterlineAgreementDAO::instance();
    return dao.get(deleteList, country, gds, validatingCarrier, ticketDate);
  }
}

const std::vector<AirlineInterlineAgreementInfo*>&
AirlineInterlineAgreementDAO::get(DeleteList& del,
                                  const NationCode& country,
                                  const CrsCode& gds,
                                  const CarrierCode& validatingCarrier,
                                  const DateTime& ticketDate)
{
  _codeCoverageGetCallCount++;

  AirlineInterlineAgreementKey aiaKey(country, gds, validatingCarrier);
  DAOCache::pointer_type ptr = cache().get(aiaKey);
  del.copy(ptr);
  return *applyFilter(del,
                      ptr,
                      IsNotEffectiveG<AirlineInterlineAgreementInfo>(ticketDate));
}

std::vector<AirlineInterlineAgreementInfo*>*
AirlineInterlineAgreementDAO::create(AirlineInterlineAgreementKey aiaKey)
{
  std::vector<AirlineInterlineAgreementInfo*>* aiaList =
      new std::vector<AirlineInterlineAgreementInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAirlineInterlineAgreement aiaQuery(dbAdapter->getAdapter());
    aiaQuery.findAirlineInterlineAgreement(*aiaList, aiaKey._a, aiaKey._b, aiaKey._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in AirlineInterlineAgreementDAO::create");
    destroyContainer(aiaList);
    throw;
  }

  return aiaList;
}

AirlineInterlineAgreementKey
AirlineInterlineAgreementDAO::createKey(const AirlineInterlineAgreementInfo* aia)
{
  return AirlineInterlineAgreementKey(
      aia->getCountryCode(), aia->getGds(), aia->getValidatingCarrier());
}

bool
AirlineInterlineAgreementDAO::translateKey(const ObjectKey& objectKey,
                                           AirlineInterlineAgreementKey& aiaKey) const
{
  aiaKey.initialized = objectKey.getValue("NATION", aiaKey._a) &&
                       objectKey.getValue("GDS", aiaKey._b) &&
                       objectKey.getValue("VALIDATINGCARRIER", aiaKey._c);

  return aiaKey.initialized;
}

void
AirlineInterlineAgreementDAO::translateKey(const AirlineInterlineAgreementKey& aiaKey,
                                           ObjectKey& objectKey) const
{
  objectKey.setValue("NATION", aiaKey._a);
  objectKey.setValue("GDS", aiaKey._b);
  objectKey.setValue("VALIDATINGCARRIER", aiaKey._c);
}

bool
AirlineInterlineAgreementDAO::insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
{
  return DummyObjectInserter<AirlineInterlineAgreementInfo, AirlineInterlineAgreementDAO>(
             flatKey, objectKey).success();
}

void
AirlineInterlineAgreementDAO::destroy(AirlineInterlineAgreementKey key,
                                      std::vector<AirlineInterlineAgreementInfo*>* aiaList)
{
  destroyContainer(aiaList);
}

void
AirlineInterlineAgreementDAO::load()
{
  StartupLoaderNoDB<AirlineInterlineAgreementInfo, AirlineInterlineAgreementDAO>();
}

size_t
AirlineInterlineAgreementDAO::clear()
{
  size_t result(cache().clear());
  LOG4CXX_ERROR(_logger, "AirlineInterlineAgreement cache cleared");
  return result;
}

std::string
AirlineInterlineAgreementDAO::_name("AirlineInterlineAgreement");
std::string
AirlineInterlineAgreementDAO::_cacheClass("Common");
DAOHelper<AirlineInterlineAgreementDAO>
AirlineInterlineAgreementDAO::_helper(_name);
AirlineInterlineAgreementDAO* AirlineInterlineAgreementDAO::_instance = nullptr;

//-----------------------------------------------------------------------------
// HISTORICAL
//-----------------------------------------------------------------------------

log4cxx::LoggerPtr
AirlineInterlineAgreementHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.AirlineInterlineAgreementHistoricalDAO"));

AirlineInterlineAgreementHistoricalDAO&
AirlineInterlineAgreementHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<AirlineInterlineAgreementInfo*>&
AirlineInterlineAgreementHistoricalDAO::get(DeleteList& del,
                                            const NationCode& country,
                                            const CrsCode& gds,
                                            const CarrierCode& validatingCarrier,
                                            const DateTime& ticketDate)
{
  _codeCoverageGetCallCount++;

  AirlineInterlineAgreementHistoricalKey aiaKey(country, gds, validatingCarrier);
  DAOUtils::getDateRange(ticketDate, aiaKey._d, aiaKey._e, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(aiaKey);
  del.copy(ptr);

  std::vector<AirlineInterlineAgreementInfo*>* aiaList =
      new std::vector<AirlineInterlineAgreementInfo*>;
  del.adopt(aiaList);

  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*aiaList),
                 IsNotEffectiveH<AirlineInterlineAgreementInfo>(ticketDate, ticketDate));
  return *aiaList;
}

std::vector<AirlineInterlineAgreementInfo*>*
AirlineInterlineAgreementHistoricalDAO::create(AirlineInterlineAgreementHistoricalKey aiaKey)
{
  std::vector<AirlineInterlineAgreementInfo*>* aiaList =
      new std::vector<AirlineInterlineAgreementInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAirlineInterlineAgreementHistorical aiaQuery(dbAdapter->getAdapter());
    aiaQuery.findAirlineInterlineAgreement(
        *aiaList, aiaKey._a, aiaKey._b, aiaKey._c, aiaKey._d, aiaKey._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in AirlineInterlineAgreementHistoricalDAO::create");
    destroyContainer(aiaList);
    throw;
  }

  return aiaList;
}

bool
AirlineInterlineAgreementHistoricalDAO::translateKey(const ObjectKey& objectKey,
                                                     AirlineInterlineAgreementHistoricalKey& aiaKey) const
{
  aiaKey.initialized = objectKey.getValue("NATION", aiaKey._a) &&
                       objectKey.getValue("GDS", aiaKey._b) &&
                       objectKey.getValue("VALIDATINGCARRIER", aiaKey._c);

  return aiaKey.initialized;
}

void
AirlineInterlineAgreementHistoricalDAO::translateKey(const AirlineInterlineAgreementHistoricalKey& aiaKey,
                                                     ObjectKey& objectKey) const
{
  objectKey.setValue("NATION", aiaKey._a);
  objectKey.setValue("GDS", aiaKey._b);
  objectKey.setValue("VALIDATINGCARRIER", aiaKey._c);
}

bool
AirlineInterlineAgreementHistoricalDAO::translateKey(const ObjectKey& objectKey,
                                                     AirlineInterlineAgreementHistoricalKey& aiaKey,
                                                     const DateTime ticketDate) const
{
  DAOUtils::getDateRange(ticketDate, aiaKey._d, aiaKey._e, _cacheBy);
  return translateKey(objectKey, aiaKey);
}

void
AirlineInterlineAgreementHistoricalDAO::destroy(AirlineInterlineAgreementHistoricalKey key,
                                                std::vector<AirlineInterlineAgreementInfo*>* aiaList)
{
  destroyContainer(aiaList);
}

std::string
AirlineInterlineAgreementHistoricalDAO::_name("AirlineInterlineAgreementHistorical");
std::string
AirlineInterlineAgreementHistoricalDAO::_cacheClass("Common");
DAOHelper<AirlineInterlineAgreementHistoricalDAO>
AirlineInterlineAgreementHistoricalDAO::_helper(_name);
AirlineInterlineAgreementHistoricalDAO* AirlineInterlineAgreementHistoricalDAO::_instance = nullptr;

}
