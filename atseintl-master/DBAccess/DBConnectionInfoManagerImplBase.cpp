//----------------------------------------------------------------------------
//
//     File:           DBConnectionInfoManagerImplBase.cpp
//     Description:    Database connection info manager abstract base class.
//     Created:        03/08/2010
//     Authors:        Andrew Ahmad
//
//     Updates:
//
//     Copyright 2010, Sabre Inc.  All rights reserved.
//         This software/documentation is the confidential and proprietary
//         product of Sabre Inc. Any unauthorized use, reproduction, or
//         transfer of this software/documentation, in any medium, or
//         incorporation of this software/documentation into any system
//         or publication, is strictly prohibited.
//
//---------------------------------------------------------------------------

#include "DBAccess/DBConnectionInfoManagerImplBase.h"

#include "Common/Thread/TSELockGuards.h"
#include "Util/BranchPrediction.h"

namespace DBAccess
{

DBConnectionInfoFactory::~DBConnectionInfoFactory() {}

DBConnectionInfoManagerImplBase::DBConnectionInfoManagerImplBase() {}

DBConnectionInfoManagerImplBase::~DBConnectionInfoManagerImplBase()
{
  TSEGuard g(_mutex);

  PoolInfoMap::iterator poolIter = _poolInfos.begin();
  PoolInfoMap::iterator poolIterEnd = _poolInfos.end();
  for (; poolIter != poolIterEnd; ++poolIter)
  {
    delete (*poolIter).second;
  }
  _poolInfos.clear();

  ConnectionInfoContainer::iterator connIter = _connections.begin();
  ConnectionInfoContainer::iterator connIterEnd = _connections.end();

  for (; connIter != connIterEnd; ++connIter)
  {
    delete (*connIter);
  }
  _connections.clear();
}

bool
DBConnectionInfoManagerImplBase::getConnectInfo(const std::string& pool,
                                                std::string& host,
                                                uint16_t& port,
                                                std::string& database,
                                                std::string& service,
                                                std::string& user,
                                                std::string& password,
                                                bool isHistorical)
{
  TSEGuard g(_mutex);

  DBPoolInfo* poolInfo = getPoolInfo(pool);
  if (!poolInfo)
  {
    poolInfo = createPoolInfo(pool, isHistorical);
    if (poolInfo)
      _poolInfos[pool] = poolInfo;
  }
  if (poolInfo)
  {
    const DBConnectionInfo* conn = poolInfo->getCurrentConnectionInfo();
    if (!conn)
    {
      conn = createConnInfo(poolInfo);
      if (conn)
        poolInfo->addConnectionInfo(conn);
    }
    if (conn)
    {
      host = conn->host();
      port = conn->port();
      database = conn->database();
      service = conn->service();
      user = conn->user();
      password = conn->password();

      return true;
    }
  }
  return false;
}

bool
DBConnectionInfoManagerImplBase::isCurrentConnectInfo(const std::string& pool,
                                                      const std::string& host,
                                                      uint16_t port,
                                                      const std::string& database,
                                                      const std::string& service,
                                                      const std::string& user,
                                                      const std::string& password)
{
  TSEGuard g(_mutex);

  DBPoolInfo* poolInfo = getPoolInfo(pool);
  if (LIKELY(poolInfo))
  {
    const DBConnectionInfo* conn = poolInfo->getCurrentConnectionInfo();
    if (LIKELY(conn))
    {
      if (LIKELY(conn->isEquivalent(host, port, database, service, user, password)))
      {
        return true;
      }
    }
  }
  return false;
}

void
DBConnectionInfoManagerImplBase::processConnectionFailure(const std::string& host,
                                                          uint16_t port,
                                                          const std::string& database,
                                                          const std::string& service,
                                                          const std::string& user,
                                                          bool usePriorityOrder)
{
  TSEGuard g(_mutex);

  DBConnectionInfo* conn = getConnInfo(host, port, database, service, user);

  if (conn)
  {
    conn->incrementFailureCount();

    PoolInfoMap::iterator iter = _poolInfos.begin();
    PoolInfoMap::iterator iterEnd = _poolInfos.end();
    for (; iter != iterEnd; ++iter)
    {
      DBPoolInfo* poolInfo = (*iter).second;
      poolInfo->processConnectionFailure(conn, this, usePriorityOrder);
    }
  }
}

bool
DBConnectionInfoManagerImplBase::clearConnectInfo()
{
  // Config settings will be reloaded lazily as connection info
  // is requested for each pool. All we need to do here is delete
  // all the connection info for all pools which will trigger a
  // reload the next time connection info is requested for each pool.
  //
  TSEGuard g(_mutex);

  LOG4CXX_INFO(getLogger(), "Clearing Pool and Connection Information.");

  PoolInfoMap::iterator poolIter = _poolInfos.begin();
  PoolInfoMap::iterator poolIterEnd = _poolInfos.end();
  for (; poolIter != poolIterEnd; ++poolIter)
  {
    delete (*poolIter).second;
  }
  _poolInfos.clear();

  ConnectionInfoContainer::iterator connIter = _connections.begin();
  ConnectionInfoContainer::iterator connIterEnd = _connections.end();

  for (; connIter != connIterEnd; ++connIter)
  {
    delete (*connIter);
  }
  _connections.clear();
  return true;
}

void
DBConnectionInfoManagerImplBase::addConnInfo(DBConnectionInfo* connInfo)
{
  // Concurrency control (mutex locking) must be handled at a higher
  // enclosing scope.
  //
  if (!connInfo)
  {
    LOG4CXX_ERROR(getLogger(), "Attempted to add null DBConnectionInfo to the collection");
    return;
  }
  _connections.push_back(connInfo);
}

DBConnectionInfo*
DBConnectionInfoManagerImplBase::getConnInfo(const std::string& host,
                                             uint16_t port,
                                             const std::string& database,
                                             const std::string& service,
                                             const std::string& user)
{
  // Concurrency control (mutex locking) must be handled at a higher
  // enclosing scope.
  //
  ConnectionInfoContainer::iterator iter = _connections.begin();
  ConnectionInfoContainer::iterator iterEnd = _connections.end();

  for (; iter != iterEnd; ++iter)
  {
    DBConnectionInfo* conn = *iter;
    if (conn->isEquivalent(host, port, database, service, user))
    {
      return conn;
    }
  }
  return nullptr;
}

DBPoolInfo*
DBConnectionInfoManagerImplBase::getPoolInfo(const std::string& pool)
{
  // Concurrency control (mutex locking) must be handled at a higher
  // enclosing scope.
  //
  PoolInfoMap::iterator i = _poolInfos.find(pool);
  if (i != _poolInfos.end())
    return (*i).second;

  return nullptr;
}

tse::Logger&
DBConnectionInfoManagerImplBase::getLogger()
{
  static tse::Logger logger("atseintl.DBAccess.DBConnectionInfoManagerImplBase");
  return logger;
}

void
DBConnectionInfo::incrementFailureCount()
{
  TSEGuard g(_mutex);
  ++_failureCount;
}

bool
DBConnectionInfo::isEquivalent(const std::string& host,
                               uint16_t port,
                               const std::string& database,
                               const std::string& service,
                               const std::string& user) const
{
  if (LIKELY(_host == host && _port == port && _database == database && _service == service &&
      _user == user))
  {
    return true;
  }
  return false;
}

bool
DBConnectionInfo::isEquivalent(const std::string& host,
                               uint16_t port,
                               const std::string& database,
                               const std::string& service,
                               const std::string& user,
                               const std::string& password) const
{
  if (LIKELY(isEquivalent(host, port, database, service, user) && _password == password))
  {
    return true;
  }
  return false;
}

bool operator==(const DBConnectionInfo& lhs, const DBConnectionInfo& rhs)
{
  if (&lhs == &rhs)
    return true;

  if (lhs._isExplorer == rhs._isExplorer && lhs._url == rhs._url && lhs._host == rhs._host &&
      lhs._port == rhs._port && lhs._database == rhs._database && lhs._service == rhs._service &&
      lhs._user == rhs._user && lhs._password == rhs._password)
  {
    return true;
  }
  return false;
}

DBPoolInfo::DBPoolInfo(const std::string& pool)
  : _pool(pool),
    _useExplorerConnection(false),
    _currentConnInfo(nullptr),
    _explorerConnInfo(nullptr),
    _currentConnInfoIndex(0)
{
}

DBPoolInfo::DBPoolInfo(const std::string& pool,
                       const std::string& explorerUrl,
                       const std::string& explorerHost,
                       uint16_t explorerPort,
                       const std::string& explorerDatabase,
                       const std::string& explorerService,
                       const std::string& explorerUser,
                       const std::string& explorerPassword)
  : _pool(pool),
    _useExplorerConnection(true),
    _currentConnInfo(nullptr),
    _explorerConnInfo(nullptr),
    _currentConnInfoIndex(0)
{
  _explorerConnInfo = new DBConnectionInfo(true,
                                           explorerUrl,
                                           explorerHost,
                                           explorerPort,
                                           explorerDatabase,
                                           explorerService,
                                           explorerUser,
                                           explorerPassword);
}

DBPoolInfo::~DBPoolInfo() { delete _explorerConnInfo; }

const DBConnectionInfo*
DBPoolInfo::getCurrentConnectionInfo() const
{
  TSEGuard g(_mutex);
  return _currentConnInfo;
}

bool
DBPoolInfo::setCurrentConnectionInfo(const DBConnectionInfo* connInfo)
{
  if (!connInfo)
  {
    LOG4CXX_ERROR(getLogger(),
                  "Logic error in : setCurrentConnectionInfo(): You can't "
                      << "use a null connection as the current connection "
                      << "for a pool. Pool=" << _pool);
    return false;
  }
  if (connInfo->isExplorer())
  {
    LOG4CXX_ERROR(getLogger(),
                  "Logic error in : setCurrentConnectionInfo(): You can't "
                      << "use an Explorer connection as the current connection "
                      << "for a pool. Pool=" << _pool);
    return false;
  }

  { // Lock scope
    TSEGuard g(_mutex);

    bool connFound(false);
    ConstConnectionInfoContainer::size_type i = 0;
    for (; i < _connections.size() - 1; ++i)
    {
      const DBConnectionInfo* conn = _connections.at(i);
      if (connInfo == conn)
      {
        LOG4CXX_DEBUG(getLogger(),
                      "In setCurrentConnectionInfo(): connInfo already exists"
                          << " in pool: " << _pool << ". Using existing connInfo object.");

        connFound = true;
        _currentConnInfoIndex = i;
        _currentConnInfo = conn;
        break;
      }
    }

    if (!connFound)
    {
      LOG4CXX_DEBUG(getLogger(),
                    "In setCurrentConnectionInfo(): Adding connInfo to "
                        << "pool: " << _pool);

      _connections.push_back(connInfo);
      _currentConnInfo = connInfo;
      _currentConnInfoIndex = _connections.size() - 1;
    }

    if (_currentConnInfo->port() != 0)
    {
      LOG4CXX_INFO(getLogger(),
                   "Current connection set to: "
                       << _currentConnInfo->user() << ":" << _currentConnInfo->password() << "@"
                       << _currentConnInfo->database() << "/" << _currentConnInfo->port()
                       << " for pool: " << pool());
    }
    else
    {
      LOG4CXX_INFO(getLogger(),
                   "Current connection set to: "
                       << _currentConnInfo->user() << ":" << _currentConnInfo->password() << "@"
                       << _currentConnInfo->service() << " for pool: " << pool());
    }

  } // Lock scope
  return true;
}

bool
DBPoolInfo::addConnectionInfo(const DBConnectionInfo* connInfo)
{
  if (connInfo)
  {
    if (connInfo->isExplorer())
    {
      LOG4CXX_ERROR(getLogger(),
                    "Logic error in : addConnectionInfo(): You can't "
                        << "add an Explorer connection as a pool connection. "
                        << "Pool=" << _pool);
      return false;
    }

    { // Lock scope
      TSEGuard g(_mutex);

      ConstConnectionInfoContainer::const_iterator iter = _connections.begin();
      ConstConnectionInfoContainer::const_iterator iterEnd = _connections.end();

      // Don't add a duplicate connection that's already in the container.
      //
      for (; iter != iterEnd; ++iter)
      {
        if (*iter == connInfo)
          return false;
      }
      _connections.push_back(connInfo);
      if (!_currentConnInfo)
      {
        _currentConnInfo = connInfo;
        _currentConnInfoIndex = _connections.size() - 1;
      }
    }
    return true;
  }

  LOG4CXX_ERROR(getLogger(),
                "Logic error in : addConnectionInfo(): Attempted to add NULL "
                    << "connection to Pool=" << _pool);
  return false;
}

bool
DBPoolInfo::processConnectionFailure(DBConnectionInfo* connInfo,
                                     DBConnectionInfoFactory* factory,
                                     bool resetPriorityOrder)
{
  // Concurrency control (mutex locking) is handled at a higher
  // enclosing scope.
  //
  if (isCurrentConnectionInfo(connInfo))
  {
    LOG4CXX_INFO(getLogger(), "Processing connection failure for pool: " << pool());

    if (useExplorerConnection())
    {
      LOG4CXX_INFO(getLogger(), "Using explorer connection for failover. Pool: " << pool());

      // Use the explorer connection to create a new connection.
      //
      DBConnectionInfo* newConn = factory->createConnInfo(this);
      if (newConn)
      {
        setCurrentConnectionInfo(newConn);
        return true;
      }
    }
    else
    {
      LOG4CXX_INFO(getLogger(), "Not using explorer connection for failover. Pool: " << pool());

      // Fail-over to the next connection in the pool's collection.
      //
      if (failoverToNextConnInfo(resetPriorityOrder))
      {
        return true;
      }
    }
    LOG4CXX_ERROR(getLogger(), "Failover was unsuccessful for pool: " << pool());

    return false;
  }
  return true;
}

const DBConnectionInfo*
DBPoolInfo::getExplorerConnectionInfo() const
{
  TSEGuard g(_mutex);
  return _explorerConnInfo;
}

bool
DBPoolInfo::setExplorerConnectionInfo(const DBConnectionInfo* connInfo)
{
  if (connInfo)
  {
    if (!connInfo->isExplorer())
    {
      LOG4CXX_ERROR(getLogger(),
                    "Logic error in : setExplorerConnectionInfo(): You can't "
                        << "use a regular connection as the Explorer connection "
                        << "for a pool. Pool=" << _pool);
      return false;
    }

    { // Lock scope
      TSEGuard g(_mutex);
      _explorerConnInfo = connInfo;
    }
  }
  return false;
}

bool
DBPoolInfo::isCurrentConnectionInfo(const DBConnectionInfo* connInfo) const
{
  TSEGuard g(_mutex);
  if (_currentConnInfo && _currentConnInfo == connInfo)
    return true;
  return false;
}

bool
DBPoolInfo::failoverToNextConnInfo(bool resetPriorityOrder)
{
  // Concurrency control (mutex locking) must be handled at a higher
  // enclosing scope.
  //

  // If resetPriorityOrder is true, then we go back to the highest
  // priority connection (the first one in the list) unless that's
  // the one that just failed.
  //
  if (resetPriorityOrder && _currentConnInfoIndex)
    _currentConnInfoIndex = 0;
  else if (_currentConnInfoIndex < _connections.size() - 1)
    ++_currentConnInfoIndex;
  else if (_connections.size() > 1)
    _currentConnInfoIndex = 0;
  else
    return false;

  _currentConnInfo = _connections.at(_currentConnInfoIndex);
  return true;
}

tse::Logger&
DBPoolInfo::getLogger()
{
  static tse::Logger logger("atseintl.DBAccess.DBPoolInfo");
  return logger;
}

} // DBAccess
