//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/ServiceGroupDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DBHistoryServer.h"
#include "DBAccess/Queries/QueryGetServiceGroup.h"
#include "DBAccess/ServiceGroupInfo.h"

namespace tse
{
log4cxx::LoggerPtr
ServiceGroupDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.ServiceGroupDAO"));

ServiceGroupDAO&
ServiceGroupDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<ServiceGroupInfo*>&
getAllServiceGroupData(DeleteList& deleteList, bool isHistorical)
{
  if (isHistorical)
    return ServiceGroupHistoricalDAO::instance().getAll(deleteList);
  else
    return ServiceGroupDAO::instance().getAll(deleteList);
}

const std::vector<ServiceGroupInfo*>&
ServiceGroupDAO::getAll(DeleteList& del)
{
  // Track calls for code coverage
  _codeCoverageGetAllCallCount++;

  DAOCache::pointer_type ptr = cache().get(ServiceGroupKey(""));
  del.copy(ptr);
  return *ptr;
}

std::vector<ServiceGroupInfo*>*
ServiceGroupDAO::create(ServiceGroupKey key)
{
  std::vector<ServiceGroupInfo*>* ret = new std::vector<ServiceGroupInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllServiceGroup qasgh(dbAdapter->getAdapter());
    qasgh.findAllServiceGroup(*ret);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in ServiceGroupDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
ServiceGroupDAO::destroy(ServiceGroupKey key, std::vector<ServiceGroupInfo*>* recs)
{
  destroyContainer(recs);
}

ServiceGroupKey
ServiceGroupDAO::createKey(ServiceGroupInfo* info)
{
  return ServiceGroupKey("");
}

void
ServiceGroupDAO::load()
{
  // StartupLoader<QueryGetServiceGroup,ServiceGroupInfo,ServiceGroupDAO>() ;
  // Not pre_loading
  StartupLoaderNoDB<ServiceGroupInfo, ServiceGroupDAO>();
}

bool
ServiceGroupDAO::insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
{
  ServiceGroupInfo* info(new ServiceGroupInfo);
  ServiceGroupInfo::dummyData(*info);
  DAOCache::pointer_type ptr = cache().get(ServiceGroupKey(""));
  bool alreadyExists(false);

  for (std::vector<ServiceGroupInfo*>::const_iterator bit = ptr->begin(); bit != ptr->end(); ++bit)
  {
    const ServiceGroupInfo* thisGroup((*bit));
    if (thisGroup->svcGroup() == info->svcGroup())
    {
      alreadyExists = true;
      break;
    }
  }

  if (alreadyExists)
  {
    delete info;
  }
  else
  {
    ptr->push_back(info);
    cache().getCacheImpl()->queueDiskPut(ServiceGroupKey(""), true);
  }

  return true;
}

std::string
ServiceGroupDAO::_name("ServiceGroup");
std::string
ServiceGroupDAO::_cacheClass("Rules");
DAOHelper<ServiceGroupDAO>
ServiceGroupDAO::_helper(_name);
ServiceGroupDAO* ServiceGroupDAO::_instance = nullptr;

///////////////////////////////////////////////////////
// Historical DAO Object
///////////////////////////////////////////////////////
log4cxx::LoggerPtr
ServiceGroupHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.ServiceGroupHistDAO"));
ServiceGroupHistoricalDAO&
ServiceGroupHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<ServiceGroupInfo*>&
ServiceGroupHistoricalDAO::getAll(DeleteList& del)
{
  // Track calls for code coverage
  _codeCoverageGetAllCallCount++;

  DAOCache::pointer_type ptr = cache().get(ServiceGroupHistoricalKey(""));
  del.copy(ptr);
  return *ptr;
}

std::vector<ServiceGroupInfo*>*
ServiceGroupHistoricalDAO::create(ServiceGroupHistoricalKey key)
{
  std::vector<ServiceGroupInfo*>* ret = new std::vector<ServiceGroupInfo*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllServiceGroupHistorical qasgh(dbAdapter->getAdapter());
    qasgh.findAllServiceGroup(*ret);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in ServiceGroupDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
ServiceGroupHistoricalDAO::destroy(ServiceGroupHistoricalKey key,
                                   std::vector<ServiceGroupInfo*>* recs)
{
  destroyContainer(recs);
}

std::string
ServiceGroupHistoricalDAO::_name("ServiceGroupHistorical");
std::string
ServiceGroupHistoricalDAO::_cacheClass("Rules");
DAOHelper<ServiceGroupHistoricalDAO>
ServiceGroupHistoricalDAO::_helper(_name);
ServiceGroupHistoricalDAO* ServiceGroupHistoricalDAO::_instance = nullptr;

} // namespace tse
