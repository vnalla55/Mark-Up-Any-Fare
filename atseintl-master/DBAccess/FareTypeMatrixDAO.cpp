//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "DBAccess/FareTypeMatrixDAO.h"

#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareTypeMatrix.h"
#include "DBAccess/Queries/QueryGetFareTypeMatrixs.h"

#include <algorithm>
#include <functional>

namespace
{
template <typename T>
struct Filter
{
  Filter(const T& effectiveFilter, const tse::FareType& key)
    : _effectiveFilter(effectiveFilter), _key(key)
  {
  }
  bool operator()(const tse::FareTypeMatrix* rec) const
  {
    return _key == rec->fareType() && _effectiveFilter(rec);
  }
  const T& _effectiveFilter;
  const tse::FareType& _key;
};
}
namespace tse
{
log4cxx::LoggerPtr
FareTypeMatrixDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FareTypeMatrixDAO"));

FareTypeMatrixDAO&
FareTypeMatrixDAO::instance()
{
  if (UNLIKELY(_instance == nullptr))
  {
    _helper.init();
  }
  return *_instance;
}

const FareTypeMatrix*
getFareTypeMatrixData(const FareType& key,
                      const DateTime& date,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical)
{
  if (UNLIKELY(isHistorical))
  {
    FareTypeMatrixHistoricalDAO& dao = FareTypeMatrixHistoricalDAO::instance();
    return dao.get(deleteList, key, date, ticketDate);
  }
  else
  {
    FareTypeMatrixDAO& dao = FareTypeMatrixDAO::instance();
    return dao.get(deleteList, key, date, ticketDate);
  }
}

const FareTypeMatrix*
FareTypeMatrixDAO::get(DeleteList& del,
                       const FareType& key,
                       const DateTime& date,
                       const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  IsEffectiveG<FareTypeMatrix> isEffective(date, ticketDate);

  DAOCache::pointer_type ptr = cache().get(IntKey(0));
  del.copy(ptr);
  Filter<IsEffectiveG<FareTypeMatrix> > filter(isEffective, key);
  DAOCache::value_type::const_iterator it(find_if(ptr->begin(), ptr->end(), filter));
  if (it != ptr->end())
  {
    return *it;
  }
  return nullptr;
}

const std::vector<FareTypeMatrix*>&
getAllFareTypeMatrixData(const DateTime& date, DeleteList& deleteList)
{
  FareTypeMatrixDAO& dao = FareTypeMatrixDAO::instance();
  return dao.getAll(deleteList, date);
}

const std::vector<FareTypeMatrix*>&
FareTypeMatrixDAO::getAll(DeleteList& del, const DateTime& date)
{
  // Track calls for code coverage
  ++_codeCoverageGetAllCallCount;

  std::vector<FareTypeMatrix*>* ret = new std::vector<FareTypeMatrix*>();
  del.adopt(ret);
  DAOCache::pointer_type ptr = cache().get(IntKey(0));
  del.copy(ptr);
  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotCurrentG<FareTypeMatrix>(date));
  return *ret;
}

IntKey
FareTypeMatrixDAO::createKey(FareTypeMatrix* info)
{
  return IntKey(0);
}

bool
FareTypeMatrixDAO::insertDummyObject(std::string& flatKey, ObjectKey& objectKey)
{
  FareTypeMatrix* info(new FareTypeMatrix);
  FareTypeMatrix::dummyData(*info);
  DAOCache::pointer_type ptr = cache().get(IntKey(0));
  bool alreadyExists(false);

  for (std::vector<FareTypeMatrix*>::const_iterator bit = ptr->begin(); bit != ptr->end(); ++bit)
  {
    const FareTypeMatrix* thisFTM((*bit));
    if (thisFTM->fareType() == info->fareType())
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
    cache().getCacheImpl()->queueDiskPut(IntKey(0), true);
  }

  return true;
}

std::vector<FareTypeMatrix*>*
FareTypeMatrixDAO::create(IntKey key)
{
  std::vector<FareTypeMatrix*>* ret = new std::vector<FareTypeMatrix*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareTypeMatrixs ftm(dbAdapter->getAdapter());
    ftm.findAllFareTypeMatrix(*ret);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareTypeMatrixDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

void
FareTypeMatrixDAO::destroy(IntKey key, std::vector<FareTypeMatrix*>* recs)
{
  destroyContainer(recs);
}

void
FareTypeMatrixDAO::load()
{
  //---REMOVE----
  // StartupLoader<QueryGetFareTypeMatrixs,FareTypeMatrix,FareTypeMatrixDAO>() ;
  //-------------

  std::vector<FareTypeMatrix*>* recs = new std::vector<FareTypeMatrix*>;
  Loader<QueryGetFareTypeMatrixs, FareTypeMatrixDAO, std::vector<FareTypeMatrix*> > loader(*recs);
  if (loader.successful() && (!loader.gotFromLDC()))
  {
    cache().put(IntKey(0), recs);
  }
  else
  {
    destroyContainer(recs);
  }
}

std::string
FareTypeMatrixDAO::_name("FareTypeMatrix");
std::string
FareTypeMatrixDAO::_cacheClass("Common");

DAOHelper<FareTypeMatrixDAO>
FareTypeMatrixDAO::_helper(_name);

FareTypeMatrixDAO* FareTypeMatrixDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: FareTypeMatrixHistoricalDAO
// --------------------------------------------------

log4cxx::LoggerPtr
FareTypeMatrixHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareTypeMatrixHistoricalDAO"));
FareTypeMatrixHistoricalDAO&
FareTypeMatrixHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const FareTypeMatrix*
FareTypeMatrixHistoricalDAO::get(DeleteList& del,
                                 const FareType& key,
                                 const DateTime& date,
                                 const DateTime& ticketDate)
{
  // Track calls for code coverage
  ++_codeCoverageGetCallCount;

  IsEffectiveHist<FareTypeMatrix> isEffective(date, ticketDate);

  DAOCache::pointer_type ptr = cache().get(dummy);
  del.copy(ptr);
  Filter<IsEffectiveHist<FareTypeMatrix> > filter(isEffective, key);
  DAOCache::value_type::const_iterator it(find_if(ptr->begin(), ptr->end(), filter));
  if (it != ptr->end())
  {
    return *it;
  }
  return nullptr;
}

const std::vector<FareTypeMatrix*>&
FareTypeMatrixHistoricalDAO::getAll(DeleteList& del, const DateTime& date)
{
  // Track calls for code coverage
  ++_codeCoverageGetAllCallCount;

  std::vector<FareTypeMatrix*>* ret = new std::vector<FareTypeMatrix*>();
  del.adopt(ret);
  DAOCache::pointer_type ptr = cache().get(dummy);
  del.copy(ptr);
  remove_copy_if(
      ptr->begin(), ptr->end(), back_inserter(*ret), IsNotCurrentH<FareTypeMatrix>(date));
  return *ret;
}

void
FareTypeMatrixHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<FareTypeMatrix*>* ret = new std::vector<FareTypeMatrix*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareTypeMatrixsHistorical ftm(dbAdapter->getAdapter());
    ftm.findAllFareTypeMatrix(*ret);
    cache().put(dummy, ret);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareTypeMatrixHistoricalDAO::load");
    destroyContainer(ret);
    throw;
  }
}

std::vector<FareTypeMatrix*>*
FareTypeMatrixHistoricalDAO::create(int key)
{
  std::vector<FareTypeMatrix*>* ret = new std::vector<FareTypeMatrix*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareTypeMatrixsHistorical ftm(dbAdapter->getAdapter());
    ftm.findAllFareTypeMatrix(*ret);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareTypeMatrixHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }
  return ret;
}

void
FareTypeMatrixHistoricalDAO::destroy(int key, std::vector<FareTypeMatrix*>* recs)
{
  destroyContainer(recs);
}

std::string
FareTypeMatrixHistoricalDAO::_name("FareTypeMatrixHistorical");
std::string
FareTypeMatrixHistoricalDAO::_cacheClass("Common");
const int FareTypeMatrixHistoricalDAO::dummy;
DAOHelper<FareTypeMatrixHistoricalDAO>
FareTypeMatrixHistoricalDAO::_helper(_name);

FareTypeMatrixHistoricalDAO* FareTypeMatrixHistoricalDAO::_instance = nullptr;

} // namespace tse
