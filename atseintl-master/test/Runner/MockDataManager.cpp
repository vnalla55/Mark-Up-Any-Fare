#include <cstdlib>
#include "test/include/MockDataManager.h"
#include "DBAccess/CacheManager.h"

namespace tse
{
tse::ConfigMan MockDataManager::_mainConfig;

MockDataManager::MockDataManager() : DataManager("MockDataManager", _mainConfig)
{
  const MallocContextDisabler context;

  _mainConfig.read(TSE_VOB_DIR "/test/tseserver.cfg");
  _dbConfig.read(TSE_VOB_DIR "/test/dbaccess.ini");

  const char* fromEnv = getenv("TEST_DB_CONNECTION");
  std::string dbConnection = fromEnv ? fromEnv : "ORACLE-DEV-HIST";

  _dbConfig.setValue("Default", dbConnection, "Connection", true);
  _dbConfig.setValue("Historical", dbConnection, "Connection", true);

  initConfig();
  initDBServerPools();

  try
  {
    if (CacheManager::_instance == 0)
      CacheManager::_instance = new CacheManager(_dbConfig);
  }
  catch (...)
  {
    std::cerr << "An Error occurred while creating the Cache Manager\n"
              << "Please review your dbaccess.ini file and verify it is up to date\n ";
    throw;
  }
}

} // tse
