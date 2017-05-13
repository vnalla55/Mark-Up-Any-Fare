//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "DBAccess/StateDAO.h"

#include "Common/Logger.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DAOInterface.h"
#include "DBAccess/DBAdapterPool.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/Queries/QueryGetState.h"
#include "DBAccess/State.h"

namespace tse
{
log4cxx::LoggerPtr
StateDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.StateDAO"));

StateDAO&
StateDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}
const State*
getStateData(const NationCode& nationCode,
             const StateCode& stateCode,
             const DateTime& date,
             DeleteList& deleteList,
             const DateTime& ticketDate,
             bool isHistorical)
{
  if (isHistorical)
  {
    StateHistoricalDAO& dao = StateHistoricalDAO::instance();
    return dao.get(deleteList, nationCode, stateCode, date, ticketDate);
  }
  else
  {
    StateDAO& dao = StateDAO::instance();
    return dao.get(deleteList, nationCode, stateCode, date, ticketDate);
  }
}

const State*
StateDAO::get(DeleteList& del,
              const NationCode& nation,
              const StateCode& state,
              const DateTime& date,
              const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  StateKey key(nation, state);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  State* ret = nullptr;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveG<State>(date, ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

void
StateDAO::load()
{
  StartupLoader<QueryGetStates, State, StateDAO>();
}

StateKey
StateDAO::createKey(State* info)
{
  return StateKey(info->nation(), info->state());
}

std::vector<State*>*
StateDAO::create(StateKey key)
{
  std::vector<State*>* ret = new std::vector<State*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetState sta(dbAdapter->getAdapter());
    sta.findState(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in StateDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
StateDAO::destroy(StateKey key, std::vector<State*>* recs)
{
  destroyContainer(recs);
}

std::string
StateDAO::_name("State");
std::string
StateDAO::_cacheClass("Common");
DAOHelper<StateDAO>
StateDAO::_helper(_name);
StateDAO* StateDAO::_instance = nullptr;

// --------------------------------------------------
// Historical DAO: StateHistoricalDAO
// --------------------------------------------------
log4cxx::LoggerPtr
StateHistoricalDAO::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.StateHistoricalDAO"));
StateHistoricalDAO&
StateHistoricalDAO::instance()
{
  if (_instance == nullptr)
  {
    _helper.init();
  }
  return *_instance;
}

const State*
StateHistoricalDAO::get(DeleteList& del,
                        const NationCode& nation,
                        const StateCode& state,
                        const DateTime& date,
                        const DateTime& ticketDate)
{
  // Track calls for code coverage
  _codeCoverageGetCallCount++;

  StateKey key(nation, state);
  DAOCache::pointer_type ptr = cache().get(key);
  del.copy(ptr);
  State* ret = nullptr;
  DAOCache::value_type::iterator i =
      find_if(ptr->begin(), ptr->end(), IsEffectiveHist<State>(date, ticketDate));
  if (i != ptr->end())
    ret = *i;
  return ret;
}

struct StateHistoricalDAO::groupByKey
{
public:
  StateKey prevKey;
  DAOCache& cache;

  groupByKey() : cache(StateHistoricalDAO::instance().cache()), ptr(nullptr) {}

  std::vector<State*>* ptr;

  void operator()(State* info)
  {
    StateKey key(info->nation(), info->state());
    if (!(key == prevKey))
    {
      ptr = new std::vector<State*>;
      cache.put(key, ptr);
      prevKey = key;
    }
    ptr->push_back(info);
  }
};

void
StateHistoricalDAO::load()
{
  if (!Global::allowHistorical())
    return;

  // Track calls for code coverage
  _codeCoverageLoadCallCount++;

  std::vector<State*> recs;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetStatesHistorical sta(dbAdapter->getAdapter());
    sta.findAllStates(recs);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in StateHistoricalDAO::load");
    deleteVectorOfPointers(recs);
    throw;
  }

  std::for_each(recs.begin(), recs.end(), groupByKey());
}

std::vector<State*>*
StateHistoricalDAO::create(StateKey key)
{
  std::vector<State*>* ret = new std::vector<State*>;

  DBAdapterPool& dbAdapterPool = DBAdapterPool::instance();
  DBAdapterPool::pointer_type dbAdapter = dbAdapterPool.get(this->cacheClass());
  try
  {
    QueryGetStateHistorical sta(dbAdapter->getAdapter());
    sta.findState(*ret, key._a, key._b);
  }
  catch (...)
  {
    LOG4CXX_WARN(_logger, "DB exception in StateHistoricalDAO::create");
    destroyContainer(ret);
    throw;
  }

  return ret;
}

void
StateHistoricalDAO::destroy(StateKey key, std::vector<State*>* recs)
{
  destroyContainer(recs);
}

std::string
StateHistoricalDAO::_name("StateHistorical");
std::string
StateHistoricalDAO::_cacheClass("Common");
DAOHelper<StateHistoricalDAO>
StateHistoricalDAO::_helper(_name);
StateHistoricalDAO* StateHistoricalDAO::_instance = nullptr;

} // namespace tse
