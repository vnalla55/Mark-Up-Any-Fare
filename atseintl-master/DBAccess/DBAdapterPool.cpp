#include "DBAccess/DBAdapterPool.h"

namespace tse
{

DBAdapterPool DBAdapterPool::_instance;

DBAdapterPool&
DBAdapterPool::instance()
{
  return _instance;
}

DBAdapterPool::DBAdapterPool() {}

DBAdapterPool::~DBAdapterPool() {}

DBAdapterPool::pointer_type
DBAdapterPool::get(const std::string& poolName, bool fHistorical)
{
  pointer_type pDBAdapter(new DBAdapterWrapper());

  if (UNLIKELY(fHistorical))
  {
    DBHistoryServerPool& dbServerPool = DBHistoryServerPool::instance();
    DBHistoryServerPool::pointer_type dbServer = dbServerPool.get(poolName);

    pDBAdapter->setDBServer(dbServer);
  }
  else
  {
    DBServerPool& dbServerPool = DBServerPool::instance();
    DBServerPool::pointer_type dbServer = dbServerPool.get(poolName);

    pDBAdapter->setDBServer(dbServer);
  }

  return pDBAdapter;
}
} // namespace tse
