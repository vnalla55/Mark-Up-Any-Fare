#pragma once

#include "Common/QueueKeyedPool.h"
#include "DBAccess/DBConnectionKey.h"
#include "DBAccess/DBServerFactory.h"
#include "DBAccess/DBServer.h"

#include <string>

namespace tse
{
class ConfigMan;
class DataManager;
class DBServerFactory;
class MockDataManager;
class QueryMockDataManager;

class DBServerPool : public sfc::QueueKeyedPool<DBConnectionKey, DBServer>
{
private:
  friend class tse::DataManager;
  friend class tse::MockDataManager;
  friend class tse::QueryMockDataManager;

  static DBServerPool* _instance;

  ConfigMan& _config;

  DBServerPool(ConfigMan& config, DBServerFactory& factory)
    : sfc::QueueKeyedPool<DBConnectionKey, DBServer>(factory), _config(config)
  {
  }

  DBServerPool(const DBServerPool& rhs) = delete;
  DBServerPool& operator=(const DBServerPool& rhs) = delete;

public:
  static DBServerPool& instance();

  pointer_type get(const std::string&);
};

} // namespace tse

