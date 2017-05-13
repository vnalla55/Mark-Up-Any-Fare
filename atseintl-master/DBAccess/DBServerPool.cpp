#include "DBAccess/DBServerPool.h"

namespace tse
{

DBServerPool* DBServerPool::_instance = nullptr;

DBServerPool&
DBServerPool::instance()
{
  if (LIKELY(_instance))
    return *_instance;
  else
    throw std::runtime_error("DB SERVER POOL NOT INITIALIZED");
}

DBServerPool::pointer_type
DBServerPool::get(const std::string& poolName)
{
  std::string keyPart;
  if (!_config.getValue(poolName, keyPart, "Connection"))
  {
    if (!_config.getValue("Default", keyPart, "Connection"))
      keyPart = "Default";
  }

  DBConnectionKey key(poolName, keyPart);

  return sfc::QueueKeyedPool<DBConnectionKey, tse::DBServer>::get(key);
}

} // namespace tse
