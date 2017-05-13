//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/MileageSubstitutionDAO.h"

#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/MileageSubstitution.h"
#include "DBAccess/Queries/QueryGetMileageSubstitution.h"

#include <algorithm>
#include <functional>

namespace tse
{
log4cxx::LoggerPtr
MileageSubstitutionDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.MileageSubstitutionDAO"));

MileageSubstitutionDAO&
MileageSubstitutionDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const MileageSubstitution*
getMileageSubstitutionData(const LocCode& key,
                           const DateTime& date,
                           DeleteList& deleteList,
                           const DateTime& ticketDate,
                           bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    MileageSubstitutionHistoricalDAO& dao = MileageSubstitutionHistoricalDAO::instance();
    return dao.get(deleteList, key, date, ticketDate);
  }
  else
  {
    MileageSubstitutionDAO& dao = MileageSubstitutionDAO::instance();
    return dao.get(deleteList, key, date, ticketDate);
  }
}

const MileageSubstitution*
MileageSubstitutionDAO::get(DeleteList& del,
                            const LocCode& key,
                            const DateTime& date,
                            const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(LocCodeKey(key));
  del.copy(ptr);
  MileageSubstitution* ret = nullptr;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveG<MileageSubstitution>(date, ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

void
MileageSubstitutionDAO::load()
{
  StartupLoader<QueryGetAllMileageSubstitution, MileageSubstitution, MileageSubstitutionDAO>();
}

LocCodeKey
MileageSubstitutionDAO::createKey(MileageSubstitution* info)
{
  return LocCodeKey(info->substitutionLoc());
}

std::vector<MileageSubstitution*>*
MileageSubstitutionDAO::create(LocCodeKey key)
{
  std::vector<MileageSubstitution*>* ret = new std::vector<MileageSubstitution*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMileageSubstitution ms(dbAdapter->getAdapter());
    ms.findMileageSubstitution(*ret, key._a);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MileageSubstitutionDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MileageSubstitutionDAO::destroy(LocCodeKey key, std::vector<MileageSubstitution*>* recs)
{
  destroyContainer(recs);
}

sfc::CompressedData*
MileageSubstitutionDAO::compress(const std::vector<MileageSubstitution*>* vect) const
{
  return compressVector(vect);
}

std::vector<MileageSubstitution*>*
MileageSubstitutionDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MileageSubstitution>(compressed);
}

std::string
MileageSubstitutionDAO::_name("MileageSubstitution");
std::string
MileageSubstitutionDAO::_cacheClass("Routing");
DAOHelper<MileageSubstitutionDAO>
MileageSubstitutionDAO::_helper(_name);
MileageSubstitutionDAO* MileageSubstitutionDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: MileageSubstitutionHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
MileageSubstitutionHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.MileageSubstitutionHistoricalDAO"));
MileageSubstitutionHistoricalDAO&
MileageSubstitutionHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const MileageSubstitution*
MileageSubstitutionHistoricalDAO::get(DeleteList& del,
                                      const LocCode& key,
                                      const DateTime& date,
                                      const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  MileageSubstitution* ret = nullptr;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveHist<MileageSubstitution>(date, ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

std::vector<MileageSubstitution*>*
MileageSubstitutionHistoricalDAO::create(LocCode key)
{
  std::vector<MileageSubstitution*>* ret = new std::vector<MileageSubstitution*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetMileageSubstitutionHistorical ms(dbAdapter->getAdapter());
    ms.findMileageSubstitution(*ret, key);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MileageSubstitutionHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
MileageSubstitutionHistoricalDAO::destroy(LocCode key, std::vector<MileageSubstitution*>* recs)
{
  std::vector<MileageSubstitution*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

struct MileageSubstitutionHistoricalDAO::groupByKey
{
public:
  LocCode prevKey;

  DAOCache& cache;

  groupByKey()
    : prevKey(INVALID_LOCCODE), cache(MileageSubstitutionHistoricalDAO::instance().cache()), ptr(nullptr)
  {
  }

  std::vector<MileageSubstitution*>* ptr;

  void operator()(MileageSubstitution* info)
  {
    if (info->substitutionLoc() != prevKey)
    {
      ptr = new std::vector<MileageSubstitution*>;
      cache.put(info->substitutionLoc(), ptr);
      prevKey = info->substitutionLoc();
    }
    ptr->push_back(info);
  }
};

void
MileageSubstitutionHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<MileageSubstitution*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetAllMileageSubstitutionHistorical ms(dbAdapter->getAdapter());
    ms.findAllMileageSubstitution(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in MileageSubstitutionHistoricalDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::for_each(recs.begin(), recs.end(), groupByKey());
}

sfc::CompressedData*
MileageSubstitutionHistoricalDAO::compress(const std::vector<MileageSubstitution*>* vect) const
{
  return compressVector(vect);
}

std::vector<MileageSubstitution*>*
MileageSubstitutionHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<MileageSubstitution>(compressed);
}

std::string
MileageSubstitutionHistoricalDAO::_name("MileageSubstitutionHistorical");
std::string
MileageSubstitutionHistoricalDAO::_cacheClass("Routing");
DAOHelper<MileageSubstitutionHistoricalDAO>
MileageSubstitutionHistoricalDAO::_helper(_name);

MileageSubstitutionHistoricalDAO* MileageSubstitutionHistoricalDAO::_instance = nullptr;

} // namespace tse
