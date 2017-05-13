//
// DataManager.h
//
#pragma once

#include "Common/Config/ConfigMan.h" // sfc
#include "Common/LoggerPtr.h"
#include "Common/TseSynchronizingValue.h"

#include <string>
#include <vector>

namespace DBAccess
{
class SQLStatementHelperImpl;
class SQLQueryHelper;
}

namespace tse
{
class DBServerFactory;
class DBHistoryServerFactory;

class DataManager
{
public:
  virtual bool initialize(int argc = 0, char* argv[] = nullptr);
  static DataManager& instance();

  const tse::ConfigMan& dbConfig() const { return _dbConfig; }

  void dbConnChanged();
  void loadConfigValues();
  static void dumpDbConnections(std::ostream& os);
  static void dumpCacheMemory(std::ostream& os, std::string name);
  static void dumpPoolMemory(std::ostream& os, std::string name);

  static const DBAccess::SQLStatementHelperImpl& getSQLStatementHelperImpl();

  static const DBAccess::SQLQueryHelper& getSQLQueryHelper();

  static bool forceSortOrder() { return _forceSortOrder; }

  static const TseSynchronizingValue getSyncToken();

protected:
  friend class TseServer;
  friend class MockDataManager;
  friend class QueryMockDataManager;

  std::string _name;
  tse::ConfigMan& _config;

  static bool _forceSortOrder;

  DataManager(const std::string& name, tse::ConfigMan& config);

  virtual ~DataManager();

  static DataManager* _instance;

  tse::ConfigMan _dbConfig;
  DBServerFactory* _factory = nullptr;
  DBHistoryServerFactory* _historyFactory = nullptr;
  struct LogCache;
  struct LogPool;

  DBAccess::SQLStatementHelperImpl* _sqlStatementHelperImpl = nullptr;
  DBAccess::SQLQueryHelper* _sqlQueryHelper = nullptr;

  /** read in config file */
  void initConfig();
  void initDBServerPools();

private:
  DataManager(const DataManager&);
  DataManager& operator=(const DataManager&);
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
