//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/PfcCollectMethDAO.h"

#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/PfcCollectMeth.h"
#include "DBAccess/Queries/QueryGetPfcCollectMeth.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
PfcCollectMethDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.PfcCollectMethDAO"));

PfcCollectMethDAO&
PfcCollectMethDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<PfcCollectMeth*>&
getAllPfcCollectMethData(DeleteList& del)
{
  PfcCollectMethDAO& dao = PfcCollectMethDAO::instance();
  return dao.getAll(del);
}

const std::vector<PfcCollectMeth*>&
PfcCollectMethDAO::getAll(DeleteList& del)
{

  std::vector<PfcCollectMeth*>* pfcolmetV = new std::vector<PfcCollectMeth*>;
  del.adopt(pfcolmetV);

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());

  QueryGetAllPfcCollectMeth pfc(dbAdapter->getAdapter());
  pfc.findAllPfcCollectMeth(*pfcolmetV);

  return *pfcolmetV;
}

const std::vector<PfcCollectMeth*>&
getPfcCollectMethData(const CarrierCode& carrier,
                      const DateTime& date,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical)
{
  if (isHistorical)
  {
    PfcCollectMethHistoricalDAO& dao = PfcCollectMethHistoricalDAO::instance();
    return dao.get(deleteList, carrier, date, ticketDate);
  }
  else
  {
    PfcCollectMethDAO& dao = PfcCollectMethDAO::instance();
    return dao.get(deleteList, carrier, date, ticketDate);
  }
}

const std::vector<PfcCollectMeth*>&
PfcCollectMethDAO::get(DeleteList& del,
                       const CarrierCode& key,
                       const DateTime& date,
                       const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(CarrierKey(key));
  /*
  del.copy(ptr);
  std::vector<PfcCollectMeth*>* ret = new std::vector<PfcCollectMeth*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(), ptr->end(), back_inserter(*ret),
  IsNotEffectiveG<PfcCollectMeth>(date, ticketDate));
  return *ret;
  */
  return *(applyFilter(del, ptr, IsNotEffectiveG<PfcCollectMeth>(date, ticketDate)));
}

void
PfcCollectMethDAO::load()
{
  StartupLoader<QueryGetAllPfcCollectMeth, PfcCollectMeth, PfcCollectMethDAO>();
}

CarrierKey
PfcCollectMethDAO::createKey(PfcCollectMeth* info)
{
  return CarrierKey(info->carrier());
}

std::vector<PfcCollectMeth*>*
PfcCollectMethDAO::create(CarrierKey key)
{
  std::vector<PfcCollectMeth*>* ret = new std::vector<PfcCollectMeth*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetPfcCollectMeth pcm(dbAdapter->getAdapter());
    pcm.findPfcCollectMeth(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in PfcCollectMethDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
PfcCollectMethDAO::destroy(CarrierKey key, std::vector<PfcCollectMeth*>* recs)
{
  destroyContainer(recs);
}

std::string
PfcCollectMethDAO::_name("PfcCollectMeth");
std::string
PfcCollectMethDAO::_cacheClass("Taxes");

DAOHelper<PfcCollectMethDAO>
PfcCollectMethDAO::_helper(_name);

PfcCollectMethDAO* PfcCollectMethDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: PfcCollectMethHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
PfcCollectMethHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.PfcCollectMethHistoricalDAO"));
PfcCollectMethHistoricalDAO&
PfcCollectMethHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<PfcCollectMeth*>&
PfcCollectMethHistoricalDAO::get(DeleteList& del,
                                 const CarrierCode& key,
                                 const DateTime& date,
                                 const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  PfcCollectMethHistoricalKey cacheKey(key);
  DAOUtils::getDateRange(ticketDate, cacheKey._b, cacheKey._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(cacheKey);
  del.copy(ptr);
  std::vector<PfcCollectMeth*>* ret = new std::vector<PfcCollectMeth*>;
  del.adopt(ret);
  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*ret),
                 IsNotEffectiveHist<PfcCollectMeth>(date, ticketDate));
  return *ret;
}

std::vector<PfcCollectMeth*>*
PfcCollectMethHistoricalDAO::create(PfcCollectMethHistoricalKey key)
{
  std::vector<PfcCollectMeth*>* ret = new std::vector<PfcCollectMeth*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetPfcCollectMethHistorical pcm(dbAdapter->getAdapter());
    pcm.findPfcCollectMeth(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in PfcCollectMethHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

void
PfcCollectMethHistoricalDAO::destroy(PfcCollectMethHistoricalKey key,
                                     std::vector<PfcCollectMeth*>* recs)
{
  std::vector<PfcCollectMeth*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
PfcCollectMethHistoricalDAO::_name("PfcCollectMethHistorical");
std::string
PfcCollectMethHistoricalDAO::_cacheClass("Taxes");
DAOHelper<PfcCollectMethHistoricalDAO>
PfcCollectMethHistoricalDAO::_helper(_name);

PfcCollectMethHistoricalDAO* PfcCollectMethHistoricalDAO::_instance = nullptr;

} // namespace tse
