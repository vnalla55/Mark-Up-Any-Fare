//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/FareCalcConfigDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/Queries/QueryGetFareCalcConfigs.h"

namespace tse
{
log4cxx::LoggerPtr
FareCalcConfigDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.FareCalcConfigDAO"));
FareCalcConfigDAO&
FareCalcConfigDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareCalcConfig*>&
getFareCalcConfigData(const Indicator& userApplType,
                      const UserApplCode& userAppl,
                      const PseudoCityCode& pseudoCity,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical)
{
  if (isHistorical)
  {
    FareCalcConfigHistoricalDAO& dao = FareCalcConfigHistoricalDAO::instance();
    return dao.get(deleteList, userApplType, userAppl, pseudoCity, ticketDate);
  }
  else
  {
    FareCalcConfigDAO& dao = FareCalcConfigDAO::instance();
    return dao.get(deleteList, userApplType, userAppl, pseudoCity, ticketDate);
  }
}

const std::vector<FareCalcConfig*>&
FareCalcConfigDAO::get(DeleteList& del,
                       const Indicator userApplType,
                       const UserApplCode& userAppl,
                       const PseudoCityCode& pseudoCity,
                       const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  std::vector<FareCalcConfig*>* recs = new std::vector<FareCalcConfig*>;
  del.adopt(recs);

  FareCalcConfigKey key(userApplType, userAppl, pseudoCity);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  copy(ptr->begin(), ptr->end(), back_inserter(*recs));

  if (pseudoCity != "")
  {
    FareCalcConfigKey key1(userApplType, userAppl, "");
    ptr = cache().get(key1);
    del.copy(ptr);
    copy(ptr->begin(), ptr->end(), back_inserter(*recs));
  }
  if (userApplType != ' ' || userAppl != "")
  {
    FareCalcConfigKey key2(' ', "", pseudoCity);
    ptr = cache().get(key2);
    del.copy(ptr);
    copy(ptr->begin(), ptr->end(), back_inserter(*recs));
    if (userApplType != ' ' || userAppl != "")
    {
      FareCalcConfigKey key3(' ', "", "");
      ptr = cache().get(key3);
      del.copy(ptr);
      copy(ptr->begin(), ptr->end(), back_inserter(*recs));
    }
  }

  return *recs;
}

void
FareCalcConfigDAO::load()
{
  StartupLoader<QueryGetAllFareCalcConfigs, FareCalcConfig, FareCalcConfigDAO>();
}

FareCalcConfigKey
FareCalcConfigDAO::createKey(FareCalcConfig* info)
{
  return FareCalcConfigKey(info->userApplType(), info->userAppl(), info->pseudoCity());
}

std::vector<FareCalcConfig*>*
FareCalcConfigDAO::create(FareCalcConfigKey key)
{
  std::vector<FareCalcConfig*>* ret = new std::vector<FareCalcConfig*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareCalcConfigs fcc(dbAdapter->getAdapter());
    fcc.findFareCalcConfigs(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareCalcConfigDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

sfc::CompressedData*
FareCalcConfigDAO::compress(const std::vector<FareCalcConfig*>* vect) const
{
  return compressVector(vect);
}

std::vector<FareCalcConfig*>*
FareCalcConfigDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<FareCalcConfig>(compressed);
}

void
FareCalcConfigDAO::destroy(FareCalcConfigKey key, std::vector<FareCalcConfig*>* recs)
{
  std::vector<FareCalcConfig*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

std::string
FareCalcConfigDAO::_name("FareCalcConfig");
std::string
FareCalcConfigDAO::_cacheClass("Common");
DAOHelper<FareCalcConfigDAO>
FareCalcConfigDAO::_helper(_name);
FareCalcConfigDAO* FareCalcConfigDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: FareCalcConfigHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
FareCalcConfigHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.FareCalcConfigHistoricalDAO"));
FareCalcConfigHistoricalDAO&
FareCalcConfigHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const std::vector<FareCalcConfig*>&
FareCalcConfigHistoricalDAO::get(DeleteList& del,
                                 const Indicator userApplType,
                                 const UserApplCode& userAppl,
                                 const PseudoCityCode& pseudoCity,
                                 const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  std::vector<FareCalcConfig*>* recs = new std::vector<FareCalcConfig*>;
  del.adopt(recs);

  FareCalcConfigKey key(userApplType, userAppl, pseudoCity);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  copy(ptr->begin(), ptr->end(), back_inserter(*recs));

  remove_copy_if(ptr->begin(),
                 ptr->end(),
                 back_inserter(*recs),
                 IsNotCurrentCreateDateOnly<FareCalcConfig>(ticketDate));

  if (pseudoCity != "")
  {
    FareCalcConfigKey key1(userApplType, userAppl, "");
    ptr = cache().get(key1);
    del.copy(ptr);
    copy(ptr->begin(), ptr->end(), back_inserter(*recs));
  }
  if (userApplType != ' ' || userAppl != "")
  {
    FareCalcConfigKey key2(' ', "", pseudoCity);
    ptr = cache().get(key2);
    del.copy(ptr);
    copy(ptr->begin(), ptr->end(), back_inserter(*recs));
    if (userApplType != ' ' || userAppl != "")
    {
      FareCalcConfigKey key3(' ', "", "");
      ptr = cache().get(key3);
      del.copy(ptr);
      copy(ptr->begin(), ptr->end(), back_inserter(*recs));
    }
  }

  return *recs;
}

std::vector<FareCalcConfig*>*
FareCalcConfigHistoricalDAO::create(FareCalcConfigKey key)
{
  std::vector<FareCalcConfig*>* ret = new std::vector<FareCalcConfig*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetFareCalcConfigs fcc(dbAdapter->getAdapter());
    fcc.findFareCalcConfigs(*ret, key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in FareCalcConfigHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
FareCalcConfigHistoricalDAO::destroy(FareCalcConfigKey key, std::vector<FareCalcConfig*>* recs)
{
  std::vector<FareCalcConfig*>::iterator i;
  for (i = recs->begin(); i != recs->end(); i++)
    delete *i;
  delete recs;
}

sfc::CompressedData*
FareCalcConfigHistoricalDAO::compress(const std::vector<FareCalcConfig*>* vect) const
{
  return compressVector(vect);
}

std::vector<FareCalcConfig*>*
FareCalcConfigHistoricalDAO::uncompress(const sfc::CompressedData& compressed) const
{
  return uncompressVectorPtr<FareCalcConfig>(compressed);
}

FareCalcConfigKey
FareCalcConfigHistoricalDAO::createKey(const FareCalcConfig* info,
                                       const DateTime& startDate,
                                       const DateTime& endDate)
{
  return FareCalcConfigKey(info->userApplType(), info->userAppl(), info->pseudoCity());
}

void
FareCalcConfigHistoricalDAO::load()
{
  StartupLoader<QueryGetAllFareCalcConfigs, FareCalcConfig, FareCalcConfigHistoricalDAO>();
}

std::string
FareCalcConfigHistoricalDAO::_name("FareCalcConfigHistorical");
std::string
FareCalcConfigHistoricalDAO::_cacheClass("Common");
DAOHelper<FareCalcConfigHistoricalDAO>
FareCalcConfigHistoricalDAO::_helper(_name);
FareCalcConfigHistoricalDAO* FareCalcConfigHistoricalDAO::_instance = nullptr;

} // namespace tse
