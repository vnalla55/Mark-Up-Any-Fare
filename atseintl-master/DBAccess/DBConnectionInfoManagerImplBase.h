//----------------------------------------------------------------------------
//
//     File:           DBConnectionInfoManagerImplBase.h
//     Description:    Database connection info manager abstract base class.
//     Created:        03/04/2010
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
//----------------------------------------------------------------------------

#pragma once

#include "Common/Logger.h"
#include "Common/Thread/TSEFastMutex.h"

#include <cstdio>
#include <list>
#include <map>
#include <string>
#include <vector>

#include <stdint.h>

namespace DBAccess
{
class DBConnectionInfoFactory;
//
// class DBConnectionInfo
//
// Encapsulates the connection info for one connection specification
// as defined in dbaccess.ini or dbconn.ini. An instance of this class
// will also be created by the "explorer connection" process if the
// connection specification is configured for automatic load balancing.
//
class DBConnectionInfo
{
public:
  DBConnectionInfo(bool isExplorer,
                   const std::string& url,
                   const std::string& host,
                   uint16_t port,
                   const std::string& database,
                   const std::string& service,
                   const std::string& user,
                   const std::string& password)
    : _isExplorer(isExplorer),
      _url(url),
      _host(host),
      _port(port),
      _database(database),
      _service(service),
      _user(user),
      _password(password),
      _failureCount(0)
  {
  }

  ~DBConnectionInfo() {}

  bool isExplorer() const { return _isExplorer; }

  const std::string& url() const { return _url; }
  const std::string& host() const { return _host; }
  uint16_t port() const { return _port; }
  const std::string& database() const { return _database; }
  const std::string& service() const { return _service; }
  const std::string& user() const { return _user; }
  const std::string& password() const { return _password; }

  void incrementFailureCount();

  bool isEquivalent(const std::string& host,
                    uint16_t port,
                    const std::string& database,
                    const std::string& service,
                    const std::string& user) const;

  bool isEquivalent(const std::string& host,
                    uint16_t port,
                    const std::string& database,
                    const std::string& service,
                    const std::string& user,
                    const std::string& password) const;

  friend bool operator==(const DBConnectionInfo& lhs, const DBConnectionInfo& rhs);

private:
  DBConnectionInfo();

  bool _isExplorer;

  std::string _url;
  std::string _host;
  uint16_t _port;
  std::string _database;
  std::string _service;
  std::string _user;
  std::string _password;

  uint16_t _failureCount;

  // Declare mutex as mutable so that const members can aquire/lock it.
  mutable TSEFastMutex _mutex;

}; // class DBConnectionInfo

typedef std::vector<DBConnectionInfo*> ConnectionInfoContainer;
typedef std::vector<const DBConnectionInfo*> ConstConnectionInfoContainer;

class DBPoolInfo
{
public:
  DBPoolInfo(const std::string& pool);
  DBPoolInfo(const std::string& pool,
             const std::string& explorerUrl,
             const std::string& explorerHost,
             uint16_t explorerPort,
             const std::string& explorerDatabase,
             const std::string& explorerService,
             const std::string& explorerUser,
             const std::string& explorerPassword);

  ~DBPoolInfo();

  const DBConnectionInfo* getCurrentConnectionInfo() const;
  bool setCurrentConnectionInfo(const DBConnectionInfo* connInfo);
  bool addConnectionInfo(const DBConnectionInfo* connInfo);

  bool processConnectionFailure(DBConnectionInfo* connInfo,
                                DBConnectionInfoFactory* factory,
                                bool resetPriorityOrder);

  const DBConnectionInfo* getExplorerConnectionInfo() const;
  bool setExplorerConnectionInfo(const DBConnectionInfo* connInfo);

  bool isCurrentConnectionInfo(const DBConnectionInfo* connInfo) const;

  const std::string& pool() const { return _pool; }
  bool useExplorerConnection() const { return _useExplorerConnection; }

private:
  DBPoolInfo();

  std::string _pool;
  bool _useExplorerConnection;

  ConstConnectionInfoContainer _connections;

  const DBConnectionInfo* _currentConnInfo;
  const DBConnectionInfo* _explorerConnInfo;

  uint32_t _currentConnInfoIndex;

  // Declare mutex as mutable so that const members can aquire/lock it.
  mutable TSEFastMutex _mutex;

  bool failoverToNextConnInfo(bool resetPriorityOrder);

  static tse::Logger& getLogger();

}; // class DBPoolInfo

class DBConnectionInfoFactory
{
public:
  DBConnectionInfoFactory() {}
  virtual ~DBConnectionInfoFactory() = 0;

  virtual DBConnectionInfo* createConnInfo(const DBPoolInfo* poolInfo) = 0;

}; // class DBConnectionInfoFactory

class DBConnectionInfoManagerImplBase : public DBConnectionInfoFactory
{
public:
  DBConnectionInfoManagerImplBase();
  virtual ~DBConnectionInfoManagerImplBase() = 0;

  virtual bool getConnectInfo(const std::string& pool,
                              std::string& host,
                              uint16_t& port,
                              std::string& database,
                              std::string& service,
                              std::string& user,
                              std::string& password,
                              bool isHistorical);

  virtual bool isCurrentConnectInfo(const std::string& pool,
                                    const std::string& host,
                                    uint16_t port,
                                    const std::string& database,
                                    const std::string& service,
                                    const std::string& user,
                                    const std::string& password);

  virtual void processConnectionFailure(const std::string& host,
                                        uint16_t port,
                                        const std::string& database,
                                        const std::string& service,
                                        const std::string& user,
                                        bool resetPriorityOrder);

  virtual bool clearConnectInfo();

protected:
  void addConnInfo(DBConnectionInfo* connInfo);

  DBConnectionInfo* getConnInfo(const std::string& host,
                                uint16_t port,
                                const std::string& database,
                                const std::string& service,
                                const std::string& user);

  virtual DBPoolInfo* createPoolInfo(const std::string& pool, bool isHistorical) = 0;

private:
  typedef std::map<std::string, DBPoolInfo*> PoolInfoMap;

  ConnectionInfoContainer _connections;
  PoolInfoMap _poolInfos;

  // Declare mutex as mutable so that const members can aquire/lock it.
  mutable TSEFastMutex _mutex;

  DBPoolInfo* getPoolInfo(const std::string& pool);

  static tse::Logger& getLogger();

}; // class DBConnectionInfoManagerImplBase

}; // namespace DBAccess

