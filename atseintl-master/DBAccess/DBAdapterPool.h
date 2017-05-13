#pragma once

#include "DBAccess/DBAdapter.h"
#include "DBAccess/DBServerPool.h"

namespace tse
{

class DBAdapterPool
{
public:
  static DBAdapterPool& instance();

  typedef std::shared_ptr<DBAdapterWrapper> pointer_type;

  pointer_type get(const std::string&, bool fHistorical = false);

private:
  static DBAdapterPool _instance;

  DBAdapterPool();
  virtual ~DBAdapterPool();

  DBAdapterPool(const DBAdapterPool& rhs);
  DBAdapterPool& operator=(const DBAdapterPool& rhs);
};

} // namespace tse

