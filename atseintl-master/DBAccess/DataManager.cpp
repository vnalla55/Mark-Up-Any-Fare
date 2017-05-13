//----------------------------------------------------------------------------
//
//  File:    DataManager.C
//
//  Description:
//     DataManager initializes the database management layer.
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

#include "DBAccess/DataManager.h"

#include "Allocator/TrxMalloc.h"
#include "Common/Global.h"
#include "Common/TSEException.h"
#include "DBAccess/CacheManager.h"
#include "DBAccess/CacheRegistry.h"
#include "DBAccess/DBAccessConsts.h"
#include "DBAccess/DBConnectionInfoManager.h"
#include "DBAccess/DBHistoryServerFactory.h"
#include "DBAccess/DBHistoryServerPool.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/DBServerFactory.h"
#include "DBAccess/DBServerPool.h"
#include "DBAccess/ORACLEDBConnectionInfoManager.h"
#include "DBAccess/ORACLEDBHistoryServerFactory.h"
#include "DBAccess/ORACLEDBServerFactory.h"
#include "DBAccess/ORACLEQueryHelper.h"
#include "DBAccess/ORACLEStatementHelperImpl.h"
#include "DBAccess/Queries/PrintBaseSQL.h"
#include "DBAccess/SQLQueryHelper.h"
#include "DBAccess/SQLStatementHelperImpl.h"
#include "DBAccess/StaticObjectPool.h"

#include <boost/filesystem/operations.hpp>

#include <string>

namespace
{
std::time_t
_lastUpdate(0);
}

using namespace DBAccess;
namespace tse
{
log4cxx::LoggerPtr
DataManager::_logger(log4cxx::Logger::getLogger("atseintl.DBAccess.DataManager"));

DataManager* DataManager::_instance = nullptr;
bool DataManager::_forceSortOrder = false;

DataManager::DataManager(const std::string& name, tse::ConfigMan& config)
  : _name(name), _config(config)
{
  _instance = this;
}

DataManager::~DataManager()
{
  if (DBServerPool::_instance != nullptr)
  {
    delete DBServerPool::_instance;
    DBServerPool::_instance = nullptr;
  }
  if (Global::allowHistorical())
  {
    if (DBHistoryServerPool::_instance != nullptr)
    {
      delete DBHistoryServerPool::_instance;
      DBHistoryServerPool::_instance = nullptr;
    }
  }
  if (_factory != nullptr)
    delete _factory;
  if (_historyFactory != nullptr)
    delete _historyFactory;
  if (_sqlQueryHelper != nullptr)
    delete _sqlQueryHelper;
  if (_sqlStatementHelperImpl != nullptr)
    delete _sqlStatementHelperImpl;
}

DataManager&
DataManager::instance()
{
  if (UNLIKELY(!_instance))
  {
    throw TSEException(TSEException::NO_ERROR,
                       "Initialization sequence error! DataManager has not been initialized yet!");
  }

  // Don't bother with mutex stuff. We're guaranteed to be a singleton
  // by the way the dynamic module is loaded and initialized.
  return *DataManager::_instance;
}

bool
DataManager::initialize(int argc, char* argv[])
{
  const MallocContextDisabler context;

  LOG4CXX_DEBUG(_logger, "Data Manager initializing...");
  loadConfigValues();
  initConfig();
  initDBServerPools();
  CacheManager::_instance = new CacheManager(_dbConfig);
  PrintBaseSQL printBaseSQL;
  printBaseSQL.dump();
  LOG4CXX_DEBUG(_logger, "Data Manager initialized.");
  //    DBResultSet::setTheFactory();
  return true;
}

void
DataManager::dbConnChanged()
{
  std::string reloadStr;
  bool reload(true);
  if (_config.getValue("RELOAD_DBCONN_INI", reloadStr, "DB_MAN"))
  {
    reload = "Y" == reloadStr || "YES" == reloadStr || "T" == reloadStr || "TRUE" == reloadStr;
  }
  if (reload)
  {
    std::string key;
    _dbConfig.getValue("Default", key, "Connection");
    std::string conn_ini_fname;
    _dbConfig.getValue("ini", conn_ini_fname, key);
    bool useIni(!conn_ini_fname.empty());
    if (useIni)
    {
      // determine the age of the dbconn.ini
      boost::filesystem::path path(conn_ini_fname);
      std::time_t t(boost::filesystem::last_write_time(path));
      if (t > _lastUpdate)
      {
        bool first(0 == _lastUpdate);
        _lastUpdate = t;
        if (!first)
        {
          LOG4CXX_WARN(_logger,
                       __FUNCTION__ << ":" << conn_ini_fname << " was changed,reloading...");
          loadConfigValues();
        }
      }
    }
  }
}

void
DataManager::loadConfigValues()
{
  const MallocContextDisabler context;

  std::string fname;
  _config.getValue("INI_FILE", fname, _name);
  _dbConfig.read(fname);

  unsigned int unlimitedCacheValue;
  if (_dbConfig.getValue("UNLIMITED_CACHE_INDICATOR", unlimitedCacheValue))
  {
    if (unlimitedCacheValue != Global::getUnlimitedCacheSize())
    {
      LOG4CXX_INFO(_logger, "Setting Unlimited Cache Indicator to " << unlimitedCacheValue);
      Global::setUnlimitedCacheSize(unlimitedCacheValue);
    }
  }

  DBConnectionInfoManager::clearConnectInfo();

  if (_factory)
    _factory->modifyCurrentSynchValue();
}

void
DataManager::initConfig()
{
  const MallocContextDisabler context;

  int16_t dbType;
  std::string key = "";
  _dbConfig.getValue("Default", key, "Connection");
  if (!key.empty())
  {
    _dbConfig.getValue("dbType", dbType, key);
  }

  if (!(dbType >= UNKNOWN_SERVER && dbType < NUM_SERVERS))
  {
    LOG4CXX_ERROR(_logger, "ILLEGAL DB SERVER TYPE: dbType = " << dbType);
  }

  std::string order;
  tse::ConfigMan& config = tse::Global::config();
  config.getValue("FORCE_SQL_SORT_ORDER", order, "DATABASE_TUNING");
  _forceSortOrder = (order == "Y");

  if (dbType == ORACLE_SERVER)
  {
    LOG4CXX_INFO(log4cxx::Logger::getLogger("atseintl.Server.TseServer"), "ORACLE db enabled.")
    _forceSortOrder = true;
    _sqlStatementHelperImpl = new ORACLEStatementHelperImpl;
    _sqlQueryHelper = new ORACLEQueryHelper;
    DBConnectionInfoManager::setImpl(new ORACLEDBConnectionInfoManager(_dbConfig));
    _factory = new ORACLEDBServerFactory(_dbConfig);
    _historyFactory = new ORACLEDBHistoryServerFactory(_dbConfig);
    DBResultSet::setFactory(new ORACLEDBResultSetFactory());
    _factory->modifyCurrentSynchValue();
  }
  else
  {
    LOG4CXX_ERROR(log4cxx::Logger::getLogger("atseintl.Server.TseServer"), "dbType is not correct.")
  }

  LOG4CXX_DEBUG(_logger, " dbType= " << dbType);
  LOG4CXX_INFO(log4cxx::Logger::getLogger("atseintl.Server.TseServer"),
               "DATABASE_TUNING: Force Sort Order "
                   << (_forceSortOrder ? "[enabled]" : "[disabled]"));
}

void
DataManager::initDBServerPools()
{
  const MallocContextDisabler context;

  DBServerPool::_instance = new DBServerPool(_dbConfig, (*_factory));
  if (Global::allowHistorical())
    DBHistoryServerPool::_instance = new DBHistoryServerPool(_dbConfig, (*_historyFactory));
}

void
DataManager::dumpDbConnections(std::ostream& os)
{
  instance()._factory->dumpDbConnections(os);
}

struct DataManager::LogCache
    : public std::unary_function<std::pair<const std::string, CacheControl*>&, void>
{
  std::ostream& _os;
  LogCache(std::ostream& os) : _os(os) {}
  void operator()(std::pair<const std::string, CacheControl*>& p) const
  {
    const char* id = p.first.c_str();
    CacheControl& ctl = *(p.second);
    size_t used = 0;
    size_t indirect = 0;
    size_t item_size = 0;
    size_t memory = ctl.getCacheMemory(used, indirect, item_size);
    static const char DELIM = '|';
    _os << id << DELIM << ctl.cacheSize() << DELIM << memory << DELIM << used << DELIM << indirect
        << DELIM << item_size << std::endl;
    LOG4CXX_DEBUG(_logger,
                  id << " Entries= " << ctl.cacheSize() << " Capacity= " << memory
                     << " UsedMemory= " << used << " IndirectMemory= " << indirect
                     << " ItemSize= " << item_size);
  }
};

struct DataManager::LogPool
    : public std::unary_function<std::pair<const std::string, StaticPoolStatistics*>&, void>
{
  std::ostream& _os;
  LogPool(std::ostream& os) : _os(os) {}
  void operator()(std::pair<const std::string, StaticPoolStatistics*>& p) const
  {
    static const char DELIM = '|';
    const char* name = p.first.c_str();
    StaticPoolStatistics& pool = *(p.second);
    _os << name << DELIM << pool.itemSize() << DELIM << pool.numActive() << DELIM
        << pool.maxActive() << DELIM << pool.memory() << std::endl;
    LOG4CXX_DEBUG(_logger,
                  name << " size= " << pool.itemSize() << " active= " << pool.numActive()
                       << " max= " << pool.maxActive() << " mem= " << pool.memory());
  }
};

void
DataManager::dumpCacheMemory(std::ostream& os, std::string name)
{
  if (_logger->isInfoEnabled())
  {
    CacheRegistry& registry = CacheRegistry::instance();
    LogCache func(os);
    if (name.empty())
    {
      registry.forEach(func);
    }
    else
    {
      CacheControl* ctrl = registry.getCacheControl(name);
      std::pair<const std::string, CacheControl*> pair(name, ctrl);
      func(pair);
    }
  }
}

void
DataManager::dumpPoolMemory(std::ostream& os, std::string name)
{
  if (_logger->isInfoEnabled())
  {
    StaticPoolRegistry& registry = StaticPoolRegistry::instance();
    LogPool func(os);
    if (name.empty())
      registry.forEach(func);
    else
    {
      StaticPoolStatistics* stats = registry.getStatistics(name);
      std::pair<const std::string, StaticPoolStatistics*> pair(name, stats);
      func(pair);
    }
  }
}

const SQLStatementHelperImpl&
DataManager::getSQLStatementHelperImpl()
{
  return *(instance()._sqlStatementHelperImpl);
}

const SQLQueryHelper&
DataManager::getSQLQueryHelper()
{
  return *(instance()._sqlQueryHelper);
}

const TseSynchronizingValue
DataManager::getSyncToken()
{
  if (instance()._factory)
    return instance()._factory->getSyncToken();
  return TseSynchronizingValue();
}
}
