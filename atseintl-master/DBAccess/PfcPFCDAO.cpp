//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/PfcPFCDAO.h"

#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/PfcPFC.h"
#include "DBAccess/Queries/QueryGetPFC.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
PfcPFCDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.PfcPFCDAO"));

// dao fix
size_t
PfcPFCDAO::clear()
{
  size_t result(cache().clear());
  //DISKCACHE.remove(time(NULL), true, _name, DISKCACHE_ALL);
  load();
  return result;
}

size_t
PfcPFCDAO::invalidate(const ObjectKey& objectKey)
{
  return clear();
}

PfcPFCDAO&
PfcPFCDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const PfcPFC*
getPfcPFCData(const LocCode& key,
              const DateTime& date,
              DeleteList& deleteList,
              const DateTime& ticketDate,
              bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    PfcPFCHistoricalDAO& dao = PfcPFCHistoricalDAO::instance();
    return dao.get(deleteList, key, date, ticketDate);
  }
  else
  {
    PfcPFCDAO& dao = PfcPFCDAO::instance();
    return dao.get(deleteList, key, date, ticketDate);
  }
}

const PfcPFC*
PfcPFCDAO::get(DeleteList& del,
               const LocCode& key,
               const DateTime& date,
               const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(LocCodeKey(key));
  del.copy(ptr);
  PfcPFC* ret = nullptr;
  DAOCache::value_type::iterator i;

  IsEffectiveNotInhibit<IsEffectiveG, PfcPFC> isApplicable(date, ticketDate);
  i = find_if(ptr->begin(), ptr->end(), isApplicable);

  if (i != ptr->end())
    ret = *i;
  return ret;
}

const std::vector<PfcPFC*>&
getAllPfcPFCData(DeleteList& del)
{

  PfcPFCDAO& dao = PfcPFCDAO::instance();
  return dao.getAll(del);
}

const std::vector<PfcPFC*>&
PfcPFCDAO::getAll(DeleteList& del)
{
  // Track calls for code coverage
  _codeCoverageGetAllCallCount++;

  std::shared_ptr<std::vector<LocCodeKey>> keys = cache().keys();

  std::vector<PfcPFC*>* ret = new std::vector<PfcPFC*>;

  for (auto& elem : *keys)
  {
    DAOCache::pointer_type ptr = cache().get(elem);
    del.copy(ptr);
    std::copy(ptr->begin(), ptr->end(), std::back_inserter(*ret));
  }

  del.adopt(ret);

  return *ret;
}

void
PfcPFCDAO::load()
{
  StartupLoader<QueryGetAllPFC, PfcPFC, PfcPFCDAO>();
}

LocCodeKey
PfcPFCDAO::createKey(PfcPFC* info)
{
  return LocCodeKey(info->pfcAirport());
}

std::vector<PfcPFC*>*
PfcPFCDAO::create(LocCodeKey key)
{
  std::vector<PfcPFC*>* ret = new std::vector<PfcPFC*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetPFC pfc(dbAdapter->getAdapter());
    pfc.findPfcPFC(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in PfcPFCDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
PfcPFCDAO::destroy(LocCodeKey key, std::vector<PfcPFC*>* recs)
{
  destroyContainer(recs);
}

sfc::CompressedData*
PfcPFCDAO::compress(const std::vector<PfcPFC*>* vect) const
{
  return compressVector(vect);
}

std::vector<PfcPFC*>*
PfcPFCDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<PfcPFC>(compressed);
}

std::string
PfcPFCDAO::_name("PfcPFC");
std::string
PfcPFCDAO::_cacheClass("Taxes");

DAOHelper<PfcPFCDAO>
PfcPFCDAO::_helper(_name);

PfcPFCDAO* PfcPFCDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: PfcPFCHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
PfcPFCHistoricalDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.PfcPFCHistoricalDAO"));
PfcPFCHistoricalDAO&
PfcPFCHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const PfcPFC*
PfcPFCHistoricalDAO::get(DeleteList& del,
                         const LocCode& key,
                         const DateTime& date,
                         const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  PfcPFCHistoricalKey cacheKey(key);
  DAOUtils::getDateRange(ticketDate, cacheKey._b, cacheKey._c, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(cacheKey);
  del.copy(ptr);
  PfcPFC* ret = nullptr;
  DAOCache::value_type::iterator i;

  IsEffectiveNotInhibit<IsEffectiveHist, PfcPFC> isApplicable(date, ticketDate);
  i = find_if(ptr->begin(), ptr->end(), isApplicable);

  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<PfcPFC*>*
PfcPFCHistoricalDAO::create(PfcPFCHistoricalKey key)
{
  std::vector<PfcPFC*>* ret = new std::vector<PfcPFC*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetPFCHistorical pfc(dbAdapter->getAdapter());
    pfc.findPfcPFC(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in PfcPFCHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
PfcPFCHistoricalDAO::destroy(PfcPFCHistoricalKey key, std::vector<PfcPFC*>* recs)
{
  std::vector<PfcPFC*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
PfcPFCHistoricalDAO::compress(const std::vector<PfcPFC*>* vect) const
{
  return compressVector(vect);
}

std::vector<PfcPFC*>*
PfcPFCHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<PfcPFC>(compressed);
}

std::string
PfcPFCHistoricalDAO::_name("PfcPFCHistorical");
std::string
PfcPFCHistoricalDAO::_cacheClass("Taxes");
DAOHelper<PfcPFCHistoricalDAO>
PfcPFCHistoricalDAO::_helper(_name);
PfcPFCHistoricalDAO* PfcPFCHistoricalDAO::_instance = nullptr;

} // namespace tse
