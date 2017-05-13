#include "DBAccess/SurfaceSectorExemptionDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetSurfaceSectorExemptionInfo.h"
#include "DBAccess/SurfaceSectorExemptionInfo.h"

#include <algorithm>
#include <functional>

namespace tse
{

log4cxx::LoggerPtr
SurfaceSectorExemptionDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SurfaceSectorExemptionDAO"));

std::string
SurfaceSectorExemptionDAO::_name("SurfaceSectorExemption");
std::string
SurfaceSectorExemptionDAO::_cacheClass("Common");

DAOHelper<SurfaceSectorExemptionDAO>
SurfaceSectorExemptionDAO::_helper(_name);
SurfaceSectorExemptionDAO* SurfaceSectorExemptionDAO::_instance;

const std::vector<SurfaceSectorExemptionInfo*>&
getSurfaceSectorExemptionInfoData(const CarrierCode& carrierCode,
                                  DeleteList& deleteList,
                                  const DateTime& ticketDate,
                                  bool isHistorical)
{
  if (isHistorical)
  {
    SurfaceSectorExemptionHistoricalDAO& dao = SurfaceSectorExemptionHistoricalDAO::instance();
    return dao.get(deleteList, carrierCode, ticketDate);
  }
  else
  {
    SurfaceSectorExemptionDAO& dao = SurfaceSectorExemptionDAO::instance();
    return dao.get(deleteList, carrierCode, ticketDate);
  }
}

const std::vector<SurfaceSectorExemptionInfo*>&
SurfaceSectorExemptionDAO::get(DeleteList& del,
                               const CarrierCode& validatingCarrier,
                               const DateTime& ticketDate)
{
  ++_codeCoverageGetAllCallCount;

  std::vector<SurfaceSectorExemptionInfo*>* recs = new std::vector<SurfaceSectorExemptionInfo*>();
  del.adopt(recs);

  CarrierKey key(validatingCarrier);
  CarrierKey keyEmpty("");
  DAOCache::pointer_type ptr = cache().get(key);
  DAOCache::pointer_type ptrEmpty = cache().get(keyEmpty);
  del.copy(ptr);
  del.copy(ptrEmpty);

  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*recs),
                 IsNotEffectiveG<SurfaceSectorExemptionInfo>(ticketDate));
  remove_copy_if(ptrEmpty->begin(),
                 ptrEmpty->end(),
                 back_inserter(*recs),
                 IsNotEffectiveG<SurfaceSectorExemptionInfo>(ticketDate));

  return *recs;
}

std::vector<SurfaceSectorExemptionInfo*>*
SurfaceSectorExemptionDAO::create(CarrierKey key)
{
  std::vector<SurfaceSectorExemptionInfo*>* result = new std::vector<SurfaceSectorExemptionInfo*>();

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());

  try
  {
    QueryGetSurfaceSectorExemptionInfo sseInfoQuery(dbAdapter->getAdapter());
    sseInfoQuery.findSurfaceSectorExemptionInfo(result, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SurfaceSectorExemptionDAO::create");
    destroyContainer(result);
    throw;
  }

  return result;
}

void
SurfaceSectorExemptionDAO::destroy(CarrierKey key, std::vector<SurfaceSectorExemptionInfo*>* recs)
{
  std::vector<SurfaceSectorExemptionInfo*>::iterator iter = recs->begin();
  std::vector<SurfaceSectorExemptionInfo*>::iterator endIter = recs->end();

  while (iter != endIter)
  {
    delete *iter;

    ++iter;
  }

  delete recs;
}

/*
 * Historical DAO
 */
log4cxx::LoggerPtr
SurfaceSectorExemptionHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.SurfaceSectorExemptionHistoricalDAO"));

std::string
SurfaceSectorExemptionHistoricalDAO::_name("SurfaceSectorExemptionHistorical");
std::string
SurfaceSectorExemptionHistoricalDAO::_cacheClass("Common");

DAOHelper<SurfaceSectorExemptionHistoricalDAO>
SurfaceSectorExemptionHistoricalDAO::_helper(_name);
SurfaceSectorExemptionHistoricalDAO* SurfaceSectorExemptionHistoricalDAO::_instance;

const std::vector<SurfaceSectorExemptionInfo*>&
SurfaceSectorExemptionHistoricalDAO::get(DeleteList& del,
                                         const CarrierCode& validatingCarrier,
                                         const DateTime& ticketDate)
{
  ++_codeCoverageGetAllCallCount;

  std::vector<SurfaceSectorExemptionInfo*>* recs = new std::vector<SurfaceSectorExemptionInfo*>();
  del.adopt(recs);

  CarrierKey key(validatingCarrier);
  CarrierKey keyEmpty("");
  DAOCache::pointer_type ptr = cache().get(key);
  DAOCache::pointer_type ptrEmpty = cache().get(keyEmpty);
  del.copy(ptr);
  del.copy(ptrEmpty);

  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*recs),
                 IsNotEffectiveH<SurfaceSectorExemptionInfo>(ticketDate, ticketDate));
  remove_copy_if(ptrEmpty->begin(),
                 ptrEmpty->end(),
                 back_inserter(*recs),
                 IsNotEffectiveH<SurfaceSectorExemptionInfo>(ticketDate, ticketDate));

  return *recs;
}

std::vector<SurfaceSectorExemptionInfo*>*
SurfaceSectorExemptionHistoricalDAO::create(CarrierKey key)
{
  std::vector<SurfaceSectorExemptionInfo*>* result = new std::vector<SurfaceSectorExemptionInfo*>();

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());

  try
  {
    QueryGetSurfaceSectorExemptionInfoHistorical sseInfoQuery(dbAdapter->getAdapter());
    sseInfoQuery.findSurfaceSectorExemptionInfo(result, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in SurfaceSectorExemptionHistoricalDAO::create");
    destroyContainer(result);
    throw;
  }

  return result;
}

void
SurfaceSectorExemptionHistoricalDAO::destroy(CarrierKey key,
                                             std::vector<SurfaceSectorExemptionInfo*>* recs)
{
  std::vector<SurfaceSectorExemptionInfo*>::iterator iter = recs->begin();
  std::vector<SurfaceSectorExemptionInfo*>::iterator endIter = recs->end();

  while (iter != endIter)
  {
    delete *iter;

    ++iter;
  }

  delete recs;
}
}
