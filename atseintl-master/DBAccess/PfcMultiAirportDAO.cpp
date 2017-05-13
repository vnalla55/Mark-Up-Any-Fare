//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/PfcMultiAirportDAO.h"

#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/PfcMultiAirport.h"
#include "DBAccess/Queries/QueryGetPfcMultiAirport.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
PfcMultiAirportDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.PfcMultiAirportDAO"));

// dao fix
size_t
PfcMultiAirportDAO::clear()
{
  size_t result(cache().clear());
  //DISKCACHE.remove(time(NULL), true, _name, DISKCACHE_ALL);
  load();
  return result;
}

size_t
PfcMultiAirportDAO::invalidate(const ObjectKey& objectKey)
{
  return clear();
}

PfcMultiAirportDAO&
PfcMultiAirportDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const PfcMultiAirport*
getPfcMultiAirportData(const LocCode& key,
                       const DateTime& date,
                       DeleteList& deleteList,
                       const DateTime& ticketDate,
                       bool isHistorical)
{
  if (isHistorical)
  {
    PfcMultiAirportHistoricalDAO& dao = PfcMultiAirportHistoricalDAO::instance();
    return dao.get(deleteList, key, date, ticketDate);
  }
  else
  {
    PfcMultiAirportDAO& dao = PfcMultiAirportDAO::instance();
    return dao.get(deleteList, key, date, ticketDate);
  }
}

const PfcMultiAirport*
PfcMultiAirportDAO::get(DeleteList& del,
                        const LocCode& key,
                        const DateTime& date,
                        const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(LocCodeKey(key));
  del.copy(ptr);
  PfcMultiAirport* ret = nullptr;
  DAOCache::value_type::iterator i;

  IsEffectiveNotInhibit<IsEffectiveG, PfcMultiAirport> isApplicable(date, ticketDate);
  i = find_if(ptr->begin(), ptr->end(), isApplicable);

  if (i != ptr->end())
    ret = *i;
  return ret;
}

const std::vector<PfcMultiAirport*>&
getAllPfcMultiAirportData(DeleteList& del)
{

  PfcMultiAirportDAO& dao = PfcMultiAirportDAO::instance();
  return dao.getAll(del);
}

const std::vector<PfcMultiAirport*>&
PfcMultiAirportDAO::getAll(DeleteList& del)
{
  // Track calls for code coverage
  _codeCoverageGetAllCallCount++;

  std::shared_ptr<std::vector<LocCodeKey>> keys = cache().keys();

  std::vector<PfcMultiAirport*>* ret = new std::vector<PfcMultiAirport*>;

  for (auto& elem : *keys)
  {
    DAOCache::pointer_type ptr = cache().get(elem);
    del.copy(ptr);
    std::copy(ptr->begin(), ptr->end(), std::back_inserter(*ret));
  }

  del.adopt(ret);

  return *ret;
}

LocCodeKey
PfcMultiAirportDAO::createKey(PfcMultiAirport* info)
{
  return LocCodeKey(info->loc().loc());
}

void
PfcMultiAirportDAO::load()
{
  StartupLoader<QueryGetAllPfcMultiAirport, PfcMultiAirport, PfcMultiAirportDAO>();
}

std::vector<PfcMultiAirport*>*
PfcMultiAirportDAO::create(LocCodeKey key)
{
  std::vector<PfcMultiAirport*>* ret = new std::vector<PfcMultiAirport*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetPfcMultiAirport pma(dbAdapter->getAdapter());
    pma.findPfcMultiAirport(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in PfcMultiAirportDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
PfcMultiAirportDAO::destroy(LocCodeKey key, std::vector<PfcMultiAirport*>* recs)
{
  destroyContainer(recs);
}

std::string
PfcMultiAirportDAO::_name("PfcMultiAirport");
std::string
PfcMultiAirportDAO::_cacheClass("Taxes");

DAOHelper<PfcMultiAirportDAO>
PfcMultiAirportDAO::_helper(_name);

PfcMultiAirportDAO* PfcMultiAirportDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: PfcMultiAirportHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
PfcMultiAirportHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.PfcMultiAirportHistoricalDAO"));
PfcMultiAirportHistoricalDAO&
PfcMultiAirportHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const PfcMultiAirport*
PfcMultiAirportHistoricalDAO::get(DeleteList& del,
                                  const LocCode& key,
                                  const DateTime& date,
                                  const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  PfcMultiAirportHistoricalKey cacheKey(key);
  DAOUtils::getDateRange(ticketDate, cacheKey._b, cacheKey._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(cacheKey);
  del.copy(ptr);
  PfcMultiAirport* ret = nullptr;
  DAOCache::value_type::iterator i;

  IsEffectiveNotInhibit<IsEffectiveHist, PfcMultiAirport> isApplicable(date, ticketDate);
  i = find_if(ptr->begin(), ptr->end(), isApplicable);

  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<PfcMultiAirport*>*
PfcMultiAirportHistoricalDAO::create(PfcMultiAirportHistoricalKey key)
{
  std::vector<PfcMultiAirport*>* ret = new std::vector<PfcMultiAirport*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetPfcMultiAirportHistorical pma(dbAdapter->getAdapter());
    pma.findPfcMultiAirport(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in PfcMultiAirportHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
PfcMultiAirportHistoricalDAO::destroy(PfcMultiAirportHistoricalKey key,
                                      std::vector<PfcMultiAirport*>* recs)
{
  std::vector<PfcMultiAirport*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
PfcMultiAirportHistoricalDAO::_name("PfcMultiAirportHistorical");
std::string
PfcMultiAirportHistoricalDAO::_cacheClass("Taxes");
DAOHelper<PfcMultiAirportHistoricalDAO>
PfcMultiAirportHistoricalDAO::_helper(_name);
PfcMultiAirportHistoricalDAO* PfcMultiAirportHistoricalDAO::_instance = nullptr;

} // namespace tse
