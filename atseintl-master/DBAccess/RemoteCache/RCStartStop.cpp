#include "DBAccess/RemoteCache/RCStartStop.h"

#include "DBAccess/RemoteCache/ReadConfig.h"
#include "DBAccess/RemoteCache/ASIOClient/ClientManager.h"
#include "DBAccess/RemoteCache/ASIOServer/ASIOServer.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"

namespace tse
{
 
FIXEDFALLBACK_DECL(fallbackRCCommandInterface);

namespace RemoteCache
{

namespace
{

Logger statsLogger("atseintl.RemoteCache.RCStatistics");

}// namespace

std::time_t serverStartTime()
{
  static const std::time_t started(std::time(nullptr));
  return started;
}

void startRC()
{
  serverStartTime();
  if (ReadConfig::isEnabled())
  {
    std::string dummy;
    getParameters(dummy);
    ASIOServer::start();
    ClientManager::start();
  }
}

void stopRC()
{
  ClientManager::stop();
  ASIOServer::stop();
}

void start(std::string& msg,
           const std::string& masterHost,
           const std::string& masterPort,
           const std::string& secondaryHost,
           const std::string& secondaryPort)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  if (!(masterHost.empty() || masterPort.empty()))
  {
    ReadConfig::setMasterAttributes(masterHost, masterPort);
  }
  if (!(secondaryHost.empty() || secondaryPort.empty()))
  {
    ReadConfig::setSecondaryMasterAttributes(secondaryHost, secondaryPort);
  }
  std::string currentHost;
  std::string currentPort;
  ReadConfig::getMasterAttributes(currentHost, currentPort);
  if (!(currentHost.empty() || currentPort.empty()))
  {
    ReadConfig::setEnabled(true);
    startRC();
    msg.append("Remote cache started");
  }
  else
  {
    msg.append("ERROR:Remote cache could not be started");
  }
}

bool stop(std::string& msg)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return false;
  }
  ReadConfig::setEnabled(false);
  stopRC();
  msg.assign("Remote cache stopped");
  return true;
}

void setLdcDelay(std::string& msg,
                 int parm)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::ostringstream os;
  if (ReadConfig::getLdcDelay() != parm)
  {
    ReadConfig::setLdcDelay(parm);
    os << __FUNCTION__ << ":ldcDelay=" << parm;
  }
  else
  {
    os << "ERROR:" << __FUNCTION__ << ":ldcDelay is already " << parm;
  }
  msg.assign(os.str());
}

void setHealthcheckTimeout(std::string& msg,
                           int parm)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::ostringstream os;
  if (ReadConfig::getHealthcheckTimeout() != parm)
  {
    ReadConfig::setHealthcheckTimeout(parm);
    os << __FUNCTION__ << ":healthcheckTimeout=" << parm;
  }
  else
  {
    os << "ERROR:" << __FUNCTION__ << ":healthcheckTimeout is already " << parm;
  }
  msg.assign(os.str());
}

void setPersistent(std::string& msg,
                   bool parm)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::ostringstream os;
  if (ReadConfig::usePersistentConnections() != parm)
  {
    ReadConfig::setEnabled(false);
    stopRC();
    ReadConfig::setPersistentConnections(parm);
    os << __FUNCTION__ << ":persistent=" << std::boolalpha << parm  << ",RC stopped";
  }
  else
  {
    os << "ERROR:" << __FUNCTION__ << ":persistent is already " << std::boolalpha << parm;
  }
  msg.assign(os.str());
}

void setThreadPoolSize(std::string& msg,
                       int parm)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::ostringstream os;
  if (ReadConfig::getThreadPoolSize() != parm)
  {
    ReadConfig::setThreadPoolSize(parm);
    os << __FUNCTION__ << ":threadPoolSize=" << parm;
  }
  else
  {
    os << "ERROR:" << __FUNCTION__ << ":threadPoolSize is already " << parm;
  }
  msg.assign(os.str());
}

void setMaxNumberClients(std::string& msg,
                         int parm)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::ostringstream os;
  if (ReadConfig::getMaxNumberClients() != parm)
  {
    ReadConfig::setMaxNumberClients(parm);
    os << __FUNCTION__ << ":maxNumberClients=" << parm;
  }
  else
  {
    os << "ERROR:" << __FUNCTION__ << ":maxNumberClients is already " << parm;
  }
  msg.assign(os.str());
}

void setAsynchronousHealthcheck(std::string& msg,
                                bool parm)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::ostringstream os;
  if (ReadConfig::getAsynchronousHealthcheck() != parm)
  {
    ReadConfig::setAsynchronousHealthcheck(parm);
    os << __FUNCTION__ << ":asynchronousHealthcheck=" << std::boolalpha << parm;
  }
  else
  {
    os << "ERROR:" << __FUNCTION__ << ":asynchronousHealthcheck is already " << std::boolalpha << parm;
  }
  msg.assign(os.str());
}

void setEnableHealthcheck(std::string& msg,
                          bool parm)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::ostringstream os;
  if (ReadConfig::isHealthcheckEnabled() != parm)
  {
    ReadConfig::setHealthcheckEnabled(parm);
    os << __FUNCTION__ << ":healthcheckEnabled=" << std::boolalpha << parm;
  }
  else
  {
    os << "ERROR:" << __FUNCTION__ << ":healthcheckEnabled is already " << std::boolalpha << parm;
  }
  msg.assign(os.str());
}

void setHealthcheckPeriod(std::string& msg,
                          int parm)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::ostringstream os;
  if (ReadConfig::getHealthcheckPeriod() != parm)
  {
    ReadConfig::setHealthcheckPeriod(parm);
    os << __FUNCTION__ << ":healthcheckPeriod=" << parm;
  }
  else
  {
    os << "ERROR:" << __FUNCTION__ << ":healthcheckPeriod is already " << parm;
  }
  msg.assign(os.str());
}

void setQueueTolerance(std::string& msg,
                       double parm)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::ostringstream os;
  if (ReadConfig::getQueueTolerance() != parm)
  {
    ReadConfig::setQueueTolerance(parm);
    os << __FUNCTION__ << ":queueTolerance=" << parm;
  }
  else
  {
    os << "ERROR:" << __FUNCTION__ << ":queueTolerance is already " << parm;
  }
  msg.assign(os.str());
}

void setClientConnectionTimeout(std::string& msg,
                                int parm)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::ostringstream os;
  if (ReadConfig::getClientConnectionTimeout() != parm)
  {
    ReadConfig::setClientConnectionTimeout(parm);
    os << __FUNCTION__ << ":clientConnectionTimeout=" << parm;
  }
  else
  {
    os << "ERROR:" << __FUNCTION__ << ":clientConnectionTimeout is already " << parm;
  }
  msg.assign(os.str());
}

void setLinger(std::string& msg,
               bool parm)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::ostringstream os;
  if (ReadConfig::isLinger() != parm)
  {
    ReadConfig::setLinger(parm);
    os << __FUNCTION__ << ":linger=" << std::boolalpha << parm;
  }
  else
  {
    os << "ERROR:" << __FUNCTION__ << ":linger is already " << std::boolalpha << parm;
  }
  msg.assign(os.str());
}

void setLingerTime(std::string& msg,
                   int parm)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::ostringstream os;
  if (ReadConfig::getLingerTime() != parm)
  {
    ReadConfig::setLingerTime(parm);
    os << __FUNCTION__ << ":lingerTime=" << parm;
  }
  else
  {
    os << "ERROR:" << __FUNCTION__ << ":lingerTime is already " << parm;
  }
  msg.assign(os.str());
}

void setKeepAlive(std::string& msg,
                  bool parm)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::ostringstream os;
  if (ReadConfig::isKeepAlive() != parm)
  {
    ReadConfig::setKeepAlive(parm);
    os << __FUNCTION__ << ":keepAlive=" << std::boolalpha << parm;
  }
  else
  {
    os << "ERROR:" << __FUNCTION__ << ":keepAlive is already " << std::boolalpha << parm;
  }
  msg.assign(os.str());
}

void setUseClientSpecifiedTimeout(std::string& msg,
                                  bool parm)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::ostringstream os;
  if (ReadConfig::isClientSpecifiedTimeout() != parm)
  {
    ReadConfig::setUseClientSpecifiedTimeout(parm);
    os << __FUNCTION__ << ":useClientSpecifiedTimeout=" << std::boolalpha << parm;
  }
  else
  {
    os << "ERROR:" << __FUNCTION__ << ":useClientSpecifiedTimeout is already " << std::boolalpha << parm;
  }
  msg.assign(os.str());
}

void setClientProcessingTimeout(std::string& msg,
                                int parm)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::ostringstream os;
  if (ReadConfig::getClientProcessingTimeout() != parm)
  {
    ReadConfig::setClientProcessingTimeout(parm);
    os << __FUNCTION__ << ":clientProcessingTimeout=" << parm;
  }
  else
  {
    os << "ERROR:" << __FUNCTION__ << ":clientProcessingTimeout is already " << parm;
  }
  msg.assign(os.str());
}

void setCheckBaseline(std::string& msg,
                      bool parm)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::ostringstream os;
  if (ReadConfig::getCheckBaseline() != parm)
  {
    ReadConfig::setCheckBaseline(parm);
    os << __FUNCTION__ << ":checkBaseline=" << std::boolalpha << parm;
  }
  else
  {
    os << "ERROR:" << __FUNCTION__ << ":checkBaseline is already " << std::boolalpha << parm;
  }
  msg.assign(os.str());
}

void setIgnoreDatabaseMismatch(std::string& msg,
                               bool parm)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::ostringstream os;
  if (ReadConfig::getIgnoreDatabaseMismatch() != parm)
  {
    ReadConfig::setIgnoreDatabaseMismatch(parm);
    os << __FUNCTION__ << ":ignoreDatabaseMismatch=" << std::boolalpha << parm;
  }
  else
  {
    os << "ERROR:" << __FUNCTION__ << ":ignoreDatabaseMismatch is already " << std::boolalpha << parm;
  }
  msg.assign(os.str());
}

void setCacheUpdateDetectionInterval(std::string& msg,
                                     int parm)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::ostringstream os;
  if (ReadConfig::getCacheUpdateDetectionInterval() != parm)
  {
    ReadConfig::setCacheUpdateDetectionInterval(parm);
    os << __FUNCTION__ << ":cacheUpdateDetectionInterval=" << parm;
  }
  else
  {
    os << "ERROR:" << __FUNCTION__ << ":cacheUpdateDetectionInterval is already " << parm;
  }
  msg.assign(os.str());
}

void setClientPoolSamplingInterval(std::string& msg,
                                   int parm)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::ostringstream os;
  if (ReadConfig::getClientPoolSamplingInterval() != parm)
  {
    ReadConfig::setClientPoolSamplingInterval(parm);
    os << __FUNCTION__ << ":clientPoolSamplingInterval=" << parm;
  }
  else
  {
    os << "ERROR:" << __FUNCTION__ << ":clientPoolSamplingInterval is already " << parm;
  }
  msg.assign(os.str());
}

void setClientPoolAdjustInterval(std::string& msg,
                                 int parm)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::ostringstream os;
  if (ReadConfig::getClientPoolAdjustInterval() != parm)
  {
    ReadConfig::setClientPoolAdjustInterval(parm);
    os << __FUNCTION__ << ":clientPoolAdjustInterval=" << parm;
  }
  else
  {
    os << "ERROR:" << __FUNCTION__ << ":clientPoolAdjustInterval is already " << parm;
  }
  msg.assign(os.str());
}

void setMasterAllDatatypes(std::string& msg,
                           bool parm)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::ostringstream os;
  if (ReadConfig::isMasterAllDatatypes() != parm)
  {
    ReadConfig::setMasterAllDatatypes(parm);
    os << __FUNCTION__ << ":masterAllDatatypes=" << std::boolalpha << parm;
  }
  else
  {
    os << "ERROR:" << __FUNCTION__ << ":masterAllDatatypes is already " << std::boolalpha << parm;
  }
  msg.assign(os.str());
}

void setStatsSamplingInterval(std::string& msg,
                              int parm)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::ostringstream os;
  if (ReadConfig::getStatsSamplingInterval() != parm)
  {
    ReadConfig::setStatsSamplingInterval(parm);
    os << __FUNCTION__ << ":statsSamplingInterval=" << parm;
  }
  else
  {
    os << "ERROR:" << __FUNCTION__ << ":statsSamplingInterval is already " << parm;
  }
  msg.assign(os.str());
}

void setStatsLoggingInterval(std::string& msg,
                             int parm)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::ostringstream os;
  if (ReadConfig::getStatsLoggingInterval() != parm)
  {
    ReadConfig::setStatsLoggingInterval(parm);
    os << __FUNCTION__ << ":statsLoggingInterval=" << parm;
  }
  else
  {
    os << "ERROR:" << __FUNCTION__ << ":statsLoggingInterval is already " << parm;
  }
  msg.assign(os.str());
}

void setIdleMasterTimeout(std::string& msg,
                          int parm)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::ostringstream os;
  if (ReadConfig::getIdleMasterTimeout() != parm)
  {
    ReadConfig::setIdleMasterTimeout(parm);
    os << __FUNCTION__ << ":idleMasterTimeout=" << parm;
  }
  else
  {
    os << "ERROR:" << __FUNCTION__ << ":idleMasterTimeout is already " << parm;
  }
  msg.assign(os.str());
}

void setIdleSlaveTimeout(std::string& msg,
                         int parm)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::ostringstream os;
  if (ReadConfig::getIdleSlaveTimeout() != parm)
  {
    ReadConfig::setIdleSlaveTimeout(parm);
    os << __FUNCTION__ << ":idleSlaveTimeout=" << parm;
  }
  else
  {
    os << "ERROR:" << __FUNCTION__ << ":idleSlaveTimeout is already " << parm;
  }
  msg.assign(os.str());
}

void setServerReceiveTimeout(std::string& msg,
                             int parm)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::ostringstream os;
  if (ReadConfig::getServerReceiveTimeout() != parm)
  {
    ReadConfig::setServerReceiveTimeout(parm);
    os << __FUNCTION__ << ":serverReceiveTimeout=" << parm;
  }
  else
  {
    os << "ERROR:" << __FUNCTION__ << ":serverReceiveTimeout is already " << parm;
  }
  msg.assign(os.str());
}

void setServerSendTimeout(std::string& msg,
                          int parm)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::ostringstream os;
  if (ReadConfig::getServerSendTimeout() != parm)
  {
    ReadConfig::setServerSendTimeout(parm);
    os << __FUNCTION__ << ":serverSendTimeout=" << parm;
  }
  else
  {
    os << "ERROR:" << __FUNCTION__ << ":serverSendTimeout is already " << parm;
  }
  msg.assign(os.str());
}

void setClientSendTimeout(std::string& msg,
                          int parm)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::ostringstream os;
  if (ReadConfig::getClientSendTimeout() != parm)
  {
    ReadConfig::setClientSendTimeout(parm);
    os << __FUNCTION__ << ":clientSendTimeout=" << parm;
  }
  else
  {
    os << "ERROR:" << __FUNCTION__ << ":clientSendTimeout is already " << parm;
  }
  msg.assign(os.str());
}

void setMinClientsRatio(std::string& msg,
                        double parm)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::ostringstream os;
  if (ReadConfig::getMinClientsRatio() != parm)
  {
    ReadConfig::setMinClientsRatio(parm);
    os << __FUNCTION__ << ":minClientsRatio=" << parm;
  }
  else
  {
    os << "ERROR:" << __FUNCTION__ << ":minClientsRatio is already " << parm;
  }
  msg.assign(os.str());
}

void setDebug(std::string& msg,
              bool parm)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::ostringstream os;
  if (ReadConfig::isDebug() != parm)
  {
    ReadConfig::setDebug(parm);
    os << __FUNCTION__ << ":isDebug=" << std::boolalpha << parm;
  }
  else
  {
    os << "ERROR:" << __FUNCTION__ << ":isDebug is already " << std::boolalpha << parm;
  }
  msg.assign(os.str());
}

void resetParameters(std::string& msg)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::string tmp;
  stop(tmp);
  msg.append(tmp);
  msg.append(1, '\n');
  tmp.clear();
  ReadConfig::resetParameters();
  if (ReadConfig::isEnabled())
  {
    start(tmp);
    msg.append(tmp);
    msg.append(1, '\n');
    tmp.clear();
  }
  getParameters(tmp);
  msg.append(tmp);
  tmp.clear();
}

void getParameters(std::string& msg)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  std::ostringstream os;
  os << "enabled=" << std::boolalpha << ReadConfig::isEnabled() << '\n';
  std::string defaultHost;
  std::string defaultPort;
  ReadConfig::getMasterAttributes(defaultHost, defaultPort);
  std::string secondaryHost;
  std::string secondaryPort;
  ReadConfig::getSecondaryMasterAttributes(secondaryHost, secondaryPort);
  os << "defaultMaster=" << defaultHost << ':' << defaultPort << '\n'
     << "secondaryMaster=" << secondaryHost << ':' << secondaryPort << '\n';
  os << "clientCacheUpdateDelay=" << ReadConfig::getClientCacheUpdateDelay() << '\n';
  os << "serverPort=" << ReadConfig::getServerPort() << '\n';
  os << "threadPoolSize=" << ReadConfig::getThreadPoolSize() << '\n';
  os << "serverReceiveTimeout=" << ReadConfig::getServerReceiveTimeout() << '\n';
  os << "serverSendTimeout=" << ReadConfig::getServerSendTimeout() << '\n';
  os << "clientConnectionTimeout=" << ReadConfig::getClientConnectionTimeout() << '\n';
  os << "clientProcessingTimeout=" << ReadConfig::getClientProcessingTimeout() << '\n';
  os << "isClientSpecifiedTimeout=" << std::boolalpha << ReadConfig::isClientSpecifiedTimeout() << '\n';
  os << "maxNumberClients=" << ReadConfig::getMaxNumberClients() << '\n';
  os << "minClientsRatio=" << ReadConfig::getMinClientsRatio() << '\n';
  os << "isKeepAlive=" << std::boolalpha << ReadConfig::isKeepAlive() << '\n';
  os << "usePersistentConnections=" << std::boolalpha << ReadConfig::usePersistentConnections() << '\n';
  os << "isLinger=" << std::boolalpha << ReadConfig::isLinger() << '\n';
  os << "lingerTime=" << ReadConfig::getLingerTime() << '\n';
  os << "ldcDelay=" << ReadConfig::getLdcDelay() << '\n';
  os << "checkBaseline=" << std::boolalpha << ReadConfig::getCheckBaseline() << '\n';
  os << "ignoreDatabaseMismatch=" << std::boolalpha << ReadConfig::getIgnoreDatabaseMismatch() << '\n';
  os << "cacheUpdateDetectionInterval=" << ReadConfig::getCacheUpdateDetectionInterval() << '\n';
  os << "idleMasterTimeout=" << ReadConfig::getIdleMasterTimeout() << '\n';
  os << "idleSlaveTimeout=" << ReadConfig::getIdleSlaveTimeout() << '\n';
  os << "clientSendTimeout=" << ReadConfig::getClientSendTimeout() << '\n';
  os << "statsSamplingInterval=" << ReadConfig::getStatsSamplingInterval() << '\n';
  os << "statsLoggingInterval=" << ReadConfig::getStatsLoggingInterval() << '\n';
  os << "isMasterAllDatatypes=" << std::boolalpha << ReadConfig::isMasterAllDatatypes() << '\n';
  os << "maxDBConnections=" << ReadConfig::getMaxDBConnections() << '\n';
  os << "clientPoolSamplingInterval=" << ReadConfig::getClientPoolSamplingInterval() << '\n';
  os << "clientPoolAdjustInterval=" << ReadConfig::getClientPoolAdjustInterval() << '\n';
  os << "healthcheckEnabled=" << std::boolalpha << ReadConfig::isHealthcheckEnabled() << '\n';
  os << "healthcheckTimeout=" << ReadConfig::getHealthcheckTimeout() << '\n';
  os << "healthcheckPeriod=" << ReadConfig::getHealthcheckPeriod() << '\n';
  os << "queueTolerance=" << ReadConfig::getQueueTolerance() << '\n';
  os << "asynchronousHealthcheck=" << std::boolalpha << ReadConfig::getAsynchronousHealthcheck() << '\n';
  os << "isDebug=" << std::boolalpha << ReadConfig::isDebug() << '\n';
  msg.assign(os.str());
  LOG4CXX_INFO(statsLogger, os.str());
}

void getCaches(std::string& msg)
{
  if (fallback::fixed::fallbackRCCommandInterface())
  {
    msg.assign("Disabled");
    return;
  }
  ReadConfig::writeAttributesMap(msg);
  LOG4CXX_INFO(statsLogger, msg);
}

}// RemoteCache

}// tse
