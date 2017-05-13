#pragma once

#include <ctime>
#include <string>

namespace tse
{

namespace RemoteCache
{

void startRC();
void stopRC();

void start(std::string& msg,
           const std::string& masterHost = "",
           const std::string& masterPort = "",
           const std::string& secondaryHost = "",
           const std::string& secondaryPort = "");
bool stop(std::string& msg);
std::time_t serverStartTime();
void setPersistent(std::string& msg,
                   bool parm);
void setLdcDelay(std::string& msg,
                 int parm);
void setHealthcheckTimeout(std::string& msg,
                           int parm);
void setThreadPoolSize(std::string& msg,
                       int parm);
void setMaxNumberClients(std::string& msg,
                         int parm);
void setAsynchronousHealthcheck(std::string& msg,
                                bool parm);
void setEnableHealthcheck(std::string& msg,
                          bool parm);
void setHealthcheckPeriod(std::string& msg,
                          int parm);
void setQueueTolerance(std::string& msg,
                       double parm);
void setClientConnectionTimeout(std::string& msg,
                                int parm);
void setLinger(std::string& msg,
               bool parm);
void setLingerTime(std::string& msg,
                   int parm);
void setKeepAlive(std::string& msg,
                  bool parm);
void setUseClientSpecifiedTimeout(std::string& msg,
                                  bool parm);
void setClientProcessingTimeout(std::string& msg,
                                int parm);
void setCheckBaseline(std::string& msg,
                      bool parm);
void setIgnoreDatabaseMismatch(std::string& msg,
                               bool parm);
void setCacheUpdateDetectionInterval(std::string& msg,
                                     int parm);
void setClientPoolSamplingInterval(std::string& msg,
                                   int parm);
void setClientPoolAdjustInterval(std::string& msg,
                                 int parm);
void setMasterAllDatatypes(std::string& msg,
                           bool parm);
void setStatsSamplingInterval(std::string& msg,
                              int parm);
void setStatsLoggingInterval(std::string& msg,
                             int parm);
void setIdleMasterTimeout(std::string& msg,
                          int parm);
void setIdleSlaveTimeout(std::string& msg,
                         int parm);
void setServerReceiveTimeout(std::string& msg,
                             int parm);
void setServerSendTimeout(std::string& msg,
                          int parm);
void setClientSendTimeout(std::string& msg,
                          int parm);
void setMinClientsRatio(std::string& msg,
                        double parm);
void setDebug(std::string& msg,
              bool parm);
void resetParameters(std::string& msg);

void getParameters(std::string& msg);

void getCaches(std::string& msg);

}// RemoteCache

}// tse
