#pragma once

#include "Common/Config/ConfigMan.h"
#include "Common/QueueKeyedPool.h"
#include "DBAccess/DBConnectionKey.h"
#include "DBAccess/DBHistoryServerFactory.h"

#include <string>

namespace tse
{

class DataManager;
class MockDataManager;
class QueryMockDataManager;

class DBHistoryServerPool : public sfc::QueueKeyedPool<DBConnectionKey, tse::DBHistoryServer>
{
private:
  friend class tse::DataManager;
  friend class tse::MockDataManager;
  friend class tse::QueryMockDataManager;

  static DBHistoryServerPool* _instance;

  tse::ConfigMan& _config;

  DBHistoryServerPool(tse::ConfigMan& config, tse::DBHistoryServerFactory& factory)
    : sfc::QueueKeyedPool<DBConnectionKey, tse::DBHistoryServer>(factory), _config(config)
  {
  }

  virtual ~DBHistoryServerPool() {}

  DBHistoryServerPool(const DBHistoryServerPool& rhs);
  DBHistoryServerPool& operator=(const DBHistoryServerPool& rhs);

public:
  static DBHistoryServerPool& instance();
  pointer_type get(std::string);
};

} // namespace tse

