//----------------------------------------------------------------------------
//
//     File:           ORACLEAdapter.cpp
//     Description:    ORACLEAdapter
//     Created:        01/21/2009
//     Authors:        Emad Girgis
//
//     Updates:
//
//     Copyright 2009, Sabre Inc.  All rights reserved.
//         This software/documentation is the confidential and proprietary
//         product of Sabre Inc.
//         Any unauthorized use, reproduction, or transfer of this
//         software/documentation, in any medium, or incorporation of this
//         software/documentation into any system or publication,
//         is strictly prohibited.
//----------------------------------------------------------------------------

#include "DBAccess/ORACLEAdapter.h"

#include "Allocator/TrxMalloc.h"
#include "Common/ErrorResponseException.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/TSEException.h"
#include "Common/TseSrvStats.h"
#include "Common/TseUtil.h"
#include "DBAccess/CacheManager.h"
#include "DBAccess/DBConnectionInfoManager.h"
#include "DBAccess/ORACLEBoundParameterTypes.h"
#include "DBAccess/ORACLEConnectionTimer.h"
#include "DBAccess/SQLQuery.h"
#include "DBAccess/RemoteCache/CurrentDatabase.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace DBAccess;
namespace tse
{
namespace
{

log4cxx::LoggerPtr getLogger()
{
  static log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("atseintl.DBAccess.ORACLEAdapter"));
  return logger;
}

bool
isDebugEnabled()
{
  static const bool enabled(IS_DEBUG_ENABLED(getLogger()));
  return enabled;
}

bool getCollectStatsCfg()
{
  bool collectStats(0);
  if (Global::hasConfig())
  {
    Global::config().getValue("COLLECT_QUERY_STATS", collectStats, "TSE_SERVER");
  }
  return collectStats;
}

struct QueryStats
{
  double _elapsed{0};
  long _calls{0};
};

typedef std::map<std::string, QueryStats> QueryStatsMap;
QueryStatsMap queryStatsMap;
std::mutex queryStatsMutex;

void collectStats(SQLQuery* sqlQueryObject,
                  double seconds)
{
  static const bool collect(getCollectStatsCfg());
  if (collect && sqlQueryObject)
  {
    const std::string& queryName(sqlQueryObject->getQueryName());
    std::lock_guard<std::mutex> lock(queryStatsMutex);
    auto pr(queryStatsMap.emplace(queryName, QueryStats()));
    ++pr.first->second._calls;
    pr.first->second._elapsed += seconds;
  }
}

}// namespace

void*
oracle_malloc(void* ctxp, size_t size)
{
  const MallocContextDisabler context;
  return malloc(size);
}

void*
oracle_realloc(void* ctxp, void* memptr, size_t size)
{
  const MallocContextDisabler context;
  return realloc(memptr, size);
}

void
oracle_free(void* ctxp, void* memptr)
{
  const MallocContextDisabler context;
  free(memptr);
}

std::string&
getOCIErrorString(int32_t status, OCIError* ociError, std::string& out)
{
  text buf[512];
  sb4 errorCode = 0;
  char errorCodeString[10];

  switch (status)
  {
  case OCI_SUCCESS:
    break;
  case OCI_SUCCESS_WITH_INFO:
    out = "OCI_SUCCESS_WITH_INFO";
    break;
  case OCI_NEED_DATA:
    out = "OCI_NEED_DATA";
    break;
  case OCI_NO_DATA:
    out = "OCI_NODATA";
    break;
  case OCI_ERROR:
    out = "OCI_ERROR: ";
    if (ociError)
    {
      OCIErrorGet(ociError, 1, (text*)nullptr, &errorCode, buf, sizeof(buf), OCI_HTYPE_ERROR);
      snprintf(errorCodeString, sizeof(errorCodeString), "%d ", errorCode);
      out.append(errorCodeString);
      out.append((char*)buf);
    }
    break;
  case OCI_INVALID_HANDLE:
    out = "OCI_INVALID_HANDLE";
    break;
  case OCI_STILL_EXECUTING:
    out = "OCI_STILL_EXECUTING";
    break;
  case OCI_CONTINUE:
    out = "OCI_CONTINUE";
    break;
  default:
    out = "Unknown OCI error";
    break;
  }
  return out;
}

int32_t
getOCIErrorNumber(int32_t status, OCIError* ociError)
{
  text buf[512];
  sb4 errorCode = 0;

  if (status != OCI_SUCCESS && ociError)
  {
    OCIErrorGet(ociError, 1, (text*)nullptr, &errorCode, buf, sizeof(buf), OCI_HTYPE_ERROR);
  }
  return errorCode;
}

unsigned int ORACLEAdapter::_databaseTimeoutValue = 0;

static constexpr double longQueryThreshold = 1.0;

TseSynchronizingValue ORACLEAdapter::_globalSynchValue;

boost::mutex ORACLEAdapter::_envMutex;
boost::mutex ORACLEAdapter::_mutex;
boost::mutex ORACLEAdapter::_syncMutex;
std::map<std::string, int> ORACLEAdapter::_dbConnections;

ORACLEAdapter::ORACLEAdapter()
{
  resetSynchValue();
  if (databaseTimeoutValue())
  {
    _oracleConnectionTimer =
        new ORACLEConnectionTimer(databaseTimeoutValue(), _ociSession, _ociError);
    LOG4CXX_DEBUG(getLogger(),
                  "Start timer with millisecond timeout ["
                      << (long)databaseTimeoutValue() << "] at[" << _oracleConnectionTimer << "]");
  }
}

ORACLEConnectionTimer*
ORACLEAdapter::oracleConnectionTimer()
{
  return CacheManager::initialized() || useTimeoutDuringStartup() ? _oracleConnectionTimer : nullptr;
}

size_t
ORACLEAdapter::getConfigAlertSupressionInterval()
{
  ConfigMan& config = Global::config();
  std::string _name("DATABASE_TUNING");
  size_t alertInterval = 300;

  config.getValue("DATABASE_ALERT_SUPPRESSION_INTERVAL", alertInterval, _name);

  LOG4CXX_INFO(getLogger(),
               "TseCVG DB Timeout Alert Supression interval in seconds [" << alertInterval << "]");
  return alertInterval;
}

int
ORACLEAdapter::getConfigMaxNumberTries()
{
  ConfigMan& config(Global::config());
  int numberTries(1);

  config.getValue("QUERY_MAX_NUMBER_TRIES", numberTries, "DATABASE_TUNING");

  if (numberTries < 1)
  {
    numberTries = 1;
  }

  LOG4CXX_INFO(getLogger(), "Query maximum number tries is " << numberTries);
  return numberTries;
}

size_t
ORACLEAdapter::alertSupressionInterval()
{
  static size_t _alertInterval = getConfigAlertSupressionInterval();
  return _alertInterval;
}

bool
ORACLEAdapter::useTimeoutDuringStartup()
{
  static bool useTimeoutDuringStartup = setDatabaseTimeoutValue();
  return useTimeoutDuringStartup;
}

// Since existing clock value assumes a double with seconds the whole
// part and fraction of seconds decimal part. . . calculate millisecond
// fractional equivalent
double
ORACLEAdapter::databaseTimeoutValue()
{
  static unsigned int timeout = setDatabaseTimeoutValue();
  static double fractional = (1.0 * timeout) / 1000.0;
  return fractional;
}

int
ORACLEAdapter::setDatabaseTimeoutValue()
{
  ConfigMan& config = Global::config();
  std::string _name("DATABASE_TUNING");
  // get config parameters
  if (!config.getValue("DATABASE_TIMEOUT_VALUE", _databaseTimeoutValue, _name))
  {
    LOG4CXX_DEBUG(getLogger(), "Couldn't find config entry for 'DATABASE_TIMEOUT_VALUE' default to "
                               "disabled with value == 0 ");
    _databaseTimeoutValue = 0;
  }
  LOG4CXX_INFO(getLogger(),
               "Initialize databaseTimeoutValue to millisecond timeout ["
                   << (long)_databaseTimeoutValue << "]");
  return _databaseTimeoutValue;
}

void
ORACLEAdapter::modifyCurrentSynchValue()
{
  boost::lock_guard<boost::mutex> g(_syncMutex);
  _globalSynchValue++;
}

TseSynchronizingValue
ORACLEAdapter::getCurrentSynchValue()
{
  boost::lock_guard<boost::mutex> g(_syncMutex);
  return _globalSynchValue;
}

void
ORACLEAdapter::resetSynchValue()
{
  boost::lock_guard<boost::mutex> g(_syncMutex);
  _localSynchValue = _globalSynchValue;
}

bool
ORACLEAdapter::checkSynchValue()
{
  boost::lock_guard<boost::mutex> g(_syncMutex);
  return (_localSynchValue == _globalSynchValue);
}

bool
ORACLEAdapter::isValid()
{
  if (UNLIKELY(!_ociEnv || !_ociSvcCtx || !_ociServer || !checkSynchValue()))
  {
    return false;
  }
  if (UNLIKELY(!DBConnectionInfoManager::isCurrentConnectInfo(
          _databaseConnectGroup, host, port, "", database, user, pass)))
  {
    return false;
  }
  return true;
}

static log4cxx::LoggerPtr&
getLoggerForLongQueries()
{
  static log4cxx::LoggerPtr logger(
      log4cxx::Logger::getLogger("atseintl.DBAccess.ORACLEAdapter.LongQuery"));
  return logger;
}

OCIStmt*
ORACLEAdapter::prepareQuery(SQLQuery* sqlQueryObject, unsigned int reconnectCount)
{
  if (UNLIKELY(!_ociEnv))
  {
    reconnectCount++;
    if (reconnectCount <= _reconnectLimit && reconnect())
    {
      return prepareQuery(sqlQueryObject, reconnectCount);
    }
  }

  try
  {
    _etw.stop();
    _etw.start();

    if (isDebugEnabled())
    {
      LOG4CXX_DEBUG(getLogger(), sqlQueryObject->getQueryName() << " | " << sqlQueryObject->c_str());
    }
    int32_t status = 0;
    std::string ociErrMsg;

    status = OCIStmtPrepare2(_ociSvcCtx,
                             &_ociStmt,
                             _ociError,
                             (const unsigned char*)sqlQueryObject->c_str(),
                             sqlQueryObject->length(),
                             nullptr,
                             0,
                             OCI_NTV_SYNTAX,
                             OCI_DEFAULT);

    if (UNLIKELY(status != OCI_SUCCESS))
    {
      LOG4CXX_ERROR(getLogger(),
                    "Oracle error preparing Statement. "
                        << "SERVICE_NAME: " << this->database << " USER: " << this->user
                        << " QUERY_NAME: " << sqlQueryObject->getQueryName()
                        << " Error: " << getOCIErrorString(status, _ociError, ociErrMsg));
      closeStatement();

      return reconnectAndPrepareQuery(sqlQueryObject, reconnectCount, status, _etw);
    }

    if (isDebugEnabled())
    {
      LOG4CXX_DEBUG(getLogger(), "Statement prepared");
    }

    sqlQueryObject->bindAllParameters();

    return _ociStmt;
  }
  catch (const TSEException& e)
  {
    _etw.stop();

    closeStatement();
    closeSession();
    detachServer();
    closeEnvironment();

    throw;
  }
  catch (...)
  {
    _etw.stop();

    LOG4CXX_ERROR(getLogger(),
                  "Unknown exception caught while preparing query "
                      << sqlQueryObject->getQueryName() << " Closing database connection. "
                      << "SERVICE_NAME: " << this->database << " USER: " << this->user);

    closeStatement();
    closeSession();
    detachServer();
    closeEnvironment();

    throw TSEException(TSEException::NO_ERROR, "Database error");
  }
}

int32_t
ORACLEAdapter::executeQuery(SQLQuery* sqlQueryObject,
                            uint32_t arrayFetchSize,
                            unsigned int reconnectCount)
{
  try
  {
    int32_t status = 0;
    std::string ociErrMsg;

    //
    // Enable prefetching for the statement execution.
    // We will turn it off prior to executing any subsequent fetch
    //  operations to avoid the extra data copy between the OCI
    //  internal prefetch buffers and our array fetch buffers.
    // This was recommended by the Oracle OCI engineers.
    //
    uint32_t prefetchRowCount = 0;
    if (arrayFetchSize == 0)
    {
      prefetchRowCount = 10;
    }

    OCIAttrSet(_ociStmt, OCI_HTYPE_STMT, &prefetchRowCount, 0, OCI_ATTR_PREFETCH_ROWS, _ociError);

    status = OCIStmtExecute(_ociSvcCtx,
                            _ociStmt,
                            _ociError,
                            arrayFetchSize,
                            0,
                            (CONST OCISnapshot*)nullptr,
                            (OCISnapshot*)nullptr,
                            OCI_DEFAULT);
    if (UNLIKELY(status != OCI_SUCCESS && status != OCI_NO_DATA))
    {
      std::string queryName;
      std::string sqlString;
      std::string parameterString;
      if (sqlQueryObject != nullptr)
      {
        queryName = sqlQueryObject->getQueryName();
        sqlQueryObject->getSQLString(sqlString);
        sqlQueryObject->fillParameterString(parameterString);
      }
      LOG4CXX_ERROR(getLogger(),
                    "Oracle error executing Statement. "
                    << "SERVICE_NAME: " << this->database << " USER: " << this->user
                    << " Error: " << getOCIErrorString(status, _ociError, ociErrMsg)
                    << " QUERY_NAME: " << queryName
                    << "\nSQLString: " << sqlString
                    << "\nparameters:" << parameterString);
      closeStatement();

      if (shouldReconnect(status))
      {
        return status;
      }
      else
      {
        throw TSEException(TSEException::NO_ERROR, "Database error");
      }
    }

    if (isDebugEnabled())
    {
      LOG4CXX_DEBUG(getLogger(), "Statement executed");
    }

    // Turn off prefetching
    //
    if (prefetchRowCount)
    {
      prefetchRowCount = 0;
      OCIAttrSet(_ociStmt, OCI_HTYPE_STMT, &prefetchRowCount, 0, OCI_ATTR_PREFETCH_ROWS, _ociError);
    }
    uint32_t prefetchMemory = 0;
    OCIAttrSet(_ociStmt, OCI_HTYPE_STMT, &prefetchMemory, 0, OCI_ATTR_PREFETCH_MEMORY, _ociError);

    _etw.stop();
    TseSrvStats::recordDBCall(_etw.elapsedTime(), true);

    collectStats(sqlQueryObject, _etw.elapsedTime());
    if (_etw.elapsedTime() > longQueryThreshold)
    {
      if (isDebugEnabled())
      {
        LOG4CXX_DEBUG(getLoggerForLongQueries(),
                      "Long query: " << _etw.elapsedTime() << " seconds " << sqlQueryObject->c_str());
      }
    }

    sqlQueryObject->clearBoundParameters();

    // LOG4CXX_ERROR(getLogger(), sqlQueryObject->getQueryName() << ":ElapsedTime=" <<
    // _etw.elapsedTime());

    return status;
  }
  catch (const TSEException& e)
  {
    _etw.stop();

    closeStatement();
    closeSession();
    detachServer();
    closeEnvironment();

    throw;
  }
  catch (...)
  {
    _etw.stop();

    LOG4CXX_ERROR(getLogger(),
                  "Unknown exception caught while executing query "
                      << sqlQueryObject->getQueryName() << " Closing database connection. "
                      << "SERVICE_NAME: " << this->database << " USER: " << this->user);

    closeStatement();
    closeSession();
    detachServer();
    closeEnvironment();

    throw TSEException(TSEException::NO_ERROR, "Database error");
  }
}

OCIStmt*
ORACLEAdapter::reconnectAndPrepareQuery(SQLQuery* sqlQueryObject,
                                        unsigned int& reconnectCount,
                                        int32_t ociStatusCode,
                                        ElapseTimeWatch& etw)
{
  static DateTime lastReportedAlert;
  int32_t errorCode = getOCIErrorNumber(ociStatusCode, _ociError);
  if (shouldReconnect(errorCode))
  {
    reconnectCount++;
    if (reconnectCount <= _reconnectLimit && reconnect())
    {
      return prepareQuery(sqlQueryObject, reconnectCount);
    }
    else
    {
      closeSession();
      detachServer();
      closeEnvironment();

      if (_databaseConnectGroup.empty())
      {
        LOG4CXX_ERROR(getLogger(), "Database connection lost");
      }
      else
      {
        std::ostringstream o;
        o << "Database connect error.  DBGroup: " << _databaseConnectGroup
          << " SERVICE_NAME: " << this->database << " USER: " << this->user;
        LOG4CXX_ERROR(getLogger(), o.str());
        DateTime now(DateTime::localTime());
        if (now.subtractSeconds(alertSupressionInterval()) > lastReportedAlert)
        {
          lastReportedAlert = now;
          TseUtil::alert(o.str().c_str());
        }
        LOG4CXX_ERROR(getLogger(), o.str());
        throw ErrorResponseException(ErrorResponseException::DB_TIMEOUT_ERROR,
                                     "Database query timeout");
      }
      if ((CacheManager::initialized() || useTimeoutDuringStartup()) && databaseTimeoutValue() &&
          etw.elapsedTime() > databaseTimeoutValue())
      {
        std::ostringstream o;
        o << "Query Timeout. "
          << "SERVICE_NAME: " << this->database << " USER: " << this->user
          << " SQL: " << sqlQueryObject->c_str();
        DateTime now(DateTime::localTime());
        if (now.subtractSeconds(alertSupressionInterval()) > lastReportedAlert)
        {
          lastReportedAlert = now;
          TseUtil::alert(o.str().c_str());
        }
        LOG4CXX_ERROR(getLogger(), o.str());
        throw TSEException(TSEException::NO_ERROR, "Database query timeout");
      }
      else
      {
        throw TSEException(TSEException::NO_ERROR, "Database connection lost");
      }
    }
  }
  else if (shouldDisconnect(errorCode))
  {
    if (_databaseConnectGroup.empty())
    {
      LOG4CXX_ERROR(getLogger(), "Database error. Closing database connection.");
    }
    else
    {
      LOG4CXX_ERROR(getLogger(),
                    "Database error. DBGroup: "
                        << _databaseConnectGroup << " Closing database connection. "
                        << "SERVICE_NAME: " << this->database << " USER: " << this->user);

      DBConnectionInfoManager::processConnectionFailure("", 0, "", this->database, this->user);

      if (checkSynchValue())
      {
        modifyCurrentSynchValue();
      }
    }

    closeSession();
    detachServer();
    closeEnvironment();

    throw TSEException(TSEException::NO_ERROR, "Database error");
  }
  else
  {
    std::string queryName;
    std::string sqlString;
    std::string parameterString;
    if (sqlQueryObject != nullptr)
    {
      queryName = sqlQueryObject->getQueryName();
      sqlQueryObject->getSQLString(sqlString);
      sqlQueryObject->fillParameterString(parameterString);
    }
    if (_databaseConnectGroup.empty())
    {
      LOG4CXX_ERROR(getLogger(), "Query failed. "
                    << "SERVICE_NAME: " << this->database
                    << " QUERY_NAME: " << queryName
                    << "\nSQLString: " << sqlString
                    << "\nparameters:" << parameterString);
    }
    else
    {
      LOG4CXX_ERROR(getLogger(), "Query failed. DBGroup: "
                    << _databaseConnectGroup
                    << " SERVICE_NAME: " << this->database
                    << " USER: " << this->user
                    << " QUERY_NAME: " << queryName
                    << "\nSQLString: " << sqlString
                    << "\nparameters:" << parameterString);
    }
    throw TSEException(TSEException::NO_ERROR, "Database error");
  }
}

bool
ORACLEAdapter::init(std::string& dbConnGroup,
                    std::string& user,
                    std::string& pass,
                    std::string& database,
                    std::string& host,
                    int port,
                    bool compress,
                    bool isHistorical)
{
  // No user or no password? Try to connect with oracle wallet.
  bool useOracleWallet(!user.length() || !pass.length());
  std::string authentication("wallet");

  closeStatement();
  closeSession();
  detachServer();
  closeEnvironment();

  this->_databaseConnectGroup = dbConnGroup;
  this->user = user;
  this->pass = pass;
  this->database = database;
  this->modulename = "ATSEv2"; // may need to make this a config file option

  // the following three dosn't mean much to oracle
  this->host = host;
  this->port = port;
  this->compress = compress;
  _isHistorical = isHistorical;

  try
  {
    int32_t status = 0;
    std::string ociErrMsg;

    {
      boost::lock_guard<boost::mutex> g(_envMutex);
      /*
      status = OCIEnvCreate(&_ociEnv, OCI_THREADED | OCI_NO_MUTEX, 0,
                            //oracle_malloc, oracle_realloc, oracle_free,
                            0, 0, 0,
                            0, (void **) 0);
      */
      status = OCIEnvCreate(&_ociEnv, OCI_THREADED | OCI_NO_MUTEX, nullptr, nullptr, nullptr, nullptr, 0, (void**)nullptr);
    }

    if (status != OCI_SUCCESS)
    {
      TseSrvStats::recordDBCall(0, false);
      LOG4CXX_ERROR(getLogger(), "Error initializing OCI Environment handle: " << status);
      return false;
    }

    // Allocate the OCI error handle
    OCIHandleAlloc(_ociEnv, (void**)&_ociError, OCI_HTYPE_ERROR, 0, (void**)nullptr);

    // Allocate the OCI Server handle
    OCIHandleAlloc(_ociEnv, (void**)&_ociServer, OCI_HTYPE_SERVER, 0, (void**)nullptr);

    // Allocate the OCI Service Context handle
    OCIHandleAlloc(_ociEnv, (void**)&_ociSvcCtx, OCI_HTYPE_SVCCTX, 0, (void**)nullptr);

    // Attach to the server
    status = OCIServerAttach(
        _ociServer, _ociError, (text*)this->database.c_str(), this->database.size(), OCI_DEFAULT);
    if (status != OCI_SUCCESS)
    {
      TseSrvStats::recordDBCall(0, false);
      LOG4CXX_ERROR(getLogger(),
                    "Oracle error attaching to database server. "
                        << "SERVICE_NAME: " << this->database << " USER: " << this->user
                        << " Error: " << getOCIErrorString(status, _ociError, ociErrMsg));
      _ociServer = nullptr;
      closeEnvironment();
      return false;
    }

    {
      boost::lock_guard<boost::mutex> g(_mutex);
      int numberConnections(++_dbConnections[database]);
      RemoteCache::recordCurrentDatabase(database, _isHistorical, numberConnections);
      dumpDbConnections();
    }

    // Set the Server handle attribute in the Service Context
    OCIAttrSet(_ociSvcCtx, OCI_HTYPE_SVCCTX, _ociServer, 0, OCI_ATTR_SERVER, _ociError);

    // Allocate a Session handle
    OCIHandleAlloc(_ociEnv, (void**)&_ociSession, OCI_HTYPE_SESSION, 0, (void**)nullptr);

    // Set the username attribute in the Session
    OCIAttrSet(_ociSession,
               OCI_HTYPE_SESSION,
               (void*)this->user.c_str(),
               this->user.size(),
               OCI_ATTR_USERNAME,
               _ociError);

    // Set the password attribute in the Session
    OCIAttrSet(_ociSession,
               OCI_HTYPE_SESSION,
               (void*)this->pass.c_str(),
               this->pass.size(),
               OCI_ATTR_PASSWORD,
               _ociError);

    // Set the module name attribute in the Session. This helps
    //  identify the application in AWR reports.
    OCIAttrSet(_ociSession,
               OCI_HTYPE_SESSION,
               (void*)this->modulename.c_str(),
               this->modulename.size(),
               OCI_ATTR_MODULE,
               _ociError);

    // Begin the Session
    if (useOracleWallet)
    {
      status = OCISessionBegin(_ociSvcCtx, _ociError, _ociSession, OCI_CRED_EXT, OCI_DEFAULT);
    }
    else
    {
      authentication = "user password";
      status = OCISessionBegin(_ociSvcCtx, _ociError, _ociSession, OCI_CRED_RDBMS, OCI_DEFAULT);
    }

    if (status != OCI_SUCCESS)
    {
      TseSrvStats::recordDBCall(0, false);
      if (useOracleWallet)
      {
        LOG4CXX_ERROR(getLogger(),
                      "Oracle error establishing database session. "
                          << "SERVICE_NAME: " << this->database);
        LOG4CXX_ERROR(
            getLogger(),
            "\n------------------------------------------------------------------------\n"
            "Attempted to connect to database using oracle wallet because no userid and\n"
            "or no password were specified.\n"
            "Is sqlnet.ora in the TNS_ADMIN path configured to include the following:\n"
            "WALLET_LOCATION =\n"
            "   (SOURCE =\n"
            "     (METHOD = FILE)\n"
            "     (METHOD_DATA =\n"
            "       (DIRECTORY = /path/to/wallet )\n"
            "     )\n"
            "   )\n\n"
            "SQLNET.WALLET_OVERRIDE = TRUE\n"
            "SSL_CLIENT_AUTHENTICATION = FALSE\n"
            "SSL_VERSION = 0\n"
            "------------------------------------------------------------------------\n"
            "Is there a wallet in a directory specified in WALLET_LOCATION that is\n"
            "readable by your effective userid? Does it have an entry for the database\n"
            "["
                << this->database << "] specified.\n"
                << "------------------------------------------------------------------------\n");
      }
      else
        LOG4CXX_ERROR(getLogger(),
                      "Oracle error establishing database session. "
                          << "SERVICE_NAME: " << this->database << " USER: " << this->user
                          << " Error: " << getOCIErrorString(status, _ociError, ociErrMsg));

      detachServer();
      closeEnvironment();
      return false;
    }

    // Set the Session attribute in the Service Context
    OCIAttrSet((void*)_ociSvcCtx, OCI_HTYPE_SVCCTX, _ociSession, 0, OCI_ATTR_SESSION, _ociError);

    // Set the statement cache size for the Service Context
    uint32_t stmtCacheSize = getStmtCacheSize();
    OCIAttrSet(
        (void*)_ociSvcCtx, OCI_HTYPE_SVCCTX, &stmtCacheSize, 0, OCI_ATTR_STMTCACHESIZE, _ociError);

    OCIStmt* stmt;

    std::string sessionSQL("alter session set TIME_ZONE='+00:00' "
                           "NLS_TIMESTAMP_FORMAT='YYYY-MM-DD:HH24:MI:SS.FF6' "
                           "NLS_DATE_FORMAT='YYYY-MM-DD:HH24:MI:SS'");

    status = OCIStmtPrepare2(_ociSvcCtx,
                             &stmt,
                             _ociError,
                             (const unsigned char*)sessionSQL.c_str(),
                             sessionSQL.length(),
                             nullptr,
                             0,
                             OCI_NTV_SYNTAX,
                             OCI_DEFAULT);

    if (status != OCI_SUCCESS)
    {
      TseSrvStats::recordDBCall(0, false);
      LOG4CXX_ERROR(getLogger(),
                    "Oracle error preparing session attributes. "
                        << "SERVICE_NAME: " << this->database << " USER: " << this->user
                        << " Error: " << getOCIErrorString(status, _ociError, ociErrMsg));

      status = OCIStmtRelease(stmt, _ociError, nullptr, 0, OCI_DEFAULT);
      if (status != OCI_SUCCESS)
      {
        LOG4CXX_ERROR(getLogger(),
                      "Oracle error releasing statement handle. "
                          << "SERVICE_NAME: " << this->database << " USER: " << this->user
                          << " Error: " << getOCIErrorString(status, _ociError, ociErrMsg));
      }
      closeSession();
      detachServer();
      closeEnvironment();
      return false;
    }

    status = OCIStmtExecute(_ociSvcCtx,
                            stmt,
                            _ociError,
                            1,
                            0,
                            (CONST OCISnapshot*)nullptr,
                            (OCISnapshot*)nullptr,
                            OCI_DEFAULT);

    if (status != OCI_SUCCESS)
    {
      TseSrvStats::recordDBCall(0, false);
      LOG4CXX_ERROR(getLogger(),
                    "Oracle error executing session attributes. "
                        << "SERVICE_NAME: " << this->database << " USER: " << this->user
                        << " Error: " << getOCIErrorString(status, _ociError, ociErrMsg));

      status = OCIStmtRelease(stmt, _ociError, nullptr, 0, OCI_DEFAULT);
      if (status != OCI_SUCCESS)
      {
        LOG4CXX_ERROR(getLogger(),
                      "Oracle error releasing statement handle. "
                          << "SERVICE_NAME: " << this->database << " USER: " << this->user
                          << " Error: " << getOCIErrorString(status, _ociError, ociErrMsg));
      }
      closeSession();
      detachServer();
      closeEnvironment();
      return false;
    }

    status = OCIStmtRelease(stmt, _ociError, nullptr, 0, OCI_DEFAULT);
    if (status != OCI_SUCCESS)
    {
      LOG4CXX_ERROR(getLogger(),
                    "Oracle error releasing statement handle. "
                        << "SERVICE_NAME: " << this->database << " USER: " << this->user
                        << " Error: " << getOCIErrorString(status, _ociError, ociErrMsg));
    }
  }
  catch (...)
  {
    TseSrvStats::recordDBCall(0, false);
    LOG4CXX_ERROR(getLogger(),
                  "Unknown exception caught while connecting to database. "
                      << "SERVICE_NAME: " << this->database << " USER: " << this->user);
    closeSession();
    detachServer();
    closeEnvironment();
    return false;
  }
  std::string message("Connected to ORACLE database using authentication [");
  message += authentication + "] SERVICE_NAME: " + this->database;
  if (!useOracleWallet)
    message += " USER: " + this->user;

  LOG4CXX_INFO(getLogger(), message);

  return true;
}

ORACLEAdapter::~ORACLEAdapter()
{
  LOG4CXX_INFO(getLogger(), "Closing connection to ORACLE database: " << database);

  try
  {
    if (_oracleConnectionTimer)
    {
      _oracleConnectionTimer->finish();
      delete _oracleConnectionTimer;
      _oracleConnectionTimer = nullptr;
    }

    closeStatement();
    closeSession();
    detachServer();
    closeEnvironment();

  }
  catch (...) { LOG4CXX_ERROR(getLogger(), "Unknown exception in ~ORACLEAdapter()"); }
}

bool
ORACLEAdapter::shouldReconnect(int oraErrorCode)
{
  // Return true only if we have to reconnect in order to
  //  recover from this error. If we can continue without
  //  reconnecting, then return false.
  //
  if (oraErrorCode == 3113) // ORA-03113: end-of-file on communication channel
  {
    return true;
  }
  else if (oraErrorCode == 3114) // ORA-03114: not connected to ORACLE
  {
    return true;
  }
  else if (oraErrorCode == 28) // ORA-00028: your session has been killed
  {
    return true;
  }
  else if (oraErrorCode == 1012) // ORA-01012: not logged on
  {
    return true;
  }
  else if (oraErrorCode == 12537) // ORA-12537: TNS:connection closed
  {
    return true;
  }
  else if (oraErrorCode == 12571) // ORA-12571: TNS:packet writer failure
  {
    return true;
  }
  else if (oraErrorCode == 3135) // ORA-03135: connection lost contact
  {
    return true;
  }
  /*
  else if (oraErrorCode == 12170) // ORA-12170: TNS:Connect timeout occurred
  {
    return true;
  }
  else if (oraErrorCode == 12528) // ORA-12528: TNS:listener: all appropriate
                                  //   instances are blocking new connections
  {
    return true;
  }
  */
  return false;
}

bool
ORACLEAdapter::shouldDisconnect(int oraErrorCode)
{
  // Return false if we can recover from this error without
  //  disconnecting. If we don't know, then this connection
  //  is in an unknown state and the only safe thing to do
  //  is close the connection.
  //
  if (shouldReconnect(oraErrorCode))
  {
    return false;
  }
  if (oraErrorCode >= 900 && oraErrorCode <= 999)
  {
    return false;
  }
  return true;
}

bool
ORACLEAdapter::reconnect()
{
  return init(_databaseConnectGroup, user, pass, database, host, port, compress, _isHistorical);
}

void
ORACLEAdapter::closeResultSet()
{
  cleanupDescriptorHandles();
  closeStatement();
}

void
ORACLEAdapter::closeStatement()
{
  if (_ociStmt)
  {
    int32_t status = OCIStmtRelease(_ociStmt, _ociError, nullptr, 0, OCI_DEFAULT);
    if (UNLIKELY(status != OCI_SUCCESS))
    {
      std::string ociErrMsg;
      LOG4CXX_ERROR(getLogger(),
                    "Oracle error releasing statement handle. "
                        << "SERVICE_NAME: " << this->database << " USER: " << this->user
                        << " Error: " << getOCIErrorString(status, _ociError, ociErrMsg));
    }
    _ociStmt = nullptr;
  }
}

void
ORACLEAdapter::closeEnvironment()
{
  clearDescriptorHandleCollection();

  if (_ociSvcCtx)
  {
    OCIHandleFree(_ociSvcCtx, OCI_HTYPE_SVCCTX);
    _ociSvcCtx = nullptr;
  }
  if (_ociError)
  {
    OCIHandleFree(_ociError, OCI_HTYPE_ERROR);
    _ociError = nullptr;
  }
  if (_ociEnv)
  {
    OCIHandleFree(_ociEnv, OCI_HTYPE_ENV);
    _ociEnv = nullptr;
  }
}

void
ORACLEAdapter::detachServer()
{
  if (_ociServer)
  {
    int32_t status;
    std::string msg;

    status = OCIServerDetach(_ociServer, _ociError, OCI_DEFAULT);
    if (status != OCI_SUCCESS)
    {
      LOG4CXX_ERROR(getLogger(),
                    "Oracle error detaching from database server. "
                        << "SERVICE_NAME: " << this->database << " USER: " << this->user
                        << " Error: " << getOCIErrorString(status, _ociError, msg));
    }
    OCIHandleFree(_ociServer, OCI_HTYPE_SERVER);
    _ociServer = nullptr;

    {
      boost::lock_guard<boost::mutex> g(_mutex);
      int numberConnections(--_dbConnections[this->database]);
      RemoteCache::recordCurrentDatabase(database, _isHistorical, numberConnections);
      dumpDbConnections();
    }
  }
}

void
ORACLEAdapter::closeSession()
{
  if (_ociSession)
  {
    int32_t status;
    std::string msg;

    status = OCISessionEnd(_ociSvcCtx, _ociError, _ociSession, OCI_DEFAULT);
    if (status != OCI_SUCCESS)
    {
      LOG4CXX_ERROR(getLogger(),
                    "Oracle error ending session. "
                        << "SERVICE_NAME: " << this->database << " USER: " << this->user
                        << " Error: " << getOCIErrorString(status, _ociError, msg));
    }
    OCIHandleFree(_ociSession, OCI_HTYPE_SESSION);
    _ociSession = nullptr;
  }
}

uint32_t
ORACLEAdapter::getStmtCacheSize()
{
  // TODO: Get from config file
  return 500;
}

void
ORACLEAdapter::cleanupDescriptorHandles()
{
  for (auto& handle : _descriptorHandles)
    handle.release();

  clearDescriptorHandleCollection();
}

void
ORACLEAdapter::clearDescriptorHandleCollection()
{
  _descriptorHandles.clear();
}

void
ORACLEAdapter::bindParameter(BoundParameter* parm)
{
  ORACLEBoundString* boundString = nullptr;
  ORACLEBoundInteger* boundInteger = nullptr;
  ORACLEBoundLong* boundLong = nullptr;
  ORACLEBoundDate* boundDate = nullptr;
  ORACLEBoundDateTime* boundDateTime = nullptr;

  if ((boundString = dynamic_cast<ORACLEBoundString*>(parm)))
  {
#ifdef DEBUG
    LOG4CXX_DEBUG(getLogger(),
                  "Binding query variable # "
                      << parm->index()
                      << " of type: SQLT_STR (string) size: " << boundString->getBindBufferSize()
                      << " with value: " << boundString->getBindBuffer());
#endif

    OCIBind* ociBindHandle = nullptr;
    int32_t status = OCIBindByPos(_ociStmt,
                                  &ociBindHandle,
                                  _ociError,
                                  parm->index(),
                                  boundString->getBindBuffer(),
                                  boundString->getBindBufferSize(),
                                  SQLT_STR,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  0,
                                  nullptr,
                                  OCI_DEFAULT);
    if (UNLIKELY(status != OCI_SUCCESS))
    {
      std::string msg;
      LOG4CXX_ERROR(getLogger(),
                    "Oracle error binding variable # "
                        << parm->index() << " Type: String "
                        << getOCIErrorString(status, _ociError, msg));
      throw std::runtime_error("Database error");
    }
  }
  else if ((boundInteger = dynamic_cast<ORACLEBoundInteger*>(parm)))
  {
#ifdef DEBUG
    LOG4CXX_DEBUG(getLogger(),
                  "Binding query variable # "
                      << parm->index()
                      << " of type: SQLT_INT (int) size: " << boundInteger->getBindBufferSize()
                      << " with value: " << *(boundInteger->getBindBuffer()));
#endif

    OCIBind* ociBindHandle = nullptr;
    int32_t status = OCIBindByPos(_ociStmt,
                                  &ociBindHandle,
                                  _ociError,
                                  parm->index(),
                                  boundInteger->getBindBuffer(),
                                  boundInteger->getBindBufferSize(),
                                  SQLT_INT,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  0,
                                  nullptr,
                                  OCI_DEFAULT);
    if (UNLIKELY(status != OCI_SUCCESS))
    {
      std::string msg;
      LOG4CXX_ERROR(getLogger(),
                    "Oracle error binding variable # "
                        << parm->index() << " Type: Integer "
                        << getOCIErrorString(status, _ociError, msg));
      throw std::runtime_error("Database error");
    }
  }
  else if ((boundLong = dynamic_cast<ORACLEBoundLong*>(parm)))
  {
#ifdef DEBUG
    LOG4CXX_DEBUG(getLogger(),
                  "Binding query variable # "
                      << parm->index()
                      << " of type: SQLT_LONG (long) size: " << boundLong->getBindBufferSize()
                      << " with value: " << *(boundLong->getBindBuffer()));
#endif

    OCIBind* ociBindHandle = nullptr;
    int32_t status = OCIBindByPos(_ociStmt,
                                  &ociBindHandle,
                                  _ociError,
                                  parm->index(),
                                  boundLong->getBindBuffer(),
                                  boundLong->getBindBufferSize(),
                                  SQLT_INT,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  0,
                                  nullptr,
                                  OCI_DEFAULT);
    if (UNLIKELY(status != OCI_SUCCESS))
    {
      std::string msg;
      LOG4CXX_ERROR(getLogger(),
                    "Oracle error binding variable # "
                        << parm->index() << " Type: Long "
                        << getOCIErrorString(status, _ociError, msg));
      throw std::runtime_error("Database error");
    }
  }
  else if ((boundDate = dynamic_cast<ORACLEBoundDate*>(parm)))
  {
#ifdef DEBUG
    char* bindBuffer = boundDate->getBindBuffer();
    int16_t year = (bindBuffer[0] - 100) * 100 + (bindBuffer[1] - 100);
    LOG4CXX_DEBUG(getLogger(),
                  "Binding query variable # "
                      << parm->index() << " of type: SQLT_DAT (date) size: "
                      << boundDate->getBindBufferSize() << " with value: " << year << "-" << setw(2)
                      << setfill('0') << ((int)bindBuffer[2]) << "-" << setw(2) << setfill('0')
                      << ((int)bindBuffer[3]) << ":" << setw(2) << setfill('0')
                      << ((int)bindBuffer[4]) - 1 << ":" << setw(2) << setfill('0')
                      << ((int)bindBuffer[5]) - 1 << ":" << setw(2) << setfill('0')
                      << ((int)bindBuffer[5]) - 1);
#endif

    OCIBind* ociBindHandle = nullptr;
    int32_t status = OCIBindByPos(_ociStmt,
                                  &ociBindHandle,
                                  _ociError,
                                  parm->index(),
                                  boundDate->getBindBuffer(),
                                  boundDate->getBindBufferSize(),
                                  SQLT_DAT,
                                  nullptr,
                                  nullptr,
                                  nullptr,
                                  0,
                                  nullptr,
                                  OCI_DEFAULT);
    if (UNLIKELY(status != OCI_SUCCESS))
    {
      std::string msg;
      LOG4CXX_ERROR(getLogger(),
                    "Oracle error binding variable # "
                        << parm->index() << " Type: Date "
                        << getOCIErrorString(status, _ociError, msg));
      throw std::runtime_error("Database error");
    }
  }
  else if (LIKELY((boundDateTime = dynamic_cast<ORACLEBoundDateTime*>(parm))))
  {
    int32_t status = OCIDescriptorAlloc(
        _ociEnv, (void**)&(boundDateTime->getBindBuffer()), OCI_DTYPE_TIMESTAMP, 0, nullptr);

    if (UNLIKELY(status != OCI_SUCCESS))
    {
      boundDateTime->getBindBuffer() = nullptr;
      std::string msg;
      LOG4CXX_ERROR(getLogger(),
                    "Oracle error binding variable # "
                        << parm->index() << " Type: TimeStamp "
                        << getOCIErrorString(status, _ociError, msg));
      throw std::runtime_error("Database error");
    }

    _descriptorHandles.emplace_back(boundDateTime->getBindBuffer(), OCI_DTYPE_TIMESTAMP);

    DateTime& value = boundDateTime->getValue();
    int16_t year(0);
    uint8_t month(0), day(0), hour(0), min(0), sec(0);
    uint32_t fsec(0);

    if (value.isInfinity())
    {
      year = 9999;
      month = 12;
      day = 31;
      hour = 23;
      min = 59;
      sec = 59;
      fsec = 999000000;
    }
    else if (UNLIKELY(value.isNegInfinity()))
    {
      year = -4712;
      month = 1;
      day = 1;
    }
    else
    {
      year = value.year();
      month = value.month();
      day = value.day();
      hour = value.hours();
      min = value.minutes();
      sec = value.seconds();
      fsec = value.fractionalSeconds() * 1000;
    }

#ifdef DEBUG
    char buf[28];
    snprintf(buf,
             sizeof(buf),
             "%.4d-%.2d-%.2d:%.2d:%.2d:%.2d.%.6ld",
             (int)year,
             (int)month,
             (int)day,
             (int)hour,
             (int)min,
             (int)sec,
             (long)fsec / 1000);

    LOG4CXX_DEBUG(getLogger(),
                  "Binding query variable # "
                      << parm->index() << " of type: SQLT_TIMESTAMP (TimeStamp) size: "
                      << boundDateTime->getBindBufferSize() << " with value: " << buf);
#endif

    status = OCIDateTimeConstruct(_ociEnv,
                                  _ociError,
                                  boundDateTime->getBindBuffer(),
                                  year,
                                  month,
                                  day,
                                  hour,
                                  min,
                                  sec,
                                  fsec,
                                  nullptr,
                                  0);

    if (UNLIKELY(status != OCI_SUCCESS))
    {
      std::string msg;
      LOG4CXX_ERROR(getLogger(),
                    "Oracle error binding variable # "
                        << parm->index() << " Type: TimeStamp "
                        << getOCIErrorString(status, _ociError, msg));
      throw std::runtime_error("Database error");
    }

    OCIBind* ociBindHandle = nullptr;
    status = OCIBindByPos(_ociStmt,
                          &ociBindHandle,
                          _ociError,
                          parm->index(),
                          &(boundDateTime->getBindBuffer()),
                          boundDateTime->getBindBufferSize(),
                          SQLT_TIMESTAMP,
                          nullptr,
                          nullptr,
                          nullptr,
                          0,
                          nullptr,
                          OCI_DEFAULT);

    if (UNLIKELY(status != OCI_SUCCESS))
    {
      std::string msg;
      LOG4CXX_ERROR(getLogger(),
                    "Oracle error binding variable # "
                        << parm->index() << " Type: TimeStamp "
                        << getOCIErrorString(status, _ociError, msg));
      throw std::runtime_error("Database error");
    }
  }
  else
  {
    LOG4CXX_ERROR(getLogger(),
                  "Oracle error binding variable # " << parm->index()
                                                     << " Unknown BoundParameter type");
    throw std::runtime_error("Database error");
  }
}

void
ORACLEAdapter::dumpDbConnections(std::ostream& os)
{
  DBServerPool& dbServer = DBServerPool::instance();
  os << "<DBCONN>";
  os << "<GEN USED=\"" << dbServer.numActive() << "\" IDLE=\"" << dbServer.numIdle() << "\"/>";
  if (Global::allowHistorical())
  {
    DBHistoryServerPool& hisServer = DBHistoryServerPool::instance();
    os << "<HIS USED=\"" << hisServer.numActive() << "\" IDLE=\"" << hisServer.numIdle() << "\"/>";
  }

  { // Lock scope
    // Lock scope must be here to avoid potential deadlock with
    // DBServer (StackKeyedPool) which can happen if a DBAdapter
    // (connection) is created or deleted while this method is
    // in progress.
    //
    boost::lock_guard<boost::mutex> g(_mutex);
    std::map<std::string, int>::const_iterator db(_dbConnections.begin());
    std::map<std::string, int>::const_iterator end(_dbConnections.end());
    while (db != end)
    {
      os << "<CONN HOST=\"" << (*db).first << "\""
         << " CT=\"" << (*db).second << "\" />";
      ++db;
    }
  } //-Lock Scope

  os << "</DBCONN>";
}

void
ORACLEAdapter::dumpDbConnections()
{
  std::string dbNames;
  bool found = false;
  std::map<std::string, int>::const_iterator db(_dbConnections.begin());
  std::map<std::string, int>::const_iterator end(_dbConnections.end());
  while (db != end)
  {
    if (db->second > 0)
    {
      if (found)
      {
        dbNames += ",";
      }
      size_t pos = db->first.find_first_of('.');
      if (pos != std::string::npos)
        dbNames += db->first.substr(0, pos);
      else
        dbNames += db->first;
      found = true;
    }
    ++db;
  }
  if (found)
    TseSrvStats::recordCurrentDatabase(dbNames);
  if (getLogger()->isInfoEnabled())
  {
    std::ostringstream os;
    db = _dbConnections.begin();
    os << "Database Connections:";
    while (db != end)
    {
      os << " " << (*db).first << " (" << (*db).second << ")";
      ++db;
    }
    LOG4CXX_INFO(getLogger(), os.str());
  }
}

void getQueryElapsedStats(std::string& stats)
{
  std::ostringstream oss;
  std::lock_guard<std::mutex> lock(queryStatsMutex);
  for (const auto& pair : queryStatsMap)
  {
    oss << pair.first << '|' << pair.second._calls
        << '|' << pair.second._elapsed / pair.second._calls
        << '\n';
  }
  oss.str().swap(stats);
}

}// tse
