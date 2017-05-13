//----------------------------------------------------------------------------
//
//     File:           ORACLEDBConnectionInfoManager.cpp
//     Description:    Oracle Database connection info manager implementation.
//     Created:        03/11/2010
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

#include "DBAccess/ORACLEDBConnectionInfoManager.h"

#include "Common/Config/ConfigMan.h"
#include "Common/Global.h"

#include <oci.h>

using namespace DBAccess;

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

ORACLEDBConnectionInfoManager::ORACLEDBConnectionInfoManager(tse::ConfigMan& config)
  : _config(config), _explorerServiceSuffix("")
{
}

ORACLEDBConnectionInfoManager::~ORACLEDBConnectionInfoManager() {}

DBPoolInfo*
ORACLEDBConnectionInfoManager::createPoolInfo(const std::string& pool, bool isHistorical)
{
  std::string useExplorerString("");
  bool useExplorer = false;
  if (_config.getValue("useExplorer", useExplorerString, pool))
  {
    if ("Y" == useExplorerString || "T" == useExplorerString || "YES" == useExplorerString ||
        "TRUE" == useExplorerString)
    {
      useExplorer = true;
    }
  }

  if (useExplorer)
  {
    // Get the suffix to use for the Explorer Service.
    //
    // If the suffix is defined we will use this to override the suffix specified
    // for the ORACLE_EXPLORER connection when explorer for the cluster specific service.
    //
    // If this is not defined we will default to the explorer naming convention
    // that maintains the underscore delimited suffix for cluster specific naming
    // for example: FUNCP_PRC

    if (!_config.getValue("explorerServiceSuffix", _explorerServiceSuffix, pool))
    {
      LOG4CXX_INFO(getLogger(),
                   "No Explorer Service Suffix found for pool: "
                       << pool << ".  Using explorer service name as configured.");
    }
    else
    {
      LOG4CXX_INFO(getLogger(),
                   "Using Explorer Service Suffix ["
                       << _explorerServiceSuffix << "] for pool: " << pool
                       << ".  Explorer service name suffix will be overridden.");
    }
  }

  std::string ini;
  bool useIni = _config.getValue("ini", ini, pool);

  if (useIni)
  {
    LOG4CXX_INFO(getLogger(), "Using ini: " << ini << " for pool: " << pool);

    tse::ConfigMan iniConfig;
    if (!iniConfig.read(ini))
    {
      LOG4CXX_ERROR(getLogger(), "Unable to read ini file: " << ini);
      return nullptr;
    }

    if (useExplorer)
    {
      LOG4CXX_INFO(getLogger(), "Using explorer connection for pool: " << pool);

      std::string key;
      if (isHistorical)
        key = "DBConnect\\ORACLE_HISTORICAL_EXPLORER";
      else
        key = "DBConnect\\ORACLE_EXPLORER";

      std::vector<std::string> explorerUrls;
      if (!iniConfig.getValues(key, explorerUrls))
      {
        LOG4CXX_ERROR(getLogger(),
                      "Unable to read Explorer URL(s) using key: " << key
                                                                   << " from ini file: " << ini);
        return nullptr;
      }

      if (explorerUrls.empty())
      {
        LOG4CXX_ERROR(getLogger(),
                      "No Explorer URL(s) found in ini file: " << ini << " for key: " << key);
        return nullptr;
      }

      std::vector<std::string>::iterator urlIter = explorerUrls.begin();
      std::vector<std::string>::iterator urlIterEnd = explorerUrls.end();
      for (; urlIter != urlIterEnd; ++urlIter)
      {
        std::string& url = *urlIter;
        std::string::size_type pos = url.find('@');
        if (pos == std::string::npos)
        {
          LOG4CXX_ERROR(getLogger(),
                        "Error parsing Explorer URL: " << url << " from ini file: " << ini
                                                       << " for key: " << key);
          return nullptr;
        }
        std::string userPass = url.substr(0, pos);
        std::string::size_type colonPos = userPass.find(':');
        if (colonPos == std::string::npos)
        {
          LOG4CXX_ERROR(getLogger(),
                        "Error parsing Explorer URL: " << url << " from ini file: " << ini
                                                       << " for key: " << key);
          return nullptr;
        }
        std::string user = userPass.substr(0, colonPos);
        std::string password = userPass.substr(colonPos + 1);
        std::string service = url.substr(pos + 1);

        // Try to establish the explorer connection and build a
        // cluster-specific service name.
        //
        std::string clusterSpecificService("");
        if (!getClusterSpecificServiceName(user, password, service, clusterSpecificService))
        {
          LOG4CXX_ERROR(getLogger(),
                        "Could not get cluster specific service name using "
                            << "explorer connection URL: " << url << " (" << user << ":" << password
                            << "@" << service << ")");

          // Try any remaining explorer URLs
          continue;
        }

        DBPoolInfo* poolInfo = new DBPoolInfo(pool, url, "", 0, "", service, user, password);

        DBConnectionInfo* connInfo = getConnInfo("", 0, "", clusterSpecificService, user);

        if (!connInfo)
        {
          connInfo =
              new DBConnectionInfo(false, "", "", 0, "", clusterSpecificService, user, password);
          addConnInfo(connInfo);
        }
        poolInfo->addConnectionInfo(connInfo);

        LOG4CXX_INFO(getLogger(),
                     "Added connection info: " << user << ":" << password << "@"
                                               << clusterSpecificService << " for pool: " << pool);

        // If multple explorer connection URLs are specified, we only need
        // to use the first one that works. If we get to this point we
        // have successfully used the explorer connection to construct
        // a cluster-specific connection info object. When running in
        // explorer-connection mode we only need one connection info
        // object for each pool.

        return poolInfo;
      }
    }
    else
    {
      // Not using explorer connection
      //
      LOG4CXX_INFO(getLogger(), "Not using explorer connection for pool: " << pool);

      std::string key;
      if (isHistorical)
        key = "DBConnect\\ORACLE_HISTORICAL";
      else if (pool == "Fares")
        key = "DBConnect\\ORACLE_FARES";
      else if (pool == "Rules")
        key = "DBConnect\\ORACLE_RULES";
      else
        key = "DBConnect\\ORACLE";

      std::vector<std::string> urls;
      if (!iniConfig.getValues(key, urls))
      {
        LOG4CXX_ERROR(getLogger(),
                      "Unable to read database connection URL(s) using key: "
                          << key << " from ini file: " << ini);
        return nullptr;
      }

      DBPoolInfo* poolInfo(nullptr);

      std::vector<std::string>::iterator urlIter = urls.begin();
      std::vector<std::string>::iterator urlIterEnd = urls.end();
      for (; urlIter != urlIterEnd; ++urlIter)
      {
        std::string& url = *urlIter;
        std::string::size_type pos = url.find('@');
        if (pos == std::string::npos)
        {
          LOG4CXX_ERROR(getLogger(),
                        "Error parsing URL: " << url << " from ini file: " << ini
                                              << " for key: " << key);
          return nullptr;
        }
        std::string userPass = url.substr(0, pos);
        std::string::size_type colonPos = userPass.find(':');
        if (colonPos == std::string::npos)
        {
          LOG4CXX_ERROR(getLogger(),
                        "Error parsing URL: " << url << " from ini file: " << ini
                                              << " for key: " << key);
          return nullptr;
        }
        std::string user = userPass.substr(0, colonPos);
        std::string password = userPass.substr(colonPos + 1);
        std::string service = url.substr(pos + 1);

        DBConnectionInfo* connInfo = getConnInfo("", 0, "", service, user);

        if (!connInfo)
        {
          connInfo = new DBConnectionInfo(false, url, "", 0, "", service, user, password);
          addConnInfo(connInfo);
        }

        if (!poolInfo)
          poolInfo = new DBPoolInfo(pool);

        poolInfo->addConnectionInfo(connInfo);

        LOG4CXX_INFO(getLogger(), "Added connection info: " << url << " for pool: " << pool);
      }
      return poolInfo;
    }
  }
  else
  {
    // Not using ini
    //
    LOG4CXX_INFO(getLogger(), "Not using ini for pool: " << pool);

    std::string user, password, service;
    std::string backupUser, backupPassword, backupService;

    _config.getValue("user", user, pool);
    _config.getValue("password", password, pool);
    if (!_config.getValue("sid", service, pool))
    {
      LOG4CXX_ERROR(getLogger(), "Cannot connect, no sid (service) specified");
      return nullptr;
    }

    if (useExplorer)
    {
      LOG4CXX_INFO(getLogger(), "Using explorer connection for pool: " << pool);

      // Try to establish the explorer connection and build a
      // cluster-specific service name.
      //
      std::string clusterSpecificService;
      if (!getClusterSpecificServiceName(user, password, service, clusterSpecificService))
      {
        LOG4CXX_ERROR(getLogger(),
                      "Could not get cluster specific service name using "
                          << "explorer connection. User: " << user << " Password: " << password
                          << " SID(SERVICE): " << service);
        return nullptr;
      }

      DBPoolInfo* poolInfo = new DBPoolInfo(pool, "", "", 0, "", service, user, password);

      DBConnectionInfo* connInfo = getConnInfo("", 0, "", clusterSpecificService, user);

      if (!connInfo)
      {
        connInfo =
            new DBConnectionInfo(false, "", "", 0, "", clusterSpecificService, user, password);
        addConnInfo(connInfo);

        LOG4CXX_INFO(getLogger(),
                     "Added connection info: " << user << ":" << password << "@"
                                               << clusterSpecificService << " for pool: " << pool);
      }
      poolInfo->addConnectionInfo(connInfo);

      return poolInfo;
    }
    else
    {
      // Not using explorer connection
      //
      LOG4CXX_INFO(getLogger(), "Not using explorer connection for pool: " << pool);

      DBPoolInfo* poolInfo = new DBPoolInfo(pool);

      DBConnectionInfo* connInfo = getConnInfo("", 0, "", service, user);

      if (!connInfo)
      {
        connInfo = new DBConnectionInfo(false, "", "", 0, "", service, user, password);
        addConnInfo(connInfo);
      }
      poolInfo->addConnectionInfo(connInfo);

      LOG4CXX_INFO(getLogger(),
                   "Added connection info: " << user << ":" << password << "@" << service
                                             << " for pool: " << pool);

      _config.getValue("backupUser", backupUser, pool);
      _config.getValue("backupPassword", backupPassword, pool);
      if (_config.getValue("backupSid", backupService, pool))
      {
        DBConnectionInfo* backupConnInfo = getConnInfo("", 0, "", backupService, backupUser);

        if (!backupConnInfo)
        {
          backupConnInfo =
              new DBConnectionInfo(false, "", "", 0, "", backupService, backupUser, backupPassword);
          addConnInfo(backupConnInfo);

          LOG4CXX_INFO(getLogger(),
                       "Added connection info: " << user << ":" << password << "@" << service
                                                 << " for pool: " << pool);
        }
        poolInfo->addConnectionInfo(backupConnInfo);
      }
      return poolInfo;
    }
  }
  return nullptr;
}

DBConnectionInfo*
ORACLEDBConnectionInfoManager::createConnInfo(const DBPoolInfo* poolInfo)
{
  if (poolInfo)
  {
    if (poolInfo->useExplorerConnection())
    {
      const DBConnectionInfo* expConnInfo = poolInfo->getExplorerConnectionInfo();

      if (!expConnInfo)
      {
        LOG4CXX_ERROR(getLogger(),
                      "PoolInfo contained null Explorer Connection for Pool: " << poolInfo->pool());
        return nullptr;
      }

      // Try to establish the explorer connection and build a
      // cluster-specific service name.
      //
      std::string clusterSpecificService;
      if (!getClusterSpecificServiceName(expConnInfo->user(),
                                         expConnInfo->password(),
                                         expConnInfo->service(),
                                         clusterSpecificService))
      {
        LOG4CXX_ERROR(getLogger(),
                      "Could not get cluster specific service name using "
                          << "explorer connection. User: " << expConnInfo->user()
                          << " Password: " << expConnInfo->password()
                          << " SID(SERVICE): " << expConnInfo->service());
        return nullptr;
      }

      DBConnectionInfo* connInfo =
          getConnInfo("", 0, "", clusterSpecificService, expConnInfo->user());

      if (!connInfo)
      {
        connInfo = new DBConnectionInfo(false,
                                        "",
                                        "",
                                        0,
                                        "",
                                        clusterSpecificService,
                                        expConnInfo->user(),
                                        expConnInfo->password());
        addConnInfo(connInfo);
      }
      return connInfo;
    }
    else
    {
      LOG4CXX_ERROR(getLogger(),
                    "In createConnInfo(): Invalid attempt to create a new "
                        << "DBConnectionInfo for Pool: " << poolInfo->pool()
                        << ". The pool must be configured "
                        << "as an Explorer based pool to create a "
                        << "DBConnectionInfo in this way.");
    }
  }
  LOG4CXX_ERROR(getLogger(), "In createConnInfo(): poolInfo is null.");
  return nullptr;
}

bool
ORACLEDBConnectionInfoManager::getClusterSpecificServiceName(
    const std::string& user,
    const std::string& password,
    const std::string& explorerServiceName,
    std::string& clusterSpecificServiceName)
{
  bool useOracleWallet(!user.length() || !password.length());
  std::string authentication("wallet");

  // Get the Prefix and Suffix for the specified service in the explorer service name

  std::string serviceNameSuffix;
  std::string serviceNamePrefix;

  std::string::size_type lastPos = explorerServiceName.find_last_of("_");
  if (lastPos == std::string::npos)
  {
    LOG4CXX_ERROR(getLogger(),
                  "Invalid explorer service name: "
                      << explorerServiceName
                      << " The explorer connection service name must end with "
                      << "an underscore delimited suffix, for example: FUNCP_PRC");
    return false;
  }

  serviceNameSuffix = explorerServiceName.substr(lastPos);
  serviceNamePrefix = explorerServiceName.substr(0, lastPos);

  // If the explorer service suffix is specified then override the suffix from
  // the explorerServiceName

  std::string explorerServiceNameToUse;

  if (!_explorerServiceSuffix.empty())
  {
    explorerServiceNameToUse = serviceNamePrefix;
    explorerServiceNameToUse.append(_explorerServiceSuffix);
  }
  else
  {
    explorerServiceNameToUse = explorerServiceName;
  }

  LOG4CXX_INFO(getLogger(), "Using Explorer Service Name [" << explorerServiceNameToUse << "]");

  OCIEnv* ociEnv;
  OCIServer* ociServer;
  OCISvcCtx* ociSvcCtx;
  OCISession* ociSession;
  OCIStmt* ociStmt;
  OCIError* ociError;

  std::string ociErrMsg;

  int32_t status = OCIEnvCreate(&ociEnv, OCI_THREADED | OCI_NO_MUTEX, nullptr, nullptr, nullptr, nullptr, 0, (void**)nullptr);

  if (status != OCI_SUCCESS)
  {
    LOG4CXX_ERROR(getLogger(), "Error initializing OCI Environment handle: " << status);
    return false;
  }

  // Allocate the OCI error handle
  OCIHandleAlloc(ociEnv, (void**)&ociError, OCI_HTYPE_ERROR, 0, (void**)nullptr);

  // Allocate the OCI Server handle
  OCIHandleAlloc(ociEnv, (void**)&ociServer, OCI_HTYPE_SERVER, 0, (void**)nullptr);

  // Allocate the OCI Service Context handle
  OCIHandleAlloc(ociEnv, (void**)&ociSvcCtx, OCI_HTYPE_SVCCTX, 0, (void**)nullptr);

  // Attach to the server
  status = OCIServerAttach(ociServer,
                           ociError,
                           (text*)explorerServiceNameToUse.c_str(),
                           explorerServiceNameToUse.size(),
                           OCI_DEFAULT);
  if (status != OCI_SUCCESS)
  {
    LOG4CXX_ERROR(getLogger(),
                  "Oracle error attaching to database server. "
                      << "SERVICE_NAME: " << explorerServiceNameToUse << " USER: " << user
                      << " Error: " << getOCIErrorString(status, ociError, ociErrMsg));
    ociServer = nullptr;
    OCIHandleFree(ociEnv, OCI_HTYPE_ENV);
    return false;
  }

  // Set the Server handle attribute in the Service Context
  OCIAttrSet(ociSvcCtx, OCI_HTYPE_SVCCTX, ociServer, 0, OCI_ATTR_SERVER, ociError);

  // Allocate a Session handle
  OCIHandleAlloc(ociEnv, (void**)&ociSession, OCI_HTYPE_SESSION, 0, (void**)nullptr);

  // Set the username attribute in the Session
  OCIAttrSet(
      ociSession, OCI_HTYPE_SESSION, (void*)user.c_str(), user.size(), OCI_ATTR_USERNAME, ociError);

  // Set the password attribute in the Session
  OCIAttrSet(ociSession,
             OCI_HTYPE_SESSION,
             (void*)password.c_str(),
             password.size(),
             OCI_ATTR_PASSWORD,
             ociError);

  /*
  // Set the module name attribute in the Session. This helps
  //  identify the application in AWR reports.
  OCIAttrSet(ociSession, OCI_HTYPE_SESSION,
             (void *) modulename.c_str(), modulename.size(),
             OCI_ATTR_MODULE, ociError);
  */

  // Begin the Session
  if (useOracleWallet)
  {
    status = OCISessionBegin(ociSvcCtx, ociError, ociSession, OCI_CRED_EXT, OCI_DEFAULT);
  }
  else
  {
    authentication = "user password";
    status = OCISessionBegin(ociSvcCtx, ociError, ociSession, OCI_CRED_RDBMS, OCI_DEFAULT);
  }

  if (status != OCI_SUCCESS)
  {
    if (useOracleWallet)
    {
      LOG4CXX_ERROR(getLogger(),
                    "Oracle error establishing database session. "
                        << "SERVICE_NAME: " << explorerServiceNameToUse);
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
              << explorerServiceNameToUse << "] specified.\n"
              << "------------------------------------------------------------------------\n");
    }
    else
      LOG4CXX_ERROR(getLogger(),
                    "Oracle error establishing database session. "
                        << "SERVICE_NAME: " << explorerServiceNameToUse << " USER: " << user
                        << " Error: " << getOCIErrorString(status, ociError, ociErrMsg));

    OCIServerDetach(ociServer, ociError, OCI_DEFAULT);
    OCIHandleFree(ociServer, OCI_HTYPE_SERVER);
    OCIHandleFree(ociEnv, OCI_HTYPE_ENV);
    return false;
  }

  if (status != OCI_SUCCESS)
  {
    LOG4CXX_ERROR(getLogger(),
                  "Oracle error establishing database session. "
                      << "SERVICE_NAME: " << explorerServiceNameToUse << " USER: " << user
                      << " Error: " << getOCIErrorString(status, ociError, ociErrMsg));

    OCIServerDetach(ociServer, ociError, OCI_DEFAULT);
    OCIHandleFree(ociServer, OCI_HTYPE_SERVER);
    OCIHandleFree(ociEnv, OCI_HTYPE_ENV);
    return false;
  }

  // Set the Session attribute in the Service Context
  OCIAttrSet((void*)ociSvcCtx, OCI_HTYPE_SVCCTX, ociSession, 0, OCI_ATTR_SESSION, ociError);

  // Set the statement cache size for the Service Context
  uint32_t stmtCacheSize = 0;
  OCIAttrSet(
      (void*)ociSvcCtx, OCI_HTYPE_SVCCTX, &stmtCacheSize, 0, OCI_ATTR_STMTCACHESIZE, ociError);

  std::string explorerSQL("SELECT SYS_CONTEXT('USERENV', 'DB_NAME') AS DB_NAME FROM DUAL");

  status = OCIStmtPrepare2(ociSvcCtx,
                           &ociStmt,
                           ociError,
                           (const unsigned char*)explorerSQL.c_str(),
                           explorerSQL.length(),
                           nullptr,
                           0,
                           OCI_NTV_SYNTAX,
                           OCI_DEFAULT);

  std::string message("Connected to ORACLE database using authentication [");
  message += authentication + "] SERVICE_NAME: " + explorerServiceNameToUse;
  if (!useOracleWallet)
    message += " USER: " + user;

  LOG4CXX_INFO(getLogger(), message);
  if (status != OCI_SUCCESS)
  {
    LOG4CXX_ERROR(getLogger(),
                  "Oracle error during prepare for explorer connection. "
                      << "SERVICE_NAME: " << explorerServiceNameToUse << " USER: " << user
                      << " Error: " << getOCIErrorString(status, ociError, ociErrMsg));

    status = OCIStmtRelease(ociStmt, ociError, nullptr, 0, OCI_DEFAULT);
    if (status != OCI_SUCCESS)
    {
      LOG4CXX_ERROR(getLogger(),
                    "Oracle error releasing statement handle. "
                        << "SERVICE_NAME: " << explorerServiceNameToUse << " USER: " << user
                        << " Error: " << getOCIErrorString(status, ociError, ociErrMsg));
    }
    OCISessionEnd(ociSvcCtx, ociError, ociSession, OCI_DEFAULT);
    OCIHandleFree(ociSession, OCI_HTYPE_SESSION);
    OCIServerDetach(ociServer, ociError, OCI_DEFAULT);
    OCIHandleFree(ociServer, OCI_HTYPE_SERVER);
    OCIHandleFree(ociEnv, OCI_HTYPE_ENV);
    return false;
  }

  char db_name[256];
  memset(db_name, 0, sizeof(db_name));
  int16_t nullIndicator(0);
  uint16_t returnedDataLength(0);

  OCIDefine* ociDefine;
  status = OCIDefineByPos(ociStmt,
                          &ociDefine,
                          ociError,
                          1,
                          db_name,
                          sizeof(db_name) - 1,
                          SQLT_CHR,
                          &nullIndicator,
                          &returnedDataLength,
                          nullptr,
                          OCI_DEFAULT);

  if (status != OCI_SUCCESS)
  {
    LOG4CXX_ERROR(getLogger(),
                  "Oracle error during define for explorer connection. "
                      << "SERVICE_NAME: " << explorerServiceNameToUse << " USER: " << user
                      << " Error: " << getOCIErrorString(status, ociError, ociErrMsg));

    return false;
  }

  status = OCIStmtExecute(ociSvcCtx,
                          ociStmt,
                          ociError,
                          1,
                          0,
                          (CONST OCISnapshot*)nullptr,
                          (OCISnapshot*)nullptr,
                          OCI_DEFAULT);

  if (status != OCI_SUCCESS)
  {
    LOG4CXX_ERROR(getLogger(),
                  "Oracle error during execute for explorer connection. "
                      << "SERVICE_NAME: " << explorerServiceNameToUse << " USER: " << user
                      << " Error: " << getOCIErrorString(status, ociError, ociErrMsg));

    status = OCIStmtRelease(ociStmt, ociError, nullptr, 0, OCI_DEFAULT);
    if (status != OCI_SUCCESS)
    {
      LOG4CXX_ERROR(getLogger(),
                    "Oracle error releasing statement handle. "
                        << "SERVICE_NAME: " << explorerServiceNameToUse << " USER: " << user
                        << " Error: " << getOCIErrorString(status, ociError, ociErrMsg));
    }
    OCISessionEnd(ociSvcCtx, ociError, ociSession, OCI_DEFAULT);
    OCIHandleFree(ociSession, OCI_HTYPE_SESSION);
    OCIServerDetach(ociServer, ociError, OCI_DEFAULT);
    OCIHandleFree(ociServer, OCI_HTYPE_SERVER);
    OCIHandleFree(ociEnv, OCI_HTYPE_ENV);
    return false;
  }

  // Check the null indicator
  //
  if (nullIndicator == -1)
  {
    LOG4CXX_ERROR(getLogger(),
                  "Explorer connection SQL returned NULL data. SQL: "
                      << explorerSQL << "SERVICE_NAME: " << explorerServiceNameToUse
                      << " USER: " << user);

    status = OCIStmtRelease(ociStmt, ociError, nullptr, 0, OCI_DEFAULT);
    if (status != OCI_SUCCESS)
    {
      LOG4CXX_ERROR(getLogger(),
                    "Oracle error releasing statement handle. "
                        << "SERVICE_NAME: " << explorerServiceNameToUse << " USER: " << user
                        << " Error: " << getOCIErrorString(status, ociError, ociErrMsg));
    }
    OCISessionEnd(ociSvcCtx, ociError, ociSession, OCI_DEFAULT);
    OCIHandleFree(ociSession, OCI_HTYPE_SESSION);
    OCIServerDetach(ociServer, ociError, OCI_DEFAULT);
    OCIHandleFree(ociServer, OCI_HTYPE_SERVER);
    OCIHandleFree(ociEnv, OCI_HTYPE_ENV);
    return false;
  }

  if (returnedDataLength > sizeof(db_name))
  {
    LOG4CXX_ERROR(getLogger(),
                  "Returned data length: " << returnedDataLength
                                           << " is too long for the available buffer size: "
                                           << sizeof(db_name));
    return false;
  }
  db_name[returnedDataLength] = 0;
  std::string serviceName(db_name);

  serviceName.append(serviceNameSuffix);

  clusterSpecificServiceName = serviceName;

  LOG4CXX_INFO(getLogger(),
               "Generated cluster-specific service name: " << clusterSpecificServiceName);

  status = OCIStmtRelease(ociStmt, ociError, nullptr, 0, OCI_DEFAULT);
  if (status != OCI_SUCCESS)
  {
    LOG4CXX_ERROR(getLogger(),
                  "Oracle error releasing statement handle. "
                      << "SERVICE_NAME: " << explorerServiceNameToUse << " USER: " << user
                      << " Error: " << getOCIErrorString(status, ociError, ociErrMsg));
  }

  OCISessionEnd(ociSvcCtx, ociError, ociSession, OCI_DEFAULT);
  OCIHandleFree(ociSession, OCI_HTYPE_SESSION);
  OCIServerDetach(ociServer, ociError, OCI_DEFAULT);
  OCIHandleFree(ociServer, OCI_HTYPE_SERVER);
  OCIHandleFree(ociEnv, OCI_HTYPE_ENV);

  return true;
}

log4cxx::LoggerPtr&
ORACLEDBConnectionInfoManager::getLogger()
{
  static log4cxx::LoggerPtr logger(
      log4cxx::Logger::getLogger("atseintl.DBAccess.ORACLEDBConnectionInfoManager"));
  return logger;
}
