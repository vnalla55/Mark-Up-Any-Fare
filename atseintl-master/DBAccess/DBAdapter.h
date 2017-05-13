#pragma once

#include "DBAccess/DBHistoryServerPool.h"
#include "DBAccess/DBServerPool.h"

#include <string>

namespace DBAccess
{
class BoundParameter;
}

namespace tse
{
class DateTime;

class DBAdapter
{
public:
  virtual ~DBAdapter() = default;

  // The following methods are optional and only required if the concrete
  //  DB Adapter supports bound query parameters (aka: bind variables).
  //
  virtual void bindParameter(const char* parm, int32_t index) {}
  virtual void bindParameter(const std::string& parm, int32_t index) {}
  virtual void bindParameter(int32_t parm, int32_t index) {}
  virtual void bindParameter(int64_t parm, int32_t index) {}
  virtual void bindParameter(float parm, int32_t index) {}
  virtual void bindParameter(const DateTime& parm, int32_t index, bool dateOnly = false) {}

  virtual void bindParameter(DBAccess::BoundParameter* parm) {}
};

class DBAdapterWrapper final
{
public:
  void setDBServer(DBServerPool::pointer_type a_dbServer)
  {
    m_dbServer = a_dbServer;
    m_Historical = false;
  }

  void setDBServer(DBHistoryServerPool::pointer_type a_dbServer)
  {
    m_dbHistoryServer = a_dbServer;
    m_Historical = true;
  }

  DBAdapter* getAdapter()
  {
    return (m_Historical) ? m_dbHistoryServer->getAdapter() : m_dbServer->getAdapter();
  }

private:
  bool m_Historical = false;
  DBServerPool::pointer_type m_dbServer;
  DBHistoryServerPool::pointer_type m_dbHistoryServer;
};
} // namespace tse

