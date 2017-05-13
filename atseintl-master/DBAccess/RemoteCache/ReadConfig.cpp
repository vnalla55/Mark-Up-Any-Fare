#include "DBAccess/RemoteCache/ReadConfig.h"

#include "Common/Config/ConfigMan.h"
#include "Common/Global.h"
#include "DBAccess/RemoteCache/RCServerAttributes.h"

#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>

#include <atomic>
#include <mutex>
#include <sstream>

namespace tse
{

namespace RemoteCache
{

namespace ReadConfig
{

typedef std::map<std::string, RCServerAttributes> AttributesMap;

void readConfig();

namespace
{

std::mutex mutex;

std::atomic<bool> enabled(false);
std::atomic<bool> persistentConnections(false);
std::string defaultMasterHost;
std::string defaultMasterPort;
std::string defaultSecondaryHost;
std::string defaultSecondaryPort;
std::string serverPort;
AttributesMap attributesMap;
std::atomic<bool> ignoreDBMismatch(false);
std::atomic<int> clientCacheUpdateDelay(60);
std::atomic<bool> masterAllDatatypes(false);
std::atomic<int> threadPoolSize(100);
std::atomic<int> maxNumberClients(20);
std::atomic<bool> asynchronousHealthcheck(false);
std::atomic<bool> healthcheckEnabled(false);
std::atomic<int> ldcDelay(0);
std::atomic<int> healthcheckTimeout(5);
std::atomic<double> queueTolerance(0.2);
std::atomic<int> healthcheckPeriod(0);
std::atomic<int> clientConnectionTimeout(10);
std::atomic<bool> linger(true);
std::atomic<int> lingerTime(0);
std::atomic<bool> keepAlive(false);
std::atomic<bool> useClientSpecifiedTimeout(true);
std::atomic<int> clientProcessingTimeout(130);
std::atomic<bool> checkBaseline(false);
std::atomic<int> cacheUpdateDetectionInterval(180);
std::atomic<int> clientPoolSamplingInterval(0);
std::atomic<int> clientPoolAdjustInterval(0);
std::atomic<int> statsSamplingInterval(100);
std::atomic<int> statsLoggingInterval(60);
std::atomic<int> idleMasterTimeout(25);
std::atomic<int> idleSlaveTimeout(20);
std::atomic<int> serverReceiveTimeout(20);
std::atomic<int> serverSendTimeout(100);
std::atomic<int> clientSendTimeout(200);
std::atomic<double> minClientsRatio(.4);
std::atomic<bool> debugRC(false);

enum ATTRIBUTE
{
  ATTRIBUTES_NONE = -1
  , ATTRIBUTES_ENABLED
  , ATTRIBUTES_HOST
  , ATTRIBUTES_PORT
  , ATTRIBUTES_SECONDARY_HOST
  , ATTRIBUTES_SECONDARY_PORT
};

const std::string thisHost(boost::to_upper_copy(boost::asio::ip::host_name()));

bool boolStringCheck(const std::string& rep)
{
  return "Y" == rep || "y" == rep
         || "T" == rep || "t" == rep
         || "TRUE" == rep || "true" == rep || "1" == rep;
}

bool readEnabled()
{
  std::string enableStr;
  if (Global::hasConfig()
      && Global::config().getValue("ENABLED", enableStr, "REMOTE_CACHE_OPTIONS"))
  {
    enabled = boolStringCheck(enableStr);
  }
  return true;
}

void readStatsSamplingInterval()
{
  int cfgValue(0);
  if (Global::hasConfig()
      && Global::config().getValue("STATS_SAMPLING_INTERVAL", cfgValue, "REMOTE_CACHE_OPTIONS")
      && cfgValue > 0)
  {
    statsSamplingInterval = cfgValue;
  }
}

void readStatsLoggingInterval()
{
  int cfgValue(0);
  if (Global::hasConfig()
      && Global::config().getValue("STATS_LOGGING_INTERVAL", cfgValue, "REMOTE_CACHE_OPTIONS")
      && cfgValue > 0)
  {
    statsLoggingInterval = cfgValue;
  }
}

void readCheckBaseline()
{
  std::string cfgValue;
  if (Global::hasConfig()
      && Global::config().getValue("CHECK_BASELINE", cfgValue, "REMOTE_CACHE_OPTIONS"))
  {
    checkBaseline = boolStringCheck(cfgValue);
  }
}

void readMaxNumberClients()
{
  int maxNumberClientsCfg(0);
  if (Global::hasConfig()
      && Global::config().getValue("MAX_NUMBER_CLIENTS", maxNumberClientsCfg, "REMOTE_CACHE_OPTIONS")
      && maxNumberClientsCfg > 0)
  {
    maxNumberClients = maxNumberClientsCfg;
  }
}

void readMinClientsRatio()
{
  double minClientsRatioCfg(0);
  if (Global::hasConfig()
      && Global::config().getValue("MIN_CLIENTS_RATIO", minClientsRatioCfg, "REMOTE_CACHE_OPTIONS")
      && minClientsRatioCfg > 0
      && minClientsRatioCfg < 1)
  {
    minClientsRatio = minClientsRatioCfg;
  }
}

void readLdcDelay()
{
  ldcDelay = 0;
  std::string ldcEnabledCfg;
  if (Global::hasConfig()
      && Global::config().getValue("STARTUP_ACTION", ldcEnabledCfg, "DISK_CACHE_OPTIONS")
      && "load" == ldcEnabledCfg)
  {
    int ldcDelayCfg(0);
    if (Global::config().getValue("LDC_DELAY", ldcDelayCfg, "REMOTE_CACHE_OPTIONS")
        && ldcDelayCfg >= 0)
    {
      ldcDelay = ldcDelayCfg;
    }
  }
}

void readThreadPoolSize()
{
  int poolSizeCfg(0);
  if (Global::hasConfig()
      && Global::config().getValue("SERVER_THREAD_POOL_SIZE", poolSizeCfg, "REMOTE_CACHE_OPTIONS")
      && poolSizeCfg > 0)
  {
    threadPoolSize = poolSizeCfg;
  }
}

void readCacheUpdateDetectionInterval()
{
  int detectionIntervalCfg(0);
  if (Global::hasConfig()
      && Global::config().getValue("CACHE_UPDATE_DETECTION", detectionIntervalCfg, "REMOTE_CACHE_OPTIONS")
      && detectionIntervalCfg > 0)
  {
    cacheUpdateDetectionInterval = detectionIntervalCfg;
  }
}

void readIgnoreDatabaseMismatch()
{
  std::string stringCfg;
  if (Global::hasConfig()
      && Global::config().getValue("IGNORE_DATABASE_MISMATCH", stringCfg, "REMOTE_CACHE_OPTIONS"))
  {
    ignoreDBMismatch = boolStringCheck(stringCfg);
  }
}

void readPersistentConnections()
{
  std::string persistentStr;
  if (Global::hasConfig()
      && Global::config().getValue("PERSISTENT_CONNECTIONS", persistentStr, "REMOTE_CACHE_OPTIONS"))
  {
    persistentConnections = boolStringCheck(persistentStr);
  }
}

void readClientCacheUpdateDelay()
{
  int delayCfg(0);
  if (Global::hasConfig()
      && Global::config().getValue("REMOTE_CLIENT_CACHE_UPDATE_DELAY", delayCfg, "REMOTE_CACHE_OPTIONS")
      && delayCfg >= 0)
  {
    clientCacheUpdateDelay = delayCfg;
  }
}

void readAsynchronousHealthcheck()
{
  std::string stringCfg;
  if (Global::hasConfig()
      && Global::config().getValue("ASYNCHRONOUS_HEALTHCHECK", stringCfg, "REMOTE_CACHE_OPTIONS"))
  {
    asynchronousHealthcheck = boolStringCheck(stringCfg);
  }
}

void readHealthcheckEnabled()
{
  std::string cfgValue;
  if (Global::hasConfig()
      && Global::config().getValue("HEALTHCHECK", cfgValue, "REMOTE_CACHE_OPTIONS"))
  {
    healthcheckEnabled = boolStringCheck(cfgValue);
  }
}

void readClientConnectionTimeout()
{
  int timeoutCfg(0);
  if (Global::hasConfig()
      && Global::config().getValue("CLIENT_CONNECTION_TIMEOUT", timeoutCfg, "REMOTE_CACHE_OPTIONS")
      && timeoutCfg > 0)
  {
    clientConnectionTimeout = timeoutCfg;
  }
}

void readLinger()
{
  std::string lingerStr;
  if (Global::hasConfig()
      && Global::config().getValue("LINGER", lingerStr, "REMOTE_CACHE_OPTIONS"))
  {
    linger = boolStringCheck(lingerStr);
  }
}

void readLingerTime()
{
  int lingerTimeCfg(0);
  if (Global::hasConfig()
      && Global::config().getValue("LINGER_TIME", lingerTimeCfg, "REMOTE_CACHE_OPTIONS")
      && lingerTimeCfg > 0)
  {
    lingerTime = lingerTimeCfg;
  }
}

void readKeepAlive()
{
  std::string keepAliveStr;
  if (Global::hasConfig()
      && Global::config().getValue("KEEP_ALIVE", keepAliveStr, "REMOTE_CACHE_OPTIONS"))
  {
    keepAlive = boolStringCheck(keepAliveStr);
  }
}

void readUseClientSpecifiedTimeout()
{
  std::string useCfg;
  if (Global::hasConfig()
      && Global::config().getValue("USE_CLIENT_SPECIFIED_TIMEOUT", useCfg, "REMOTE_CACHE_OPTIONS"))
  {
    useClientSpecifiedTimeout = boolStringCheck(useCfg);
  }
}

void readClientProcessingTimeout()
{
  int timeoutCfg(0);
  if (Global::hasConfig()
      && Global::config().getValue("TRX_TIMEOUT", timeoutCfg, "TSE_SERVER")
      && timeoutCfg > 0)
  {
    clientProcessingTimeout = timeoutCfg;
  }
  else if (Global::hasConfig()
           && Global::config().getValue("CLIENT_PROCESSING_TIMEOUT", timeoutCfg, "REMOTE_CACHE_OPTIONS")
           && timeoutCfg > 0)
  {
    clientProcessingTimeout = timeoutCfg;
  }
}

void readClientPoolSamplingInterval()
{
  if (Global::hasConfig())
  {
    int cfgValue(0);
    if (Global::config().getValue("CLIENT_POOL_SAMPLING_INTERVAL", cfgValue, "REMOTE_CACHE_OPTIONS")
        && cfgValue > 0)
    {
      clientPoolSamplingInterval = cfgValue;
    }
  }
}

void readClientPoolAdjustInterval()
{
  if (Global::hasConfig())
  {
    int cfgValue(0);
    if (Global::config().getValue("CLIENT_POOL_ADJUST_INTERVAL", cfgValue, "REMOTE_CACHE_OPTIONS")
        && cfgValue > 0)
    {
      clientPoolAdjustInterval = cfgValue;
    }
  }
}

void readServerSendTimeout()
{
  int toCfg(0);
  if (Global::hasConfig()
      && Global::config().getValue("SERVER_SEND_TIMEOUT", toCfg, "REMOTE_CACHE_OPTIONS")
      && toCfg > 0)
  {
    serverSendTimeout = toCfg;
  }
}

void readClientSendTimeout()
{
  int toCfg(0);
  if (Global::hasConfig()
      && Global::config().getValue("CLIENT_SEND_TIMEOUT", toCfg, "REMOTE_CACHE_OPTIONS")
      && toCfg > 0)
  {
    clientSendTimeout = toCfg;
  }
}

void readMasterAttributes(const std::string& key,
                          std::string& host,
                          std::string& port)
{
  typedef boost::tokenizer<boost::char_separator<char> > Tokens;
  boost::char_separator<char> sep("|", "", boost::keep_empty_tokens);
  std::string cfgValue;
  if (Global::hasConfig()
      && Global::config().getValue(key, cfgValue, "REMOTE_CACHE_OPTIONS")
      && !cfgValue.empty())
  {
    Tokens tokens(cfgValue, sep);
    int count(ATTRIBUTES_HOST);
    for (const auto& tok : tokens)
    {
      switch (count)
      {
      case ATTRIBUTES_HOST:
        host.assign(tok);
        boost::to_upper(host);
        boost::trim(host);
        break;
      case ATTRIBUTES_PORT:
        port.assign(tok);
        boost::trim(port);
        break;
      default:
        break;
      }
      ++count;
    }
  }
}

void readMasterAllDatatypes()
{
  std::string stringCfg;
  if (Global::hasConfig()
      && Global::config().getValue("MASTER_ALL_DATATYPES", stringCfg, "REMOTE_CACHE_OPTIONS"))
  {
    masterAllDatatypes = boolStringCheck(stringCfg);
  }
}

void readHealthcheckTimeout()
{
  int cfgValue(0);
  if (Global::hasConfig()
      && Global::config().getValue("HEALTHCHECK_TIMEOUT", cfgValue, "REMOTE_CACHE_OPTIONS")
      && cfgValue > 0)
  {
    healthcheckTimeout = cfgValue;
  }
}

void readQueueTolerance()
{
  double cfgValue(0);
  if (Global::hasConfig()
      && Global::config().getValue("QUEUE_TOLERANCE", cfgValue, "REMOTE_CACHE_OPTIONS")
      && cfgValue > 0)
  {
    queueTolerance = cfgValue;
  }
}

void readHealthcheckPeriod()
{
  int cfgValue(0);
  if (Global::hasConfig()
      && Global::config().getValue("HEALTHCHECK_PERIOD", cfgValue, "REMOTE_CACHE_OPTIONS")
      && cfgValue > 0)
  {
    healthcheckPeriod = cfgValue;
  }
}

void readIdleMasterTimeout()
{
  int timeoutCfg(0);
  if (Global::hasConfig()
      && Global::config().getValue("IDLE_MASTER_TIMEOUT", timeoutCfg, "REMOTE_CACHE_OPTIONS")
      && timeoutCfg > 0)
  {
    idleMasterTimeout = timeoutCfg;
  }
}

void readIdleSlaveTimeout()
{
  int timeoutCfg(0);
  if (Global::hasConfig()
      && Global::config().getValue("IDLE_SLAVE_TIMEOUT", timeoutCfg, "REMOTE_CACHE_OPTIONS")
      && timeoutCfg > 0)
  {
    idleSlaveTimeout = timeoutCfg;
  }
}

void readServerReceiveTimeout()
{
  int toCfg(0);
  if (Global::hasConfig()
      && Global::config().getValue("SERVER_RECEIVE_TIMEOUT", toCfg, "REMOTE_CACHE_OPTIONS")
      && toCfg > 0)
  {
    serverReceiveTimeout = toCfg;
  }
}

void findServerPort()
{
  serverPort.erase();
  for (const auto& pr : attributesMap)
  {
    if (pr.second.primary()._sameHost && !pr.second.primary()._port.empty())
    {
      serverPort = pr.second.primary()._port;
    }
    else if (pr.second.secondary()._sameHost && !pr.second.secondary()._port.empty())
    {
      serverPort = pr.second.secondary()._port;
    }
  }
}

void readDebug()
{
  std::string cfgValue;
  if (Global::hasConfig()
      && Global::config().getValue("RC_DEBUG", cfgValue, "REMOTE_CACHE_OPTIONS"))
  {
    debugRC = boolStringCheck(cfgValue);
  }
}

void readAttributesMap()
{
  attributesMap.clear();
  if (enabled)
  {
    typedef boost::tokenizer<boost::char_separator<char> > Tokens;
    boost::char_separator<char> sep("|", "", boost::keep_empty_tokens);
    std::vector<ConfigMan::NameValue> cfgValues;
    if (Global::hasConfig())
    {
      Global::config().getValues(cfgValues, "REMOTE_CACHE_SERVERS");
    }
    for (const auto& cfg : cfgValues)
    {
      std::string nameUpper(cfg.name);
      boost::to_upper(nameUpper);
      RCServerAttributes attrs;
      bool typeEnabled(false);
      int count(ATTRIBUTES_ENABLED);
      Tokens tokens(cfg.value, sep);
      for (const auto& tok : tokens)
      {
        switch (count)
        {
        case ATTRIBUTES_ENABLED:
          typeEnabled = boolStringCheck(tok);
          break;
        case ATTRIBUTES_HOST:
          {
            std::string upperHost(tok);
            boost::to_upper(upperHost);
            attrs.primary()._host = upperHost;
            if (upperHost == thisHost || masterAllDatatypes)
            {
              attrs.primary()._sameHost = true;
            }
          }
          break;
        case ATTRIBUTES_PORT:
          attrs.primary()._port = tok;
          break;
        case ATTRIBUTES_SECONDARY_HOST:
          {
            std::string upperHost(tok);
            boost::to_upper(upperHost);
            attrs.secondary()._host = upperHost;
            if (upperHost == thisHost || masterAllDatatypes)
            {
              attrs.secondary()._sameHost = true;
            }
          }
          break;
        case ATTRIBUTES_SECONDARY_PORT:
          attrs.secondary()._port = tok;
          break;
        default:
          break;
        }
        ++count;
      }
      if (typeEnabled)
      {
        if (attrs.primary().empty())
        {
          attrs.primary()._host = defaultMasterHost;
          attrs.primary()._port = defaultMasterPort;
          if (defaultMasterHost == thisHost || masterAllDatatypes)
          {
            attrs.primary()._sameHost = true;
          }
        }
        if (attrs.secondary().empty())
        {
          attrs.secondary()._host = defaultSecondaryHost;
          attrs.secondary()._port = defaultSecondaryPort;
          if (defaultSecondaryHost == thisHost || masterAllDatatypes)
          {
            attrs.secondary()._sameHost = true;
          }
        }
        if (!attrs.empty())
        {
          attributesMap.emplace(nameUpper, attrs);
        }
      }
    }
  }
  findServerPort();
}

struct ChangeableAttributes
{

ChangeableAttributes()
{
  readAll();
}

static void readAll()
{
  readEnabled();
  readPersistentConnections();
  readIgnoreDatabaseMismatch();
  readClientCacheUpdateDelay();
  readMasterAllDatatypes();
  readThreadPoolSize();
  readMaxNumberClients();
  readMinClientsRatio();
  readAsynchronousHealthcheck();
  readLdcDelay();
  readHealthcheckTimeout();
  readHealthcheckEnabled();
  readQueueTolerance();
  readHealthcheckPeriod();
  readClientConnectionTimeout();
  readLinger();
  readLingerTime();
  readKeepAlive();
  readUseClientSpecifiedTimeout();
  readClientProcessingTimeout();
  readCheckBaseline();
  readCacheUpdateDetectionInterval();
  readClientPoolSamplingInterval();
  readClientPoolAdjustInterval();
  readStatsSamplingInterval();
  readStatsLoggingInterval();
  readIdleMasterTimeout();
  readIdleSlaveTimeout();
  readServerReceiveTimeout();
  readServerSendTimeout();
  readClientSendTimeout();
  readMasterAttributes("DEFAULT_MASTER", defaultMasterHost, defaultMasterPort);
  readMasterAttributes("SECONDARY_DEFAULT_MASTER", defaultSecondaryHost, defaultSecondaryPort);
  readAttributesMap();
  readDebug();
}

};

}// namespace

bool isEnabled()
{
  static const ChangeableAttributes changeableAttributes;
  return enabled;
}

void setEnabled(bool parm)
{
  enabled = parm;
  readConfig();
}

void setThreadPoolSize(int parm)
{
  threadPoolSize = parm;
}

bool isKeepAlive()
{
  return keepAlive;
}

int getIdleSlaveTimeout()
{
  return idleSlaveTimeout;
}

int getIdleMasterTimeout()
{
  return idleMasterTimeout;
}

int getClientSendTimeout()
{
  return clientSendTimeout;
}

bool usePersistentConnections()
{
  return persistentConnections;
}

void setPersistentConnections(bool parm)
{
  persistentConnections = parm;
}

void setMasterAttributes(const std::string& host,
                         const std::string& port)
{
  defaultMasterHost = host;
  boost::to_upper(defaultMasterHost);
  boost::trim(defaultMasterHost);
  defaultMasterPort = port;
  boost::trim(defaultMasterPort);
  readConfig();
}

void getMasterAttributes(std::string& host,
                         std::string& port)
{
  host = defaultMasterHost;
  port = defaultMasterPort;
}

void setSecondaryMasterAttributes(const std::string& host,
                                  const std::string& port)
{
  defaultSecondaryHost = host;
  boost::to_upper(defaultSecondaryHost);
  boost::trim(defaultSecondaryHost);
  defaultSecondaryPort = port;
  boost::trim(defaultSecondaryPort);
  readConfig();
}

void getSecondaryMasterAttributes(std::string& host,
                                  std::string& port)
{
  host = defaultSecondaryHost;
  port = defaultSecondaryPort;
}

bool isLinger()
{
  return linger;
}

int getLingerTime()
{
  return lingerTime;
}

int getClientRetryInterval(StatusType status)
{
  std::string statusStr;
  switch (status)
  {
  case RC_CONNECTION_REFUSED:
  case RC_HEALTHCHECK_TIMEOUT:
    statusStr.assign("CONNECTION_PROBLEM");
    break;
  case RC_READ_ERROR:
  case RC_WRITE_ERROR:
    statusStr.assign("READ_WRITE_ERROR");
    break;
  case RC_ADDRESS_NOT_AVAILABLE:
    statusStr.assign("ADDRESS_NOT_AVAILABLE");
    break;
  case RC_BASELINE_MISMATCH:
  case RC_DAO_VERSION_MISMATCH:
    statusStr.assign("VERSION_MISMATCH");
    break;
  case RC_DATABASE_MISMATCH:
    statusStr.assign("DATABASE_MISMATCH");
    break;
  case RC_NOT_SERVER_FOR_DATATYPE:
    statusStr.assign("NOT_SERVER_FOR_DATATYPE");
    break;
  case RC_SERVER_BUSY:
    statusStr.assign("SERVER_BUSY");
    break;
  case RC_SERVER_NOT_READY:
    statusStr.assign("SERVER_NOT_READY");
    break;
  case RC_SERVER_ERROR:
    statusStr.assign("INTERNAL_SERVER_ERROR");
    break;
  case RC_SERVER_TIMEOUT:
    statusStr.assign("SERVER_TIMEOUT_RETRY_INTERVAL");
    break;
  case RC_CLIENT_CONNECTION_TIMEOUT:
    statusStr.assign("CLIENT_CONNECTION_TIMEOUT_RETRY_INTERVAL");
    break;
  case RC_CLIENT_PROCESSING_TIMEOUT:
    statusStr.assign("CLIENT_PROCESSING_TIMEOUT_RETRY_INTERVAL");
    break;
  case RC_BAD_REQUEST:
    statusStr.assign("BAD_REQUEST");
    break;
  case RC_QUEUE_LIMIT_EXCEEDED:
    statusStr.assign("QUEUE_LIMIT_RETRY");
    break;
  case RC_MAX_NUMBER_CLIENTS_EXCEEDED:
    statusStr.assign("RC_CLIENT_BUSY");
    break;
  case RC_NOT_IMPLEMENTED:
    statusStr.assign("NOT_IMPLEMENTED");
    break;
  case RC_MASTER_CACHE_UPDATE_STOPPED:
    statusStr.assign("CACHE_UPDATE_RETRY");
    break;
  case RC_WRONG_MAGIC_STRING:
    statusStr.assign("WRONG_MAGIC_STRING_RETRY");
    break;
  case RC_HEADER_VERSION_MISMATCH:
    statusStr.assign("HEADER_VERSION_MISMATCH_RETRY");
    break;
  case RC_INCOMPATIBLE_MODE:
  case RC_MASTER_NONHISTORICAL:
    statusStr.assign("RC_INCOMPATIBLE_MODE_RETRY");
    break;
  default:
    break;
  }
  int defaultRetryInterval(0);
  int retryIntervalCfg(0);
  if (!statusStr.empty()
      && Global::hasConfig()
      && Global::config().getValue(statusStr, retryIntervalCfg, "REMOTE_CACHE_OPTIONS")
      && retryIntervalCfg > 0)
  {
    return retryIntervalCfg;
  }
  return defaultRetryInterval;
}

int getClientCacheUpdateDelay()
{
  return clientCacheUpdateDelay;
}

int getCacheUpdateDetectionInterval()
{
  return cacheUpdateDetectionInterval;
}

const std::string& getServerPort()
{
  std::unique_lock<std::mutex> lock(mutex);
  return serverPort;
}

int getThreadPoolSize()
{
  return threadPoolSize;
}

int getMaxNumberClients()
{
  return maxNumberClients;
}

double getMinClientsRatio()
{
  return minClientsRatio;
}

void setMaxNumberClients(int parm)
{
  maxNumberClients = parm;
}

void setMinClientsRatio(double parm)
{
  minClientsRatio = parm;
}

int getServerReceiveTimeout()
{
  return serverReceiveTimeout;
}

int getServerSendTimeout()
{
  return serverSendTimeout;
}

int getClientConnectionTimeout()
{
  return clientConnectionTimeout;
}

int getLdcDelay()
{
  return ldcDelay;
}

void setLdcDelay(int parm)
{
  ldcDelay = parm;
}

int getClientProcessingTimeout()
{
  return clientProcessingTimeout;
}

bool isMasterAllDatatypes()
{
  return masterAllDatatypes;
}

bool getIgnoreDatabaseMismatch()
{
  return ignoreDBMismatch;
}

const RCServerAttributes& getServer(const std::string& dataType)
{
  std::string typeUpper(dataType);
  boost::to_upper(typeUpper);
  std::unique_lock<std::mutex> lock(mutex);
  auto it(attributesMap.find(typeUpper));
  if (it != attributesMap.end())
  {
    return it->second;
  }
  static RCServerAttributes emptyAttributes;
  return emptyAttributes;
}

void readConfig()
{
  std::unique_lock<std::mutex> lock(mutex);
  readAttributesMap();
}

bool isClientSpecifiedTimeout()
{
  return useClientSpecifiedTimeout;
}

int getStatsSamplingInterval()
{
  return statsSamplingInterval;
}

int getStatsLoggingInterval()
{
  return statsLoggingInterval;
}

bool getCheckBaseline()
{
  return checkBaseline;
}

int getMaxDBConnections()
{
  if (!isEnabled())
  {
    return -1;
  }
  int defaultConnections(20);
  if (Global::hasConfig())
  {
    std::string serverPort(getServerPort());
    int maxNumberConnections(defaultConnections);
    int cfgValue(0);
    if (Global::config().getValue("CLIENT_DB_CONNECTIONS", cfgValue, "REMOTE_CACHE_OPTIONS")
        && cfgValue > 0)
    {
      maxNumberConnections = cfgValue;
    }
    if (serverPort.empty())
    {
      return maxNumberConnections;
    }
    else
    {
      if (Global::config().getValue("MASTER_DB_CONNECTIONS", cfgValue, "REMOTE_CACHE_OPTIONS")
          && cfgValue > 0)
      {
        return cfgValue;
      }
      else
      {
        return maxNumberConnections;
      }
    }
  }
  return defaultConnections;
}

int getClientPoolSamplingInterval()
{
  return clientPoolSamplingInterval;
}

int getClientPoolAdjustInterval()
{
  return clientPoolAdjustInterval;
}

bool isHealthcheckEnabled()
{
  return healthcheckEnabled;
}

void setHealthcheckEnabled(int parm)
{
  healthcheckEnabled = parm;
}

bool isHostStatus(StatusType status)
{
  switch (status)
  {
  case RC_BASELINE_MISMATCH:
  case RC_CLIENT_CONNECTION_TIMEOUT:
  case RC_CONNECTION_REFUSED:
  case RC_DATABASE_MISMATCH:
  case RC_HEADER_VERSION_MISMATCH:
  case RC_MASTER_CACHE_UPDATE_STOPPED:
  case RC_SERVER_NOT_READY:
  case RC_QUEUE_LIMIT_EXCEEDED:
  case RC_HEALTHCHECK_TIMEOUT:
  case RC_INCOMPATIBLE_MODE:
  case RC_MASTER_NONHISTORICAL:
    return true;
    break;
  default:
    break;
  }
  return false;
}

int getHealthcheckTimeout()
{
  return healthcheckTimeout;
}

void setHealthcheckTimeout(int parm)
{
  healthcheckTimeout = parm;
}

void resetClientPool(const RCServerAttributes& attrs)
{
  std::unique_lock<std::mutex> lock(mutex);
  for (auto& pair : attributesMap)
  {
    if (pair.second == attrs)
    {
      if (attrs.isSecondary())
      {
        pair.second.setSecondary();
      }
      else
      {
        pair.second.setPrimary();
      }
    }
  }
}

bool sameHost(const std::string& host)
{
  std::string hostUpper(host);
  boost::to_upper(hostUpper);
  return thisHost == hostUpper;
}

int getHealthcheckPeriod()
{
  return healthcheckPeriod;
}

double getQueueTolerance()
{
  return queueTolerance;
}

bool allowHistorical()
{
  bool allow(false);
  std::string stringCfg;
  if (Global::hasConfig()
      && Global::config().getValue("MASTER_ALLOW_HISTORICAL", stringCfg, "REMOTE_CACHE_OPTIONS"))
  {
    allow = boolStringCheck(stringCfg);
  }
  return allow;
}

bool getAsynchronousHealthcheck()
{
  return asynchronousHealthcheck;
}

void setAsynchronousHealthcheck(bool parm)
{
  asynchronousHealthcheck = parm;
}

void setQueueTolerance(double parm)
{
  queueTolerance = parm;
}

void setHealthcheckPeriod(int parm)
{
  healthcheckPeriod = parm;
}

void setClientConnectionTimeout(int parm)
{
  clientConnectionTimeout = parm;
}

void setLinger(bool parm)
{
  linger = parm;
}

void setLingerTime(int parm)
{
  lingerTime = parm;
}

void setKeepAlive(bool parm)
{
  keepAlive = parm;
}

void setUseClientSpecifiedTimeout(bool parm)
{
  useClientSpecifiedTimeout = parm;
}

void setClientProcessingTimeout(int parm)
{
  clientProcessingTimeout = parm;
}

void setCheckBaseline(bool parm)
{
  checkBaseline = parm;
}

void setIgnoreDatabaseMismatch(bool parm)
{
  ignoreDBMismatch = parm;
}

void setCacheUpdateDetectionInterval(int parm)
{
  cacheUpdateDetectionInterval = parm;
}

void setClientPoolSamplingInterval(int parm)
{
  clientPoolSamplingInterval = parm;
}

void setClientPoolAdjustInterval(int parm)
{
  clientPoolAdjustInterval = parm;
}

void setMasterAllDatatypes(bool parm)
{
  masterAllDatatypes = parm;
}

void setStatsSamplingInterval(int parm)
{
  statsSamplingInterval = parm;
}

void setStatsLoggingInterval(int parm)
{
  statsLoggingInterval = parm;
}

void setIdleMasterTimeout(int parm)
{
  idleMasterTimeout = parm;
}

void setIdleSlaveTimeout(int parm)
{
  idleSlaveTimeout = parm;
}

void setServerReceiveTimeout(int parm)
{
  serverReceiveTimeout = parm;
}

void setServerSendTimeout(int parm)
{
  serverSendTimeout = parm;
}

void setClientSendTimeout(int parm)
{
  clientSendTimeout = parm;
}

void resetParameters()
{
  std::unique_lock<std::mutex> lock(mutex);
  ChangeableAttributes::readAll();
}

void writeAttributesMap(std::string& out)
{
  std::ostringstream os;
  std::unique_lock<std::mutex> lock(mutex);
  for (const auto& pr : attributesMap)
  {
    os << pr.first << " \t";
    os << pr.second.primary()._host << ':' << pr.second.primary()._port;
    os << ' ' << pr.second.secondary()._host << ':' << pr.second.secondary()._port;
    os << ' ' << (pr.second.isSecondary() ? "secondary" : "primary");
    os << '\n';
  }
  out = os.str();
}

void setDebug(bool parm)
{
  debugRC = parm;
}

bool isDebug()
{
  return debugRC;
}

}// ReadConfig

}// RemoteCache

}// tse
