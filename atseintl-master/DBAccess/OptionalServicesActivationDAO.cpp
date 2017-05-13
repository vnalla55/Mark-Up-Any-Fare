//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/OptionalServicesActivationDAO.h"

#include "DBAccess/OptionalServicesActivationInfo.h"
#include "DBAccess/Queries/QueryGetOptionalServicesActivation.h"

namespace tse
{
log4cxx::LoggerPtr
OptionalServicesActivationDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.OptionalServicesActivationDAO"));

std::string
OptionalServicesActivationDAO::_name("OptionalServicesActivation");
std::string
OptionalServicesActivationDAO::_cacheClass("Rules");
DAOHelper<OptionalServicesActivationDAO>
OptionalServicesActivationDAO::_helper(_name);
OptionalServicesActivationDAO* OptionalServicesActivationDAO::_instance = nullptr;

OptionalServicesActivationDAO&
OptionalServicesActivationDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<OptionalServicesActivationInfo*>&
getOptServiceActivationData(Indicator crs,
                            const UserApplCode& userCode,
                            const std::string& application,
                            DeleteList& deleteList,
                            const DateTime& ticketDate,
                            bool isHistorical)
{
  if (isHistorical)
  {
    OptionalServicesActivationHistoricalDAO& dao =
        OptionalServicesActivationHistoricalDAO::instance();
    return dao.get(deleteList, crs, userCode, application, ticketDate);
  }
  else
  {
    OptionalServicesActivationDAO& dao = OptionalServicesActivationDAO::instance();
    return dao.get(deleteList, crs, userCode, application, ticketDate);
  }
}

const std::vector<OptionalServicesActivationInfo*>&
OptionalServicesActivationDAO::get(DeleteList& del,
                                   Indicator crs,
                                   const UserApplCode& userCode,
                                   const std::string& application,
                                   const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  OptServActivationKey key(crs, userCode, application);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<OptionalServicesActivationInfo*>* ret =
      new std::vector<OptionalServicesActivationInfo*>;
  del.adopt(ret);

  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotCurrentG<OptionalServicesActivationInfo>(ticketDate));

  return *ret;
}

std::vector<OptionalServicesActivationInfo*>*
OptionalServicesActivationDAO::create(OptServActivationKey key)
{
  std::vector<OptionalServicesActivationInfo*>* ret =
      new std::vector<OptionalServicesActivationInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetOptionalServicesActivation fnc(dbAdapter->getAdapter());
    fnc.findOptionalServicesActivationInfo(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in OptionalServicesActivationDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
OptionalServicesActivationDAO::destroy(OptServActivationKey key,
                                       std::vector<OptionalServicesActivationInfo*>* recs)
{
  destroyContainer(recs);
}

OptServActivationKey
OptionalServicesActivationDAO::createKey(OptionalServicesActivationInfo* info)
{
  return OptServActivationKey(info->userApplType(), info->userAppl(), info->application());
}

void
OptionalServicesActivationDAO::load()
{
  // not pre_loading
  StartupLoaderNoDB<OptionalServicesActivationInfo, OptionalServicesActivationDAO>();
}

///////////////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////////////
log4cxx::LoggerPtr
OptionalServicesActivationHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.OptionalServicesActivationHistoricalDAO"));

std::string
OptionalServicesActivationHistoricalDAO::_name("OptionalServicesActivationHistorical");
std::string
OptionalServicesActivationHistoricalDAO::_cacheClass("Rules");
DAOHelper<OptionalServicesActivationHistoricalDAO>
OptionalServicesActivationHistoricalDAO::_helper(_name);
OptionalServicesActivationHistoricalDAO* OptionalServicesActivationHistoricalDAO::_instance = nullptr;

OptionalServicesActivationHistoricalDAO&
OptionalServicesActivationHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<OptionalServicesActivationInfo*>&
OptionalServicesActivationHistoricalDAO::get(DeleteList& del,
                                             Indicator crs,
                                             const UserApplCode& userCode,
                                             const std::string& application,
                                             const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  OptServActivationHistoricalKey key(crs, userCode, application);
  DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  std::vector<OptionalServicesActivationInfo*>* ret =
      new std::vector<OptionalServicesActivationInfo*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotCurrentH<OptionalServicesActivationInfo>(ticketDate));

  return *ret;
}

std::vector<OptionalServicesActivationInfo*>*
OptionalServicesActivationHistoricalDAO::create(OptServActivationHistoricalKey key)
{
  std::vector<OptionalServicesActivationInfo*>* ret =
      new std::vector<OptionalServicesActivationInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetOptionalServicesActivationHistorical fnc(dbAdapter->getAdapter());
    fnc.findOptionalServicesActivationInfo(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in OptionalServicesActivationHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
OptionalServicesActivationHistoricalDAO::destroy(OptServActivationHistoricalKey key,
                                                 std::vector<OptionalServicesActivationInfo*>* recs)
{
  std::vector<OptionalServicesActivationInfo*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}
} // tse
