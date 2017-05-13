#include "DBAccess/DBHistoryServerPool.h"

namespace tse
{

DBHistoryServerPool* DBHistoryServerPool::_instance = nullptr;

DBHistoryServerPool&
DBHistoryServerPool::instance()
{
  if (_instance)
    return *_instance;
  else
    throw std::runtime_error("DB HISTORY SERVER POOL NOT INITIALIZED");
}

DBHistoryServerPool::pointer_type
DBHistoryServerPool::get(std::string poolName)
{
  std::string keyPart;
  if (!_config.getValue(poolName, keyPart, "Connection"))
  {
    if (!_config.getValue("Historical", keyPart, "Connection"))
      keyPart = "Historical";
  }

  DBConnectionKey key(poolName, keyPart);

  return sfc::QueueKeyedPool<DBConnectionKey, tse::DBHistoryServer>::get(key);
}
} // namespace tse
