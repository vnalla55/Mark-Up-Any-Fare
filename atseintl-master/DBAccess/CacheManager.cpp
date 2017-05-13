//----------------------------------------------------------------------------
//
//  File:    CacheManager.cpp
//
//  Description:
//     CacheManager holds all the caches.
//     It is called from the server when the shared library is loaded.
//
//  Copyright (c) Sabre 2004
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//
//----------------------------------------------------------------------------

#include "DBAccess/CacheManager.h"

#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/Thread/TseScopedExecutor.h"
#include "Common/Thread/TseThreadingConst.h"
#include "DBAccess/Cache.h"
#include "DBAccess/CacheControl.h"
#include "DBAccess/CacheInitializer.h"
#include "DBAccess/CacheRegistry.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/DBHistoryServerPool.h"
#include "DBAccess/DBServerPool.h"
#include "DBAccess/RemoteCache/ReadConfig.h"

#include <algorithm>
#include <cctype>
#include <functional>
#include <iostream>
#include <string>

namespace tse
{

Logger&
CacheManager::getLogger()
{
  static Logger logger("atseintl.DBAccess.CacheManager");
  return logger;
}

bool CacheManager::_useGenericCache(false);

std::map<std::string, CacheManager::CacheParm> CacheManager::cacheParms;

struct CacheManager::LogCache
    : public std::unary_function<std::pair<const std::string, CacheControl*>&, void>
{
  std::ostream& _os;

  LogCache(std::ostream& os) : _os(os) {}

  void operator()(std::pair<const std::string, CacheControl*>& p) const
  {
    const char* id = p.first.c_str();
    CacheControl& ctl = *(p.second);
    _os << "Cache= " << id << "; max=" << ctl.cacheMax() << "; size=" << ctl.cacheSize()
        << "; DB access=" << ctl.accessCount() << "; cache access=" << ctl.readCount() << std::endl;
  }
}; // lint !e1509

CacheManager* CacheManager::_instance = nullptr;
bool CacheManager::_initialized = false;

bool
CacheManager::initialized()
{
  return _initialized;
}

CacheManager::CacheManager(tse::ConfigMan& config) : _config(config)
{
  _instance = this;

  CacheRegistry& registry = CacheRegistry::instance();
  std::vector<tse::ConfigMan::NameValue> cfgValues;

  int useGenericCache(0);  
  if (!config.getValue("usegenericcache", useGenericCache))
  {
    useGenericCache = 0;
  }

  _useGenericCache = useGenericCache != 0;

  if (config.getValues(cfgValues, "Cache"))
  {
    std::string name = "";
    int j = 0;

    std::vector<tse::ConfigMan::NameValue>::iterator i = cfgValues.begin();
    for (; i != cfgValues.end(); i++)
    {
      if (name != i->name)
      {
        name = i->name;
        j = 0;
      }
      else
      {
        j++;
      }

      CacheParm& parm = cacheParms[name];

      switch (j)
      {
      case 0:
        parm.cacheMax = atoi(i->value.c_str());
        parm.loadOnStart = true;
        parm.loadOnUpdate = false;
        parm.flushInterval = 0;
        parm.cacheType = "";
        parm.totalCapacity = 0;
        parm.threshold = 0;
        parm.cacheBy = DAOUtils::HALFMONTHLY;
        parm.partialConsolidationLimit = std::numeric_limits<size_t>::max();
        parm.fullConsolidationLimit = std::numeric_limits<size_t>::max();
        break;
      case 1:
        if (i->value != "0")
          parm.loadOnStart = false;
        break;
      case 2:
        if (i->value == "1")
          parm.loadOnUpdate = true;
        break;
      case 3:
        parm.flushInterval = atoi(i->value.c_str());
        break;
      case 4:
        parm.cacheType = i->value;
        break;
      case 5:
        if (!i->value.empty())
          parm.totalCapacity = atoi(i->value.c_str());
        break;
      case 6:
        if (!i->value.empty())
        {
          size_t threshold(atoi(i->value.c_str()));
          if (threshold > 0)
          {
            LOG4CXX_WARN(getLogger(),
                         "Cache:" << name << " ignorind threshold value " << threshold
                                  << " using 0");
          }
          parm.threshold = 0;
        }
        break;
      case 7:
        if (!i->value.empty())
        {
          parm.cacheBy = decodeCacheBy(i->value);
          LOG4CXX_DEBUG(getLogger(),
                        "Cache: " << name << " cacheBy = " << encodeCacheBy(parm.cacheBy));
          if (parm.cacheBy == DAOUtils::ERROR)
          {
            parm.cacheBy = DAOUtils::NODATES;
            LOG4CXX_ERROR(getLogger(), "Invalid CacheBy on " << name << " = " << i->value);
          }
        }
        break;
      case 8:
        if (!i->value.empty())
          parm.partialConsolidationLimit = atoi(i->value.c_str());
        break;
      case 9:
        if (!i->value.empty())
          parm.fullConsolidationLimit = atoi(i->value.c_str());
        break;
      }
    }
  }

  int maxConn(RemoteCache::ReadConfig::getMaxDBConnections());
  if (maxConn < 1 && !config.getValue("connections", maxConn))
  {
    maxConn = 20;
  }
  int minPoolSize = -1;
  if (!config.getValue("minpoolsize", minPoolSize))
    minPoolSize = -1;

  int idleTimeout = -1;
  if (!config.getValue("idletimeout", idleTimeout))
    idleTimeout = -1;

  DBServerPool& dbServerPool = DBServerPool::instance();
  DBServerPool::pointer_type dbServer = dbServerPool.get("Default");
  dbServerPool.setMaxSize(maxConn);
  dbServerPool.setMinSize(minPoolSize);
  dbServerPool.setIdleCloseTimeout(idleTimeout);

  if (Global::allowHistorical())
  {
    DBHistoryServerPool& dbHistoryServerPool = DBHistoryServerPool::instance();
    DBHistoryServerPool::pointer_type dbHistoryServer = dbHistoryServerPool.get("Historical");
    dbHistoryServerPool.setMaxSize(maxConn);
    dbHistoryServerPool.setMinSize(minPoolSize);
    dbHistoryServerPool.setIdleCloseTimeout(idleTimeout);
  }

  static constexpr int NUM_THREADS = 5;
  TseScopedExecutor taskPool(TseThreadingConst::CACHE_INITIALIZATION_TASK, NUM_THREADS);
  CacheInitializer initCache(&taskPool, cacheParms);
  registry.forEach(initCache);
  DISKCACHE.doneWithEnvironment();
  dump();
  _initialized = true;
}

CacheManager::~CacheManager() { dump(); }

CacheManager&
CacheManager::instance()
{
  // Don't bother with mutex stuff. We're guaranteed to be a singleton
  // by the way the dynamic module is loaded and initialized.
  return *CacheManager::_instance;
}

int
CacheManager::cacheSize(const std::string& id) const
{
  std::string key(id);
  std::transform(key.begin(), key.end(), key.begin(), toupper);
  std::map<std::string, CacheParm>::const_iterator i = cacheParms.find(key);
  if (i != cacheParms.end())
    return i->second.cacheMax;
  else
    return 0;
}

bool
CacheManager::cacheLoadOnUpdate(const std::string& id) const
{
  std::string key(id);
  std::transform(key.begin(), key.end(), key.begin(), toupper);
  std::map<std::string, CacheParm>::const_iterator i = cacheParms.find(key);
  if (i != cacheParms.end())
    return i->second.loadOnUpdate;
  else
    return false;
}

bool
CacheManager::cacheLoadOnStart(const std::string& id) const
{
  bool retval(true);
  std::string key(id);
  std::transform(key.begin(), key.end(), key.begin(), toupper);
  std::map<std::string, CacheParm>::const_iterator i = cacheParms.find(key);
  if (i != cacheParms.end())
  {
    retval = i->second.loadOnStart;
  }
  return retval;
}

int
CacheManager::cacheFlushInterval(const std::string& id) const
{
  std::string key(id);
  std::transform(key.begin(), key.end(), key.begin(), toupper);
  std::map<std::string, CacheParm>::const_iterator i = cacheParms.find(key);
  if (i != cacheParms.end())
    return i->second.flushInterval;
  else
    return 0;
}

size_t
CacheManager::cacheFullConsolidationLimit(const std::string& id) const
{
  std::string key(id);
  std::transform(key.begin(), key.end(), key.begin(), toupper);
  std::map<std::string, CacheParm>::const_iterator i = cacheParms.find(key);
  if (i != cacheParms.end())
    return i->second.fullConsolidationLimit;
  else
    return std::numeric_limits<size_t>::max();
}

size_t
CacheManager::cachePartialConsolidationLimit(const std::string& id) const
{
  std::string key(id);
  std::transform(key.begin(), key.end(), key.begin(), toupper);
  std::map<std::string, CacheParm>::const_iterator i = cacheParms.find(key);
  if (i != cacheParms.end())
    return i->second.partialConsolidationLimit;
  else
    return std::numeric_limits<size_t>::max();
}

size_t
CacheManager::getTotalCapacity(const std::string& id) const
{
  std::string key(id);
  std::transform(key.begin(), key.end(), key.begin(), toupper);
  std::map<std::string, CacheParm>::const_iterator it(cacheParms.find(key));
  if (cacheParms.end() == it)
  {
    return 0;
  }
  else
  {
    return it->second.totalCapacity;
  }
}

size_t
CacheManager::getThreshold(const std::string& id) const
{
  std::string key(id);
  std::transform(key.begin(), key.end(), key.begin(), toupper);
  std::map<std::string, CacheParm>::const_iterator it(cacheParms.find(key));
  if (cacheParms.end() == it)
  {
    return 0;
  }
  else
  {
    return it->second.threshold;
  }
}

namespace
{
static std::string EmptyCacheType;
}

const std::string&
CacheManager::cacheType(const std::string& id) const
{
  std::string key(id);
  std::transform(key.begin(), key.end(), key.begin(), toupper);
  std::map<std::string, CacheParm>::const_iterator i = cacheParms.find(key);
  if (i != cacheParms.end())
    return i->second.cacheType;
  else
    return EmptyCacheType;
}

DAOUtils::CacheBy
CacheManager::cacheBy(const std::string& id) const
{
  std::string key(id);
  std::transform(key.begin(), key.end(), key.begin(), toupper);
  std::map<std::string, CacheParm>::const_iterator i = cacheParms.find(key);
  if (i != cacheParms.end())
    return i->second.cacheBy;
  else
  {
    CacheParm& parm = cacheParms[key];
    parm.cacheBy = DAOUtils::HALFMONTHLY;
    LOG4CXX_DEBUG(getLogger(),
                  "No date range for " << id << " set to " << encodeCacheBy(parm.cacheBy));
    return parm.cacheBy;
  }
}

void
CacheManager::dump(int level) const
{
  log4cxx::LevelPtr logLevel(log4cxx::Level::toLevel(level));
  if (getLogger()->isEnabledFor(logLevel))
  {
    std::ostringstream os;
    dump(os);
    getLogger()->forcedLog(logLevel, os.str());
  }
}

void
CacheManager::dump(std::ostream& os) const
{
  CacheRegistry& registry = CacheRegistry::instance();
  os << "Cache statistics" << std::endl;
  LogCache func(os);
  registry.forEach(func);
}

DAOUtils::CacheBy
CacheManager::decodeCacheBy(const std::string& name)
{
  static struct CacheByTranslate
  {
    const char* name;
    DAOUtils::CacheBy type;
  } translate[] = { { "NoDates", DAOUtils::NODATES },
                    { "Daily", DAOUtils::DAILY },
                    { "Weekly", DAOUtils::WEEKLY },
                    { "HalfMonthly", DAOUtils::HALFMONTHLY },
                    { "Monthly", DAOUtils::MONTHLY },
                    { "TwoMonths", DAOUtils::TWOMONTHS },
                    { "ThreeMonths", DAOUtils::THREEMONTHS },
                    { "FourMonths", DAOUtils::FOURMONTHS },
                    { "SixMonths", DAOUtils::SIXMONTHS },
                    { "Yearly", DAOUtils::YEARLY },
                    { "0", DAOUtils::NODATES },
                    { "N", DAOUtils::NODATES },
                    { "D", DAOUtils::DAILY },
                    { "W", DAOUtils::WEEKLY },
                    { "H", DAOUtils::HALFMONTHLY },
                    { "M", DAOUtils::MONTHLY },
                    { "T", DAOUtils::TWOMONTHS },
                    { "U", DAOUtils::THREEMONTHS },
                    { "F", DAOUtils::FOURMONTHS },
                    { "S", DAOUtils::SIXMONTHS },
                    { "Y", DAOUtils::YEARLY },
                    { nullptr, DAOUtils::NODATES } };
  int idx;
  for (idx = 0; translate[idx].name != nullptr; idx++)
  {
    if (name.compare(translate[idx].name) == 0)
      return translate[idx].type;
  }
  return DAOUtils::ERROR;
}

const char*
CacheManager::encodeCacheBy(DAOUtils::CacheBy cacheBy)
{
  static struct PrintTranslate
  {
    const char* name;
    DAOUtils::CacheBy type;
  } printNames[] = { { "NoDates", DAOUtils::NODATES },
                     { "Daily", DAOUtils::DAILY },
                     { "Weekly", DAOUtils::WEEKLY },
                     { "HalfMonthly", DAOUtils::HALFMONTHLY },
                     { "Monthly", DAOUtils::MONTHLY },
                     { "TwoMonths", DAOUtils::TWOMONTHS },
                     { "ThreeMonths", DAOUtils::THREEMONTHS },
                     { "FourMonths", DAOUtils::FOURMONTHS },
                     { "SixMonths", DAOUtils::SIXMONTHS },
                     { "Yearly", DAOUtils::YEARLY },
                     { nullptr, DAOUtils::ERROR } };
  int idx;
  for (idx = 0; printNames[idx].name != nullptr; idx++)
  {
    if (cacheBy == printNames[idx].type)
      return printNames[idx].name;
  }
  return "ERROR";
}
}
