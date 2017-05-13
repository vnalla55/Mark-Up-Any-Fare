//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc. Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/ServiceFeesCxrActivationDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DAOUtils.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DBHistoryServer.h"
#include "DBAccess/Queries/QueryGetServiceFeesCxrActivation.h"
#include "DBAccess/ServiceFeesCxrActivation.h"

namespace tse
{
log4cxx::LoggerPtr
ServiceFeesCxrActivationDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.ServiceFeesCxrActivationDAO"));

ServiceFeesCxrActivationDAO&
ServiceFeesCxrActivationDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<ServiceFeesCxrActivation*>&
getServiceFeesCxrActivationData(const CarrierCode& validatingCarrier,
                                const DateTime& date,
                                DeleteList& deleteList,
                                const DateTime& ticketDate,
                                bool isHistorical)
{
  if (isHistorical)
  {
    ServiceFeesCxrActivationHistoricalDAO& dao = ServiceFeesCxrActivationHistoricalDAO::instance();

    const std::vector<ServiceFeesCxrActivation*>& ret =
        dao.get(deleteList, validatingCarrier, date, ticketDate);

    return ret;
  }
  else
  {
    ServiceFeesCxrActivationDAO& dao = ServiceFeesCxrActivationDAO::instance();

    const std::vector<ServiceFeesCxrActivation*>& ret =
        dao.get(deleteList, validatingCarrier, date, ticketDate);

    return ret;
  }
}

const std::vector<ServiceFeesCxrActivation*>&
ServiceFeesCxrActivationDAO::get(DeleteList& del,
                                 const CarrierCode& validatingCarrier,
                                 const DateTime& date,
                                 const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  ServiceFeesCxrActivationKey key(validatingCarrier);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<ServiceFeesCxrActivation*>* ret = new std::vector<ServiceFeesCxrActivation*>;
  del.adopt(ret);

  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveG<ServiceFeesCxrActivation>(date, ticketDate));

  return *ret;
}

std::vector<ServiceFeesCxrActivation*>*
ServiceFeesCxrActivationDAO::create(ServiceFeesCxrActivationKey key)
{
  std::vector<ServiceFeesCxrActivation*>* ret = new std::vector<ServiceFeesCxrActivation*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetServiceFeesCxrActivation fnc(dbAdapter->getAdapter());
    fnc.findServiceFeesCxrActivation(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in ServiceFeesCxrActivationDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
ServiceFeesCxrActivationDAO::destroy(ServiceFeesCxrActivationKey key,
                                     std::vector<ServiceFeesCxrActivation*>* recs)
{
  destroyContainer(recs);
}

ServiceFeesCxrActivationKey
ServiceFeesCxrActivationDAO::createKey(ServiceFeesCxrActivation* info)
{
  return ServiceFeesCxrActivationKey(info->carrier());
}

void
ServiceFeesCxrActivationDAO::load()
{
  // not pre_loading

  StartupLoaderNoDB<ServiceFeesCxrActivation, ServiceFeesCxrActivationDAO>();
}

std::string
ServiceFeesCxrActivationDAO::_name("ServiceFeesCxrActivation");
std::string
ServiceFeesCxrActivationDAO::_cacheClass("Rules");
DAOHelper<ServiceFeesCxrActivationDAO>
ServiceFeesCxrActivationDAO::_helper(_name);
ServiceFeesCxrActivationDAO* ServiceFeesCxrActivationDAO::_instance = nullptr;

///////////////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////////////
log4cxx::LoggerPtr
ServiceFeesCxrActivationHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.ServiceFeesCxrActivationHistoricalDAO"));
ServiceFeesCxrActivationHistoricalDAO&
ServiceFeesCxrActivationHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<ServiceFeesCxrActivation*>&
ServiceFeesCxrActivationHistoricalDAO::get(DeleteList& del,
                                           const CarrierCode& carrier,
                                           const DateTime& date,
                                           const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  ServiceFeesCxrActivationHistoricalKey key(carrier);
  DAOUtils::getDateRange(ticketDate, key._b, key._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<ServiceFeesCxrActivation*>* ret = new std::vector<ServiceFeesCxrActivation*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveH<ServiceFeesCxrActivation>(date, ticketDate));

  return *ret;
}

std::vector<ServiceFeesCxrActivation*>*
ServiceFeesCxrActivationHistoricalDAO::create(ServiceFeesCxrActivationHistoricalKey key)
{
  std::vector<ServiceFeesCxrActivation*>* ret = new std::vector<ServiceFeesCxrActivation*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetServiceFeesCxrActivationHistorical fnc(dbAdapter->getAdapter());
    fnc.findServiceFeesCxrActivation(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in ServiceFeesCxrActivationHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
ServiceFeesCxrActivationHistoricalDAO::destroy(ServiceFeesCxrActivationHistoricalKey key,
                                               std::vector<ServiceFeesCxrActivation*>* recs)
{
  std::vector<ServiceFeesCxrActivation*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
ServiceFeesCxrActivationHistoricalDAO::_name("ServiceFeesCxrActivationHistorical");
std::string
ServiceFeesCxrActivationHistoricalDAO::_cacheClass("Rules");
DAOHelper<ServiceFeesCxrActivationHistoricalDAO>
ServiceFeesCxrActivationHistoricalDAO::_helper(_name);
ServiceFeesCxrActivationHistoricalDAO* ServiceFeesCxrActivationHistoricalDAO::_instance = nullptr;

} // namespace tse
