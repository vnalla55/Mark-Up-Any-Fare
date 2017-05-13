//----------------------------------------------------------------------------
//
//     File:           ORACLEAdapter.h
//     Description:    ORACLEAdapter
//     Created:
//     Authors:
//
//     Updates:
//
//     Copyright @2009, Sabre Inc.  All rights reserved.
//         This software/documentation is the confidential and proprietary
//         product of Sabre Inc.
//         Any unauthorized use, reproduction, or transfer of this
//         software/documentation, in any medium, or incorporation of this
//         software/documentation into any system or publication,
//         is strictly prohibited.
//----------------------------------------------------------------------------

#pragma once

#include "Common/ElapseTimeWatch.h"
#include "Common/TseSynchronizingValue.h"
#include "DBAccess/DBAdapter.h"

#include <iostream>
#include <map>
#include <string>
#include <vector>

#include <oci.h>

namespace DBAccess
{
class BoundParameter;
}

namespace tse
{

class ORACLEConnectionTimer;

std::string&
getOCIErrorString(int32_t status, OCIError* ociError, std::string& out);

int32_t
getOCIErrorNumber(int32_t status, OCIError* ociError);

class ElapseTimeWatch;
class SQLQuery;

class ORACLEAdapter final : public DBAdapter
{
private:
  class OCIDescriptorHandleHolder final
  {
  public:
    OCIDescriptorHandleHolder(void* handle, uint32_t type) : _handle(handle), _type(type) {}
    OCIDescriptorHandleHolder(OCIDescriptorHandleHolder&&) = default;

    OCIDescriptorHandleHolder(const OCIDescriptorHandleHolder&) = delete;
    OCIDescriptorHandleHolder& operator=(const OCIDescriptorHandleHolder&) = delete;

    void release() { OCIDescriptorFree(_handle, _type); }

  private:
    void* _handle;
    uint32_t _type;
  };

  std::string user;
  std::string pass;
  std::string database;
  std::string modulename;
  std::string host;
  int port = 0;
  bool compress = false;

  OCIEnv* _ociEnv = nullptr;
  OCIServer* _ociServer = nullptr;
  OCISvcCtx* _ociSvcCtx = nullptr;
  OCISession* _ociSession = nullptr;
  OCIStmt* _ociStmt = nullptr;
  OCIError* _ociError = nullptr;

  typedef std::vector<OCIDescriptorHandleHolder> HandleHolderCollection;
  HandleHolderCollection _descriptorHandles;

  static constexpr unsigned int _reconnectLimit = 5;
  std::string _databaseConnectGroup;
  bool _isHistorical{false};
  static unsigned int _databaseTimeoutValue;

  static boost::mutex _envMutex;
  static boost::mutex _mutex;
  static boost::mutex _syncMutex;
  static std::map<std::string, int> _dbConnections;
  void dumpDbConnections();

  bool shouldReconnect(int oraErrorCode);
  bool shouldDisconnect(int oraErrorCode);
  bool reconnect();

  void closeStatement();
  void closeEnvironment();
  void detachServer();
  void closeSession();

  uint32_t getStmtCacheSize();

  void cleanupDescriptorHandles();
  void clearDescriptorHandleCollection();

  bool checkSynchValue();

public:
  std::string getDatabase() { return database; }

  virtual void bindParameter(DBAccess::BoundParameter* parm) override;

  static const std::map<std::string, int>& dbConnections() { return  _dbConnections; }

  ORACLEAdapter();

  ~ORACLEAdapter();

  bool init(std::string& dbConnGroup,
            std::string& user,
            std::string& pass,
            std::string& database,
            std::string& host,
            int port,
            bool compress,
            bool isHistorical);

  bool isValid();

  static void modifyCurrentSynchValue();
  static TseSynchronizingValue getCurrentSynchValue();
  void resetSynchValue();
  static void dumpDbConnections(std::ostream& os);
  size_t alertSupressionInterval();
  int getConfigMaxNumberTries();

  OCIStmt* prepareQuery(SQLQuery* sqlQueryObject, unsigned int reconnectCount = 0);

  int32_t
  executeQuery(SQLQuery* sqlQueryObject, uint32_t arrayFetchSize, unsigned int reconnectCount = 0);

private:
  TseSynchronizingValue _localSynchValue;

  static TseSynchronizingValue _globalSynchValue;

  ElapseTimeWatch _etw;

  OCIStmt* reconnectAndPrepareQuery(SQLQuery* sqlQueryObject,
                                    unsigned int& reconnectCount,
                                    int32_t ociStatusCode,
                                    ElapseTimeWatch& etw);

  ORACLEConnectionTimer* _oracleConnectionTimer = nullptr;
  double databaseTimeoutValue();
  int setDatabaseTimeoutValue();
  size_t getConfigAlertSupressionInterval();
  bool useTimeoutDuringStartup();

public:
  void closeResultSet();

  OCIEnv* getOCIEnvironment() { return _ociEnv; }
  OCIStmt* getOCIStatement() { return _ociStmt; }
  OCIError* getOCIError() { return _ociError; }
  ORACLEConnectionTimer* oracleConnectionTimer();
};
}
