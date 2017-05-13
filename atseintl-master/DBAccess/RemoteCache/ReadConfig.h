#pragma once

#include <string>
#include "DBAccess/RemoteCache/RemoteCacheHeader.h"

namespace tse
{

namespace RemoteCache
{

class RCServerAttributes;

namespace ReadConfig
{

bool isEnabled();
void setEnabled(bool parm);
int getClientCacheUpdateDelay();
int getClientRetryInterval(StatusType status);
const std::string& getServerPort();
int getThreadPoolSize();
int getServerReceiveTimeout();
int getServerSendTimeout();
int getClientConnectionTimeout();
int getClientProcessingTimeout();
bool isClientSpecifiedTimeout();
int getMaxNumberClients();
double getMinClientsRatio();
bool isKeepAlive();
bool usePersistentConnections();
void setPersistentConnections(bool parm);
bool isLinger();
int getLingerTime();
int getLdcDelay();
const RCServerAttributes& getServer(const std::string& dataType);
bool getIgnoreDatabaseMismatch();
int getCacheUpdateDetectionInterval();
int getIdleMasterTimeout();
int getIdleSlaveTimeout();
int getClientSendTimeout();
int getStatsSamplingInterval();
int getStatsLoggingInterval();
bool getCheckBaseline();
bool isMasterAllDatatypes();
int getMaxDBConnections();
int getClientPoolSamplingInterval();
int getClientPoolAdjustInterval();
bool isHealthcheckEnabled();
int getHealthcheckTimeout();
bool isHostStatus(StatusType status);
void resetClientPool(const RCServerAttributes& attrs);
bool sameHost(const std::string& host);
int getHealthcheckPeriod();
double getQueueTolerance();
bool getAsynchronousHealthcheck();
void setMasterAttributes(const std::string& host,
                         const std::string& port);
void getMasterAttributes(std::string& host,
                         std::string& port);
void setSecondaryMasterAttributes(const std::string& host,
                                  const std::string& port);
void getSecondaryMasterAttributes(std::string& host,
                                  std::string& port);
bool isDebug();
void setThreadPoolSize(int parm);
void setMaxNumberClients(int parm);
void setMinClientsRatio(double parm);
void setAsynchronousHealthcheck(bool parm);
void setLdcDelay(int parm);
void setHealthcheckEnabled(int parm);
void setHealthcheckTimeout(int parm);
void setQueueTolerance(double parm);
void setHealthcheckPeriod(int parm);
void setClientConnectionTimeout(int parm);
void setLinger(bool parm);
void setLingerTime(int parm);
void setKeepAlive(bool parm);
void setUseClientSpecifiedTimeout(bool parm);
void setClientProcessingTimeout(int parm);
void setCheckBaseline(bool parm);
void setIgnoreDatabaseMismatch(bool parm);
void setCacheUpdateDetectionInterval(int parm);
void setClientPoolSamplingInterval(int parm);
void setClientPoolAdjustInterval(int parm);
void setMasterAllDatatypes(bool parm);
void setStatsSamplingInterval(int parm);
void setStatsLoggingInterval(int parm);
void setIdleMasterTimeout(int parm);
void setIdleSlaveTimeout(int parm);
void setServerReceiveTimeout(int parm);
void setServerSendTimeout(int parm);
void setClientSendTimeout(int parm);
void setDebug(bool parm);
void resetParameters();
void writeAttributesMap(std::string& out);

} // ReadConfig

} // RemoteCache

} // tse
