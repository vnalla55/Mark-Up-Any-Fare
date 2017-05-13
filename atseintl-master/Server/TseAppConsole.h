//-------------------------------------------------------------------
//
//  File:        TseAppConsole.h
//  Created:     May 29, 2005
//  Authors:     Mark Kasprowicz
//
//  Copyright Sabre 2005
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "AppConsole/AppConsole.h"

namespace ac
{
class SocketUtils;
}

namespace tse
{
class TseServer;
class Logger;

class TseAppConsole final : public ac::AppConsole
{
public:
  bool start(const long port, TseServer* srv = nullptr, bool allowCacheTools = false);

  virtual bool
  processMessage(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp) override;

  bool allowCacheTools() const { return _allowCacheTools; }
  bool& allowCacheTools() { return _allowCacheTools; }

private:
  TseServer* _srv = nullptr;
  bool _allowCacheTools = false;

  struct cacheControlList;

  Logger logger();

  bool processStop(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processStats(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processGetLogLevel(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processSetLogLevel(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processSetLogLevelRegEx(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processGetAllLoggers(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processSetAllLogLevels(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processResetCounters(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processDetails(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processRedeploy(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processActivate(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processCacheStats(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool
  processCompressedCacheStats(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processDisposeCache(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processDAOCoverageStats(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processTOElapsed(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processCacheFlush(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processCacheUpdates(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool
  processReloadDatabaseConfig(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processErrorCounts(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processMemKeys(const ac::SocketUtils::Message& req,
                      ac::SocketUtils::Message& rsp,
                      bool inclValues = false);
  bool processDiskKeys(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processLDCTypes(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processCompareValues(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processCompareDbValues(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processInvalidateKey(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processInsertDummyObject(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processInjectCacheNotify(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processObjectExists(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processCacheParm(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool
  processDatabaseConnections(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processCacheMemory(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processPoolMemory(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processQueryObject(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processGetNationLoadlist(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processLDCStats(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processBDBPanic(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processDynamicCfgLoading(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp);
  bool processServicesLatency(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp) const;
  bool processServerConfig(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp) const;
  bool processServerCPU(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp) const;
  bool processQueryElapsed(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp) const;
  bool processRCHealthCheck(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp) const;
  bool processRCDisable(const ac::SocketUtils::Message& req,
                        ac::SocketUtils::Message& rsp) const;
  bool processRCEnable(const ac::SocketUtils::Message& req,
                       ac::SocketUtils::Message& rsp) const;
  bool processRCPersistent(const ac::SocketUtils::Message& req,
                           ac::SocketUtils::Message& rsp) const;
  bool processRCNonPersistent(const ac::SocketUtils::Message& req,
                              ac::SocketUtils::Message& rsp) const;
  bool processRCLdcDelay(const ac::SocketUtils::Message& req,
                         ac::SocketUtils::Message& rsp) const;
  bool processRCGetParameters(const ac::SocketUtils::Message& req,
                              ac::SocketUtils::Message& rsp) const;
  bool processRCGetCaches(const ac::SocketUtils::Message& req,
                          ac::SocketUtils::Message& rsp) const;
  bool processRCThreadPoolSize(const ac::SocketUtils::Message& req,
                               ac::SocketUtils::Message& rsp) const;
  bool processRCMaxNumberClients(const ac::SocketUtils::Message& req,
                                 ac::SocketUtils::Message& rsp) const;
  bool processRCHealthcheckTimeout(const ac::SocketUtils::Message& req,
                                   ac::SocketUtils::Message& rsp) const;
  bool processRCAsynchronousHealthcheck(const ac::SocketUtils::Message& req,
                                        ac::SocketUtils::Message& rsp) const;
  bool processRCEnableHealthcheck(const ac::SocketUtils::Message& req,
                                  ac::SocketUtils::Message& rsp) const;
  bool processRCHealthcheckPeriod(const ac::SocketUtils::Message& req,
                                  ac::SocketUtils::Message& rsp) const;
  bool processRCQueueTolerance(const ac::SocketUtils::Message& req,
                               ac::SocketUtils::Message& rsp) const;
  bool processRCClientConnectTimeout(const ac::SocketUtils::Message& req,
                                     ac::SocketUtils::Message& rsp) const;
  bool processRCLinger(const ac::SocketUtils::Message& req,
                       ac::SocketUtils::Message& rsp) const;
  bool processRCLingerTime(const ac::SocketUtils::Message& req,
                           ac::SocketUtils::Message& rsp) const;
  bool processRCKeepAlive(const ac::SocketUtils::Message& req,
                          ac::SocketUtils::Message& rsp) const;
  bool processRCUseClientSpecifiedTimeout(const ac::SocketUtils::Message& req,
                                          ac::SocketUtils::Message& rsp) const;
  bool processRCClientProcessingTimeout(const ac::SocketUtils::Message& req,
                                        ac::SocketUtils::Message& rsp) const;
  bool processRCCheckBaseline(const ac::SocketUtils::Message& req,
                              ac::SocketUtils::Message& rsp) const;
  bool processRCIgnoreDatabaseMismatch(const ac::SocketUtils::Message& req,
                                       ac::SocketUtils::Message& rsp) const;
  bool processRCCacheUpdateDetectionInterval(const ac::SocketUtils::Message& req,
                                             ac::SocketUtils::Message& rsp) const;
  bool processRCClientPoolSamplingInterval(const ac::SocketUtils::Message& req,
                                           ac::SocketUtils::Message& rsp) const;
  bool processRCMasterAllDatatypes(const ac::SocketUtils::Message& req,
                                   ac::SocketUtils::Message& rsp) const;
  bool processRCStatsSamplingInterval(const ac::SocketUtils::Message& req,
                                      ac::SocketUtils::Message& rsp) const;
  bool processRCStatsLoggingInterval(const ac::SocketUtils::Message& req,
                                     ac::SocketUtils::Message& rsp) const;
  bool processRCIdleMasterTimeout(const ac::SocketUtils::Message& req,
                                  ac::SocketUtils::Message& rsp) const;
  bool processRCIdleSlaveTimeout(const ac::SocketUtils::Message& req,
                                 ac::SocketUtils::Message& rsp) const;
  bool processRCServerReceiveTimeout(const ac::SocketUtils::Message& req,
                                     ac::SocketUtils::Message& rsp) const;
  bool processRCServerSendTimeout(const ac::SocketUtils::Message& req,
                                  ac::SocketUtils::Message& rsp) const;
  bool processRCClientSendTimeout(const ac::SocketUtils::Message& req,
                                  ac::SocketUtils::Message& rsp) const;
  bool processRCMinClientsRatio(const ac::SocketUtils::Message& req,
                                ac::SocketUtils::Message& rsp) const;
  bool processRCClientPoolAdjustInterval(const ac::SocketUtils::Message& req,
                                         ac::SocketUtils::Message& rsp) const;
  bool processRCDebug(const ac::SocketUtils::Message& req,
                      ac::SocketUtils::Message& rsp) const;
  bool processRCResetParameters(const ac::SocketUtils::Message& req,
                                ac::SocketUtils::Message& rsp) const;
  bool checkAllowCacheTools(std::ostream& os);

  bool shutdown(const std::string& cmd,
                const std::string* filename = nullptr,
                const std::string* contents = nullptr);
  static bool dirExists(const std::string& dir);
}; // End class

} // End namespace
