//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/NationStateChkDAO.h"

#include "Common/Logger.h"
#include "Common/TseWrappers.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/NationStateHistIsCurrChk.h"
#include "DBAccess/Queries/QueryCheckNationAndStateLocs.h"

#include <string>

#include <time.h>

namespace tse
{
/////////////////////////////////// NationInAreaDAO ///////////////////////////////////
log4cxx::LoggerPtr
NationInAreaDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.NationInAreaDAO"));

NationInAreaDAO&
NationInAreaDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

bool
isNationInAreaData(const NationCode& nation,
                   const LocCode& area,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical)
{
  if (isHistorical)
  {
    NationInAreaHistoricalDAO& dao = NationInAreaHistoricalDAO::instance();
    return dao.isNationInArea(nation, area, deleteList, ticketDate);
  }
  else
  {
    NationInAreaDAO& dao = NationInAreaDAO::instance();
    return dao.isNationInArea(nation, area);
  }
}

bool
NationInAreaDAO::isNationInArea(const NationCode& nation, const LocCode& area)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  NationInAreaKey key(nation, area);
  return *(cache().get(key));
}

NationInAreaInfo*
NationInAreaDAO::create(NationInAreaKey key)
{
  NationInAreaInfo* inArea = new NationInAreaInfo;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryCheckNationInArea na(dbAdapter->getAdapter());
    *inArea = na.isNationInArea(key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in NationInAreaDAO::create");
    delete inArea;
    throw;
  }

  return inArea;
}

void
NationInAreaDAO::destroy(NationInAreaKey key, NationInAreaInfo* rec)
{
  delete rec;
}

std::string
NationInAreaDAO::_name("NationInArea");
std::string
NationInAreaDAO::_cacheClass("Common");
DAOHelper<NationInAreaDAO>
NationInAreaDAO::_helper(_name);
NationInAreaDAO* NationInAreaDAO::_instance = nullptr;

/////////////////////////////////// NationInAreaHistoricalDAO ///////////////////////////////////
log4cxx::LoggerPtr
NationInAreaHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.NationInAreaHistoricalDAO"));
NationInAreaHistoricalDAO&
NationInAreaHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

bool
NationInAreaHistoricalDAO::isNationInArea(const NationCode& nation,
                                          const LocCode& area,
                                          DeleteList& del,
                                          const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  NationInAreaHistoricalKey key(nation, area);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsCurrentH<NationStateHistIsCurrChk>(ticketDate));
  if (i == ptr->end())
    return false;

  if (((*i)->inclExclInd()) == 'I')
    return true;

  return false;
}

std::vector<NationStateHistIsCurrChk*>*
NationInAreaHistoricalDAO::create(NationInAreaHistoricalKey key)
{
  std::vector<NationStateHistIsCurrChk*>* ret = new std::vector<NationStateHistIsCurrChk*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryCheckNationInAreaHistorical na(dbAdapter->getAdapter());
    na.findNationInArea(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in NationInAreaHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
NationInAreaHistoricalDAO::destroy(NationInAreaHistoricalKey key,
                                   std::vector<NationStateHistIsCurrChk*>* recs)
{
  destroyContainer(recs);
}

std::string
NationInAreaHistoricalDAO::_name("NationInAreaHistorical");
std::string
NationInAreaHistoricalDAO::_cacheClass("Common");
DAOHelper<NationInAreaHistoricalDAO>
NationInAreaHistoricalDAO::_helper(_name);
NationInAreaHistoricalDAO* NationInAreaHistoricalDAO::_instance = nullptr;

/////////////////////////////////// NationInSubAreaDAO ///////////////////////////////////
log4cxx::LoggerPtr
NationInSubAreaDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.NationInSubAreaDAO"));
NationInSubAreaDAO&
NationInSubAreaDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

bool
isNationInSubAreaData(const NationCode& nation,
                      const LocCode& subArea,
                      DeleteList& deleteList,
                      const DateTime& ticketDate,
                      bool isHistorical)
{
  if (isHistorical)
  {
    NationInSubAreaHistoricalDAO& dao = NationInSubAreaHistoricalDAO::instance();
    return dao.isNationInSubArea(nation, subArea, deleteList, ticketDate);
  }
  else
  {
    NationInSubAreaDAO& dao = NationInSubAreaDAO::instance();
    return dao.isNationInSubArea(nation, subArea);
  }
}

bool
NationInSubAreaDAO::isNationInSubArea(const NationCode& nation, const LocCode& subArea)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  NationInSubAreaKey key(nation, subArea);
  return *(cache().get(key));
}

NationInAreaInfo*
NationInSubAreaDAO::create(NationInSubAreaKey key)
{
  NationInAreaInfo* inSubArea = new NationInAreaInfo;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryCheckNationInSubArea nsa(dbAdapter->getAdapter());
    *inSubArea = nsa.isNationInSubArea(key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in NationInSubAreaDAO::create");
    delete inSubArea;
    throw;
  }

  return inSubArea;
}

void
NationInSubAreaDAO::destroy(NationInSubAreaKey key, NationInAreaInfo* rec)
{
  delete rec;
}

std::string
NationInSubAreaDAO::_name("NationInSubArea");
std::string
NationInSubAreaDAO::_cacheClass("Common");
DAOHelper<NationInSubAreaDAO>
NationInSubAreaDAO::_helper(_name);
NationInSubAreaDAO* NationInSubAreaDAO::_instance = nullptr;

/////////////////////////////////// NationInSubAreaHistoricalDAO ///////////////////////////////////
log4cxx::LoggerPtr
NationInSubAreaHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.NationInSubAreaHistoricalDAO"));
NationInSubAreaHistoricalDAO&
NationInSubAreaHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

bool
NationInSubAreaHistoricalDAO::isNationInSubArea(const NationCode& nation,
                                                const LocCode& subArea,
                                                DeleteList& del,
                                                const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  NationInSubAreaHistoricalKey key(nation, subArea);
  DAOUtils::getDateRange(ticketDate, key._c, key._d, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsCurrentH<NationStateHistIsCurrChk>(ticketDate));
  if (i == ptr->end())
    return false;

  if (((*i)->inclExclInd()) == 'I')
    return true;

  return false;
}

std::vector<NationStateHistIsCurrChk*>*
NationInSubAreaHistoricalDAO::create(NationInSubAreaHistoricalKey key)
{
  std::vector<NationStateHistIsCurrChk*>* ret = new std::vector<NationStateHistIsCurrChk*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryCheckNationInSubAreaHistorical nsa(dbAdapter->getAdapter());
    nsa.findNationInSubArea(*ret, key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in NationInSubAreaHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
NationInSubAreaHistoricalDAO::destroy(NationInSubAreaHistoricalKey key,
                                      std::vector<NationStateHistIsCurrChk*>* recs)
{
  destroyContainer(recs);
}

std::string
NationInSubAreaHistoricalDAO::_name("NationInSubAreaHistorical");
std::string
NationInSubAreaHistoricalDAO::_cacheClass("Common");
DAOHelper<NationInSubAreaHistoricalDAO>
NationInSubAreaHistoricalDAO::_helper(_name);
NationInSubAreaHistoricalDAO* NationInSubAreaHistoricalDAO::_instance = nullptr;

/////////////////////////////////// NationInZoneDAO ///////////////////////////////////
log4cxx::LoggerPtr
NationInZoneDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.NationInZoneDAO"));

NationInZoneDAO&
NationInZoneDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

bool
isNationInZoneData(const VendorCode& vendor,
                   int zone,
                   char zoneType,
                   const NationCode& nation,
                   DeleteList& deleteList,
                   const DateTime& ticketDate,
                   bool isHistorical)
{
  if (isHistorical)
  {
    NationInZoneHistoricalDAO& dao = NationInZoneHistoricalDAO::instance();
    return dao.isNationInZone(vendor, zone, zoneType, nation, deleteList, ticketDate);
  }
  else
  {
    NationInZoneDAO& dao = NationInZoneDAO::instance();
    return dao.isNationInZone(vendor, zone, zoneType, nation);
  }
}

bool
NationInZoneDAO::isNationInZone(const VendorCode& vendor,
                                int zone,
                                char zoneType,
                                const NationCode& nation)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  NationInZoneKey key(vendor, zone, zoneType, nation);
  return *(cache().get(key));
}

NationInAreaInfo*
NationInZoneDAO::create(NationInZoneKey key)
{
  NationInAreaInfo* inZone = new NationInAreaInfo;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryCheckNationInZone nz(dbAdapter->getAdapter());
    *inZone = nz.isNationInZone(key._a, key._b, key._c, key._d);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in NationInZoneDAO::create");
    delete inZone;
    throw;
  }

  return inZone;
}

void
NationInZoneDAO::destroy(NationInZoneKey key, NationInAreaInfo* rec)
{
  delete rec;
}

std::string
NationInZoneDAO::_name("NationInZone");
std::string
NationInZoneDAO::_cacheClass("Common");
DAOHelper<NationInZoneDAO>
NationInZoneDAO::_helper(_name);
NationInZoneDAO* NationInZoneDAO::_instance = nullptr;

/////////////////////////////////// NationInZoneHistoricalDAO ///////////////////////////////////
log4cxx::LoggerPtr
NationInZoneHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.NationInZoneHistoricalDAO"));
NationInZoneHistoricalDAO&
NationInZoneHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

bool
NationInZoneHistoricalDAO::isNationInZone(const VendorCode& vendor,
                                          int zone,
                                          char zoneType,
                                          const NationCode& nation,
                                          DeleteList& del,
                                          const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  NationInZoneHistoricalKey key(vendor, zone, zoneType, nation);
  DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsCurrentH<NationStateHistIsCurrChk>(ticketDate));
  if (i == ptr->end())
    return false;

  if (((*i)->inclExclInd()) == 'I')
    return true;

  return false;
}

std::vector<NationStateHistIsCurrChk*>*
NationInZoneHistoricalDAO::create(NationInZoneHistoricalKey key)
{
  std::vector<NationStateHistIsCurrChk*>* ret = new std::vector<NationStateHistIsCurrChk*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryCheckNationInZoneHistorical nzh(dbAdapter->getAdapter());
    nzh.findNationInZone(*ret, key._a, key._b, key._c, key._d, key._e, key._f);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in NationInZoneHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
NationInZoneHistoricalDAO::destroy(NationInZoneHistoricalKey key,
                                   std::vector<NationStateHistIsCurrChk*>* recs)
{
  destroyContainer(recs);
}

std::string
NationInZoneHistoricalDAO::_name("NationInZoneHistorical");
std::string
NationInZoneHistoricalDAO::_cacheClass("Common");
DAOHelper<NationInZoneHistoricalDAO>
NationInZoneHistoricalDAO::_helper(_name);
NationInZoneHistoricalDAO* NationInZoneHistoricalDAO::_instance = nullptr;

//////////////////////////////////// State Stuff /////////////////////////////////////
/////////////////////////////////// StateInAreaDAO ///////////////////////////////////
log4cxx::LoggerPtr
StateInAreaDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.StateInAreaDAO"));

StateInAreaDAO&
StateInAreaDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

bool
isStateInAreaData(const NationCode& nation,
                  const StateCode& state,
                  const LocCode& area,
                  DeleteList& deleteList,
                  const DateTime& ticketDate,
                  bool isHistorical)
{
  if (isHistorical)
  {
    StateInAreaHistoricalDAO& dao = StateInAreaHistoricalDAO::instance();
    return dao.isStateInArea(nation, state, area, deleteList, ticketDate);
  }
  else
  {
    StateInAreaDAO& dao = StateInAreaDAO::instance();
    return dao.isStateInArea(nation, state, area);
  }
}

bool
StateInAreaDAO::isStateInArea(const NationCode& nation, const StateCode& state, const LocCode& area)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  StateInAreaKey key(nation, state, area);
  return *(cache().get(key));
}

NationInAreaInfo*
StateInAreaDAO::create(StateInAreaKey key)
{
  NationInAreaInfo* inArea = new NationInAreaInfo;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryCheckStateInArea na(dbAdapter->getAdapter());
    *inArea = na.isStateInArea(key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in StateInAreaDAO::create");
    delete inArea;
    throw;
  }

  return inArea;
}

void
StateInAreaDAO::destroy(StateInAreaKey key, NationInAreaInfo* rec)
{
  delete rec;
}

std::string
StateInAreaDAO::_name("StateInArea");
std::string
StateInAreaDAO::_cacheClass("Common");
DAOHelper<StateInAreaDAO>
StateInAreaDAO::_helper(_name);
StateInAreaDAO* StateInAreaDAO::_instance = nullptr;

/////////////////////////////////// StateInAreaHistoricalDAO ///////////////////////////////////
log4cxx::LoggerPtr
StateInAreaHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.StateInAreaHistoricalDAO"));
StateInAreaHistoricalDAO&
StateInAreaHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

bool
StateInAreaHistoricalDAO::isStateInArea(const NationCode& nation,
                                        const StateCode& state,
                                        const LocCode& area,
                                        DeleteList& del,
                                        const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  StateInAreaHistoricalKey key(nation, state, area);
  DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsCurrentH<NationStateHistIsCurrChk>(ticketDate));
  if (i == ptr->end())
    return false;

  if (((*i)->inclExclInd()) == 'I')
    return true;

  return false;
}

std::vector<NationStateHistIsCurrChk*>*
StateInAreaHistoricalDAO::create(StateInAreaHistoricalKey key)
{
  std::vector<NationStateHistIsCurrChk*>* ret = new std::vector<NationStateHistIsCurrChk*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryCheckStateInAreaHistorical na(dbAdapter->getAdapter());
    na.findStateInArea(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in StateInAreaHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
StateInAreaHistoricalDAO::destroy(StateInAreaHistoricalKey key,
                                  std::vector<NationStateHistIsCurrChk*>* recs)
{
  destroyContainer(recs);
}

std::string
StateInAreaHistoricalDAO::_name("StateInAreaHistorical");
std::string
StateInAreaHistoricalDAO::_cacheClass("Common");
DAOHelper<StateInAreaHistoricalDAO>
StateInAreaHistoricalDAO::_helper(_name);
StateInAreaHistoricalDAO* StateInAreaHistoricalDAO::_instance = nullptr;

/////////////////////////////////// StateInSubAreaDAO ///////////////////////////////////
log4cxx::LoggerPtr
StateInSubAreaDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.StateInSubAreaDAO"));

StateInSubAreaDAO&
StateInSubAreaDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

bool
isStateInSubAreaData(const NationCode& nation,
                     const StateCode& state,
                     const LocCode& subArea,
                     DeleteList& deleteList,
                     const DateTime& ticketDate,
                     bool isHistorical)
{
  if (isHistorical)
  {
    StateInSubAreaHistoricalDAO& dao = StateInSubAreaHistoricalDAO::instance();
    return dao.isStateInSubArea(nation, state, subArea, deleteList, ticketDate);
  }
  else
  {
    StateInSubAreaDAO& dao = StateInSubAreaDAO::instance();
    return dao.isStateInSubArea(nation, state, subArea);
  }
}

bool
StateInSubAreaDAO::isStateInSubArea(const NationCode& nation,
                                    const StateCode& state,
                                    const LocCode& subArea)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  StateInSubAreaKey key(nation, state, subArea);
  return *(cache().get(key));
}

NationInAreaInfo*
StateInSubAreaDAO::create(StateInSubAreaKey key)
{
  NationInAreaInfo* inSubArea = new NationInAreaInfo;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryCheckStateInSubArea ssa(dbAdapter->getAdapter());
    *inSubArea = ssa.isStateInSubArea(key._a, key._b, key._c);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in StateInSubAreaDAO::create");
    delete inSubArea;
    throw;
  }

  return inSubArea;
}

void
StateInSubAreaDAO::destroy(StateInSubAreaKey key, NationInAreaInfo* rec)
{
  delete rec;
}

std::string
StateInSubAreaDAO::_name("StateInSubArea");
std::string
StateInSubAreaDAO::_cacheClass("Common");
DAOHelper<StateInSubAreaDAO>
StateInSubAreaDAO::_helper(_name);
StateInSubAreaDAO* StateInSubAreaDAO::_instance = nullptr;

/////////////////////////////////// StateInSubAreaHistoricalDAO ///////////////////////////////////
log4cxx::LoggerPtr
StateInSubAreaHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.StateInSubAreaHistoricalDAO"));
StateInSubAreaHistoricalDAO&
StateInSubAreaHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

bool
StateInSubAreaHistoricalDAO::isStateInSubArea(const NationCode& nation,
                                              const StateCode& state,
                                              const LocCode& subArea,
                                              DeleteList& del,
                                              const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  StateInSubAreaHistoricalKey key(nation, state, subArea);
  DAOUtils::getDateRange(ticketDate, key._d, key._e, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsCurrentH<NationStateHistIsCurrChk>(ticketDate));
  if (i == ptr->end())
    return false;

  if (((*i)->inclExclInd()) == 'I')
    return true;

  return false;
}

std::vector<NationStateHistIsCurrChk*>*
StateInSubAreaHistoricalDAO::create(StateInSubAreaHistoricalKey key)
{
  std::vector<NationStateHistIsCurrChk*>* ret = new std::vector<NationStateHistIsCurrChk*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryCheckStateInSubAreaHistorical ssa(dbAdapter->getAdapter());
    ssa.findStateInSubArea(*ret, key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in StateInSubAreaHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
StateInSubAreaHistoricalDAO::destroy(StateInSubAreaHistoricalKey key,
                                     std::vector<NationStateHistIsCurrChk*>* recs)
{
  destroyContainer(recs);
}

std::string
StateInSubAreaHistoricalDAO::_name("StateInSubAreaHistorical");
std::string
StateInSubAreaHistoricalDAO::_cacheClass("Common");
DAOHelper<StateInSubAreaHistoricalDAO>
StateInSubAreaHistoricalDAO::_helper(_name);
StateInSubAreaHistoricalDAO* StateInSubAreaHistoricalDAO::_instance = nullptr;

/////////////////////////////////// StateInZoneDAO ///////////////////////////////////
log4cxx::LoggerPtr
StateInZoneDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.StateInZoneDAO"));

StateInZoneDAO&
StateInZoneDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

bool
isStateInZoneData(const VendorCode& vendor,
                  int zone,
                  char zoneType,
                  const NationCode& nation,
                  const StateCode& state,
                  DeleteList& deleteList,
                  const DateTime& ticketDate,
                  bool isHistorical)
{
  if (isHistorical)
  {
    StateInZoneHistoricalDAO& dao = StateInZoneHistoricalDAO::instance();
    return dao.isStateInZone(vendor, zone, zoneType, nation, state, deleteList, ticketDate);
  }
  else
  {
    StateInZoneDAO& dao = StateInZoneDAO::instance();
    return dao.isStateInZone(vendor, zone, zoneType, nation, state);
  }
}

bool
StateInZoneDAO::isStateInZone(const VendorCode& vendor,
                              int zone,
                              char zoneType,
                              const NationCode& nation,
                              const StateCode& state)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  StateInZoneKey key(vendor, zone, zoneType, nation, state);
  return *(cache().get(key));
}

NationInAreaInfo*
StateInZoneDAO::create(StateInZoneKey key)
{
  NationInAreaInfo* inZone = new NationInAreaInfo;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryCheckStateInZone sz(dbAdapter->getAdapter());
    *inZone = sz.isStateInZone(key._a, key._b, key._c, key._d, key._e);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in StateInZoneDAO::create");
    delete inZone;
    throw;
  }

  return inZone;
}

void
StateInZoneDAO::destroy(StateInZoneKey key, NationInAreaInfo* rec)
{
  delete rec;
}

std::string
StateInZoneDAO::_name("StateInZone");
std::string
StateInZoneDAO::_cacheClass("Common");
DAOHelper<StateInZoneDAO>
StateInZoneDAO::_helper(_name);
StateInZoneDAO* StateInZoneDAO::_instance = nullptr;

/////////////////////////////////// StateInZoneHistoricalDAO ///////////////////////////////////
log4cxx::LoggerPtr
StateInZoneHistoricalDAO::_logger(
    log4cxx::Logger::getLogger("atseintl.DBAccess.StateInZoneHistoricalDAO"));

StateInZoneHistoricalDAO&
StateInZoneHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

bool
StateInZoneHistoricalDAO::isStateInZone(const VendorCode& vendor,
                                        int zone,
                                        char zoneType,
                                        const NationCode& nation,
                                        const StateCode& state,
                                        DeleteList& del,
                                        const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  StateInZoneHistoricalKey key(vendor, zone, zoneType, nation, state);
  DAOUtils::getDateRange(ticketDate, key._f, key._g, _cacheBy);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsCurrentH<NationStateHistIsCurrChk>(ticketDate));
  if (i == ptr->end())
    return false;

  if (((*i)->inclExclInd()) == 'I')
    return true;

  return false;
}

std::vector<NationStateHistIsCurrChk*>*
StateInZoneHistoricalDAO::create(StateInZoneHistoricalKey key)
{
  std::vector<NationStateHistIsCurrChk*>* ret = new std::vector<NationStateHistIsCurrChk*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryCheckStateInZoneHistorical szh(dbAdapter->getAdapter());
    szh.findStateInZone(*ret, key._a, key._b, key._c, key._d, key._e, key._f, key._g);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in StateInZoneHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
StateInZoneHistoricalDAO::destroy(StateInZoneHistoricalKey key,
                                  std::vector<NationStateHistIsCurrChk*>* recs)
{
  destroyContainer(recs);
}

std::string
StateInZoneHistoricalDAO::_name("StateInZoneHistorical");
std::string
StateInZoneHistoricalDAO::_cacheClass("Common");
DAOHelper<StateInZoneHistoricalDAO>
StateInZoneHistoricalDAO::_helper(_name);
StateInZoneHistoricalDAO* StateInZoneHistoricalDAO::_instance = nullptr;
} // namespace tse
