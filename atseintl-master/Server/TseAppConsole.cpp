//-------------------------------------------------------------------
//
//  File:        TseAppConsole.cpp
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
#include "Server/TseAppConsole.h"

#include "Adapter/CacheNotifyControl.h"
#include "AppConsole/SocketUtils.h"
#include "Common/CacheStats.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/Config/DynamicConfigLoader.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/TseSrvStats.h"
#include "DBAccess/CacheControl.h"
#include "DBAccess/CacheManager.h"
#include "DBAccess/CacheRegistry.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/DataManager.h"
#include "DBAccess/DiskCache.h"
#include "DBAccess/HashKey.h"
#include "DBAccess/Nation.h"
#include "DBAccess/ObjectKey.h"
#include "DBAccess/RemoteCache/ASIOClient/ClientManager.h"
#include "DBAccess/RemoteCache/RCStartStop.h"
#include "Server/AppConsoleController.h"
#include "Server/TseServer.h"

#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <boost/tokenizer.hpp>
#include <log4cxx/spi/loggerrepository.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <db.h>
#include <dirent.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/types.h>

#define FORCE_HANDLES_OPEN                                                                         \
  bool priorDbSetting = DISKCACHE.setKeepDbHandlesOpen(true);                                      \
  bool priorEnvSetting = DISKCACHE.setKeepEnvHandleOpen(true);
/***/

#define REVERT_HANDLES_OPEN                                                                        \
  DISKCACHE.setKeepDbHandlesOpen(priorDbSetting);                                                  \
  DISKCACHE.setKeepEnvHandleOpen(priorEnvSetting);
/***/
namespace
{
tse::ConfigurableValue<std::string>
portCfg("SERVER_SOCKET_ADP", "PORT");
tse::ConfigurableValue<std::string>
configFile("CACHE_ADP", "CONFIG_FILE", "cacheNotify.xml");
tse::ConfigurableValue<uint32_t>
processingDelayCfg("CACHE_ADP", "PROCESSING_DELAY", 0);
}

namespace tse
{
void
getQueryElapsedStats(std::string& stats);

} // tse

namespace
{
typedef std::vector<std::string> STRVEC;

const std::string
CMD_STOP("DOWN");
const std::string
CMD_STATS("RFSH");
const std::string
CMD_SET_LOG_LEVEL("SLOG");
const std::string
CMD_SET_LOG_LEVEL_REGEX("SLRX");
const std::string
CMD_GET_LOG_LEVEL("GLOG");
const std::string
CMD_SET_ALL_LOG_LEVELS("SLAL");
const std::string
CMD_GET_ALL_LOGGERS("GLAL");
const std::string
CMD_COUNTER_RESET("CNRT");
const std::string
CMD_DETAILS("ETAI");
const std::string
CMD_REDEPLOY("PLOY");
const std::string
CMD_ACTIVATE("ACTI");
const std::string
CMD_CACHE_STATS("TATS");
const std::string
CMD_COMPR_CACHE_STATS("CCST");
const std::string
CMD_DISPOSE_CACHE("DSPC");
const std::string
CMD_TO_ELAPSED("LAPS");
const std::string
CMD_CACHE_FLUSH("FLSH");
const std::string
CMD_CACHE_UPDATES("UPDT");
const std::string
CMD_DAO_COVERAGE_STATS("DAOC");
const std::string
CMD_RELOAD_DATABASE_CONFIG("RLDB");
const std::string
CMD_ERROR_COUNTS("ERCT");
const std::string
CMD_MEMKEYS("MKEY");
const std::string
CMD_MEMKEYSANDVALUES("MKVL");
const std::string
CMD_DISKKEYS("DKEY");
const std::string
CMD_LDCTYPES("LDCT");
const std::string
CMD_LDC_COMPARE_VALUES("LDCC");
const std::string
CMD_INVALIDATE_KEY("INVK");
const std::string
CMD_INSERT_DUMMY_OBJECT("INSD");
const std::string
CMD_INJECT_CACHE_NOTIFY("INJC");
const std::string
CMD_OBJECT_EXISTS("EXIS");
const std::string
CMD_DATABASE_CONNECTIONS("DBCN");
const std::string
CMD_CACHE_MEMORY("MEMC");
const std::string
CMD_POOL_MEMORY("MEMP");
const std::string
CMD_CACHE_PARM("CPRM");
const std::string
CMD_QUERY_OBJECT("QOBJ");
const std::string
CMD_GET_NATION_LOADLIST("GNLL");
const std::string
CMD_LDC_COMP_DB_VALUES("LDCD");
const std::string
CMD_LDC_STATS("LATS");
const std::string
CMD_BDB_PANIC("BDBP");
const std::string
CMD_DYNAMIC_CFG("DCFG");
const std::string
CMD_SERVICES_LATENCY("SVCL");

const std::string
CMD_SERVER_CONFIG("CNFG");
const std::string
CMD_SERVER_CPU("SCPU");
const std::string
CMD_QUERY_ELAPSED("QELT");
const std::string
CMD_RC_HEALTH_CHECK("RCHC");
const std::string CMD_RC_DISABLE("RCDS");
const std::string CMD_RC_ENABLE("RCEN");
const std::string CMD_RC_PERSISTENT("RCPT");
const std::string CMD_RC_NONPERSISTENT("RCNP");
const std::string CMD_RC_GET_PARAMETERS("RCGP");
const std::string CMD_RC_GET_CACHES("RCGC");
const std::string CMD_RC_LDC_DELAY("RCLD");
const std::string CMD_RC_THREAD_POOL_SIZE("RCTP");
const std::string CMD_RC_MAX_NUMBER_CLIENTS("RCMC");
const std::string CMD_RC_HEALTHCHECK_TIMEOUT("RCHT");
const std::string CMD_RC_ASYNCHRONOUS_HEALTHCHECK("RCAH");
const std::string CMD_RC_ENABLE_HEALTHCHECK("RCEH");
const std::string CMD_RC_HEALTHCHECK_PERIOD("RCHP");
const std::string CMD_RC_QUEUE_TOLERANCE("RCQT");
const std::string CMD_RC_SLAVE_CONNECT_TO("RCSC");
const std::string CMD_RC_LINGER("RCLR");
const std::string CMD_RC_LINGER_TIME("RCLT");
const std::string CMD_RC_KEEP_ALIVE("RCKA");
const std::string CMD_RC_USE_CLIENT_SPECIFIED_TIMEOUT("RCST");
const std::string CMD_RC_CLIENT_PROCESSING_TIMEOUT("RCRT");
const std::string CMD_RC_CHECK_BASELINE("RCCB");
const std::string CMD_RC_IGNORE_DB_MISMATCH("RCID");
const std::string CMD_RC_CACHE_UPDATE_DETECTION_INTERVAL("CUDI");
const std::string CMD_RC_CLIENT_POOL_SAMPLING_INTERVAL("CPSI");
const std::string CMD_RC_CLIENT_POOL_ADJUST_INTERVAL("CPAI");
const std::string CMD_RC_MASTER_ALL_DATA_TYPES("RCMA");
const std::string CMD_RC_STATS_SAMPLING("RCSS");
const std::string CMD_RC_STATS_LOGGING_INTERVAL("RCLI");
const std::string CMD_RC_IDLE_MASTER_TIMEOUT("RCIM");
const std::string CMD_RC_IDLE_CLIENT_TIMEOUT("RCIC");
const std::string CMD_RC_SERVER_RECEIVE_TIMEOUT("RCSR");
const std::string CMD_RC_SERVER_SEND_TIMEOUT("RMST");
const std::string CMD_RC_CLIENT_SEND_TIMEOUT("CSTO");
const std::string CMD_RC_SET_MIN_CLIENTS("RSMC");
const std::string CMD_RC_DEBUG("RCDG");
const std::string CMD_RC_RESET_PARAMETERS("RSTP");

const std::string
VER("0001");
const std::string
REV("0000");

const char DELIM = '|';
const unsigned int haltDelay = 3;

struct LogCacheStats
    : public std::unary_function<std::pair<const std::string, tse::CacheControl*>&, void>
{
  std::ostream& _os;

  LogCacheStats(std::ostream& os) : _os(os) {}

  std::ostream& streamOne(const std::string& name, tse::CacheControl* ctl) const
  {
    // If we dont have any numbers yet just skip it
    if (ctl != nullptr)
    {
      _os << name << DELIM << ctl->cacheMax() << DELIM << ctl->cacheSize() << DELIM
          << ctl->accessCount() << DELIM << ctl->readCount() << DELIM;
    }
    return _os;
  }

  void operator()(std::pair<const std::string, tse::CacheControl*>& p) const
  {
    streamOne(p.first, p.second);
  }
}; // lint !e1509 !e1510

struct LogComprCacheStats
    : public std::unary_function<std::pair<const std::string, tse::CacheControl*>&, void>
{
  std::ostream& _os;
  LogComprCacheStats(std::ostream& os) : _os(os) {}
  std::ostream& streamOne(const std::string& name, tse::CacheControl* ctl) const
  {
    if (ctl != nullptr)
    {
      sfc::CompressedCacheStats stats;
      ctl->compressedCacheStats(stats);
      std::string errors(stats._errors);
      const size_t LIMIT(1024);
      if (errors.length() > LIMIT)
      {
        errors = errors.substr(0, LIMIT);
        errors.append("...");
      }
      _os << std::fixed;
      _os.precision(3);
      _os << name << DELIM << "#tot=" << stats._totalSize << DELIM
          << "totCap=" << stats._totalCapacity << DELIM << "#uncompr=" << stats._uncompressedSize
          << DELIM << "uncomprCap=" << stats._uncompressedCapacity << DELIM
          << "#compr=" << stats._compressedSize << DELIM << "#empty=" << stats._numberEmpty << DELIM
          << "mem~" << stats._memoryEstimate << DELIM
          << "avrgComprBytes=" << stats._averageCompressedBytes << DELIM
          << "avrgRatio=" << stats._averageRatio << DELIM << "thld=" << stats._threshold << DELIM
          << "#access=" << ctl->accessCount() << DELIM << "#read=" << ctl->readCount() << DELIM
          << "err:" << errors << DELIM;
    }
    return _os;
  }

  void operator()(std::pair<const std::string, tse::CacheControl*>& p) const
  {
    streamOne(p.first, p.second);
  }
};

struct LogUpdateStats
    : public std::unary_function<std::pair<const std::string, tse::CacheControl*>&, void>
{
  std::ostream& _os;
  LogUpdateStats(std::ostream& os) : _os(os) {}
  void operator()(std::pair<const std::string, tse::CacheControl*>& p) const
  {
    tse::CacheControl* ctl = p.second;
    if (ctl != nullptr)
    {
      tse::CacheStats* stats = ctl->cacheStats();
      if (stats != nullptr)
      {
        _os << p.first << DELIM << stats->updates << DELIM << stats->flushes << DELIM
            << stats->deletes << DELIM << stats->noneDeleted << DELIM << stats->cpuTime << DELIM
            << stats->rowsAdded << DELIM << stats->rowsRemoved << DELIM;
      }
    }
  }
};
}

struct LogDAOCoverageStats
    : public std::unary_function<std::pair<const std::string, tse::CacheControl*>&, void>
{
  std::ostream& _os;

  LogDAOCoverageStats(std::ostream& os) : _os(os) {}

  void operator()(std::pair<const std::string, tse::CacheControl*>& p) const
  {
    tse::CacheControl* ctl = p.second;

    // If we dont have any numbers yet just skip it
    if (ctl != nullptr)
    {
      _os << p.first << DELIM << ctl->loadCallCount() << DELIM << ctl->getCallCount() << DELIM
          << ctl->getAllCallCount() << DELIM << ctl->getFDCallCount() << DELIM;
    }
  }
}; // lint !e1509 !e1510

using namespace tse;
using namespace std;

bool
TseAppConsole::start(const long port, TseServer* srv, bool allowCacheTools)
{
  _srv = srv;
  _allowCacheTools = allowCacheTools;
  return AppConsole::start(port);
}

bool
TseAppConsole::checkAllowCacheTools(std::ostream& os)
{
  bool retval(_allowCacheTools);

  if (!retval)
  {
    os << "0" << std::endl; // Boolean return code
    os << "1" << std::endl; // Number of lines to follow
    os << "SORRY!  This server does not allow cache tools." << std::endl;
  }

  return retval;
}

bool
TseAppConsole::processMessage(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp)
{
  LOG4CXX_DEBUG(logger(),
                "Received [" << req.command << "] command with payload [" << req.payload << "]");

  // make sure its the right version/revision
  if ((VER != req.xmlVersion) || (REV != req.xmlRevision))
    return false;

  if (CMD_STATS == req.command)
  {
    return processStats(req, rsp);
  }
  else if (CMD_COUNTER_RESET == req.command)
  {
    return processResetCounters(req, rsp);
  }
  else if (CMD_GET_LOG_LEVEL == req.command)
  {
    return processGetLogLevel(req, rsp);
  }
  else if (CMD_SET_LOG_LEVEL == req.command)
  {
    return processSetLogLevel(req, rsp);
  }
  else if (CMD_SET_LOG_LEVEL_REGEX == req.command)
  {
    return processSetLogLevelRegEx(req, rsp);
  }
  else if (CMD_GET_ALL_LOGGERS == req.command)
  {
    return processGetAllLoggers(req, rsp);
  }
  else if (CMD_SET_ALL_LOG_LEVELS == req.command)
  {
    return processSetAllLogLevels(req, rsp);
  }
  else if (CMD_STOP == req.command)
  {
    return processStop(req, rsp);
  }
  else if (CMD_DETAILS == req.command)
  {
    return processDetails(req, rsp);
  }
  else if (CMD_REDEPLOY == req.command)
  {
    return processRedeploy(req, rsp);
  }
  else if (CMD_ACTIVATE == req.command)
  {
    return processActivate(req, rsp);
  }
  else if (CMD_CACHE_STATS == req.command)
  {
    return processCacheStats(req, rsp);
  }
  else if (CMD_COMPR_CACHE_STATS == req.command)
  {
    return processCompressedCacheStats(req, rsp);
  }
  else if (CMD_DISPOSE_CACHE == req.command)
  {
    return processDisposeCache(req, rsp);
  }
  else if (CMD_TO_ELAPSED == req.command)
  {
    return processTOElapsed(req, rsp);
  }
  else if (CMD_CACHE_FLUSH == req.command)
  {
    return processCacheFlush(req, rsp);
  }
  else if (CMD_CACHE_UPDATES == req.command)
  {
    return processCacheUpdates(req, rsp);
  }
  else if (CMD_DAO_COVERAGE_STATS == req.command)
  {
    return processDAOCoverageStats(req, rsp);
  }
  else if (CMD_RELOAD_DATABASE_CONFIG == req.command)
  {
    return processReloadDatabaseConfig(req, rsp);
  }
  else if (CMD_ERROR_COUNTS == req.command)
  {
    return processErrorCounts(req, rsp);
  }
  else if (CMD_MEMKEYS == req.command)
  {
    return processMemKeys(req, rsp, false);
  }
  else if (CMD_MEMKEYSANDVALUES == req.command)
  {
    return processMemKeys(req, rsp, true);
  }
  else if (CMD_DISKKEYS == req.command)
  {
    return processDiskKeys(req, rsp);
  }
  else if (CMD_LDCTYPES == req.command)
  {
    return processLDCTypes(req, rsp);
  }
  else if (CMD_LDC_COMPARE_VALUES == req.command)
  {
    return processCompareValues(req, rsp);
  }
  else if (CMD_LDC_COMP_DB_VALUES == req.command)
  {
    return processCompareDbValues(req, rsp);
  }
  else if (CMD_INVALIDATE_KEY == req.command)
  {
    return processInvalidateKey(req, rsp);
  }
  else if (CMD_INSERT_DUMMY_OBJECT == req.command)
  {
    return processInsertDummyObject(req, rsp);
  }
  else if (CMD_INJECT_CACHE_NOTIFY == req.command)
  {
    return processInjectCacheNotify(req, rsp);
  }
  else if (CMD_OBJECT_EXISTS == req.command)
  {
    return processObjectExists(req, rsp);
  }
  else if (CMD_CACHE_PARM == req.command)
  {
    return processCacheParm(req, rsp);
  }
  else if (CMD_DATABASE_CONNECTIONS == req.command)
  {
    return processDatabaseConnections(req, rsp);
  }
  else if (CMD_CACHE_MEMORY == req.command)
  {
    return processCacheMemory(req, rsp);
  }
  else if (CMD_POOL_MEMORY == req.command)
  {
    return processPoolMemory(req, rsp);
  }
  else if (CMD_QUERY_OBJECT == req.command)
  {
    return processQueryObject(req, rsp);
  }
  else if (CMD_GET_NATION_LOADLIST == req.command)
  {
    return processGetNationLoadlist(req, rsp);
  }
  else if (CMD_LDC_STATS == req.command)
  {
    return processLDCStats(req, rsp);
  }
  else if (CMD_BDB_PANIC == req.command)
  {
    return processBDBPanic(req, rsp);
  }
  else if (CMD_DYNAMIC_CFG == req.command)
  {
    return processDynamicCfgLoading(req, rsp);
  }
  else if (CMD_SERVICES_LATENCY == req.command)
  {
    return processServicesLatency(req, rsp);
  }
  else if (CMD_SERVER_CONFIG == req.command)
  {
    return processServerConfig(req, rsp);
  }
  else if (CMD_SERVER_CPU == req.command)
  {
    return processServerCPU(req, rsp);
  }
  else if (CMD_QUERY_ELAPSED == req.command)
  {
    return processQueryElapsed(req, rsp);
  }
  else if (CMD_RC_HEALTH_CHECK == req.command)
  {
    return processRCHealthCheck(req, rsp);
  }
  else if (CMD_RC_DISABLE == req.command)
  {
    return processRCDisable(req, rsp);
  }
  else if (CMD_RC_ENABLE == req.command)
  {
    return processRCEnable(req, rsp);
  }
  else if (CMD_RC_PERSISTENT == req.command)
  {
    return processRCPersistent(req, rsp);
  }
  else if (CMD_RC_NONPERSISTENT == req.command)
  {
    return processRCNonPersistent(req, rsp);
  }
  else if (CMD_RC_LDC_DELAY == req.command)
  {
    return processRCLdcDelay(req, rsp);
  }
  else if (CMD_RC_GET_PARAMETERS == req.command)
  {
    return processRCGetParameters(req, rsp);
  }
  else if (CMD_RC_GET_CACHES == req.command)
  {
    return processRCGetCaches(req, rsp);
  }
  else if (CMD_RC_THREAD_POOL_SIZE == req.command)
  {
    return processRCThreadPoolSize(req, rsp);
  }
  else if (CMD_RC_MAX_NUMBER_CLIENTS == req.command)
  {
    return processRCMaxNumberClients(req, rsp);
  }
  else if (CMD_RC_HEALTHCHECK_TIMEOUT == req.command)
  {
    return processRCHealthcheckTimeout(req, rsp);
  }
  else if (CMD_RC_ASYNCHRONOUS_HEALTHCHECK == req.command)
  {
    return processRCAsynchronousHealthcheck(req, rsp);
  }
  else if (CMD_RC_ENABLE_HEALTHCHECK == req.command)
  {
    return processRCEnableHealthcheck(req, rsp);
  }
  else if (CMD_RC_HEALTHCHECK_PERIOD == req.command)
  {
    return processRCHealthcheckPeriod(req, rsp);
  }
  else if (CMD_RC_QUEUE_TOLERANCE == req.command)
  {
    return processRCQueueTolerance(req, rsp);
  }
  else if (CMD_RC_SLAVE_CONNECT_TO == req.command)
  {
    return processRCClientConnectTimeout(req, rsp);
  }
  else if (CMD_RC_LINGER == req.command)
  {
    return processRCLinger(req, rsp);
  }
  else if (CMD_RC_LINGER_TIME == req.command)
  {
    return processRCLingerTime(req, rsp);
  }
  else if (CMD_RC_KEEP_ALIVE == req.command)
  {
    return processRCKeepAlive(req, rsp);
  }
  else if (CMD_RC_USE_CLIENT_SPECIFIED_TIMEOUT == req.command)
  {
    return processRCUseClientSpecifiedTimeout(req, rsp);
  }
  else if (CMD_RC_CLIENT_PROCESSING_TIMEOUT == req.command)
  {
    return processRCClientProcessingTimeout(req, rsp);
  }
  else if (CMD_RC_CHECK_BASELINE == req.command)
  {
    return processRCCheckBaseline(req, rsp);
  }
  else if (CMD_RC_IGNORE_DB_MISMATCH == req.command)
  {
    return processRCIgnoreDatabaseMismatch(req, rsp);
  }
  else if (CMD_RC_CACHE_UPDATE_DETECTION_INTERVAL == req.command)
  {
    return processRCCacheUpdateDetectionInterval(req, rsp);
  }
  else if (CMD_RC_CLIENT_POOL_SAMPLING_INTERVAL == req.command)
  {
    return processRCClientPoolSamplingInterval(req, rsp);
  }
  else if (CMD_RC_CLIENT_POOL_ADJUST_INTERVAL == req.command)
  {
    return processRCClientPoolAdjustInterval(req, rsp);
  }
  else if (CMD_RC_MASTER_ALL_DATA_TYPES == req.command)
  {
    return processRCMasterAllDatatypes(req, rsp);
  }
  else if (CMD_RC_STATS_SAMPLING == req.command)
  {
    return processRCStatsSamplingInterval(req, rsp);
  }
  else if (CMD_RC_STATS_LOGGING_INTERVAL == req.command)
  {
    return processRCStatsLoggingInterval(req, rsp);
  }
  else if (CMD_RC_IDLE_MASTER_TIMEOUT == req.command)
  {
    return processRCIdleMasterTimeout(req, rsp);
  }
  else if (CMD_RC_IDLE_CLIENT_TIMEOUT == req.command)
  {
    return processRCIdleSlaveTimeout(req, rsp);
  }
  else if (CMD_RC_SERVER_RECEIVE_TIMEOUT == req.command)
  {
    return processRCServerReceiveTimeout(req, rsp);
  }
  else if (CMD_RC_SERVER_SEND_TIMEOUT == req.command)
  {
    return processRCServerSendTimeout(req, rsp);
  }
  else if (CMD_RC_CLIENT_SEND_TIMEOUT == req.command)
  {
    return processRCClientSendTimeout(req, rsp);
  }
  else if (CMD_RC_SET_MIN_CLIENTS == req.command)
  {
    return processRCMinClientsRatio(req, rsp);
  }
  else if (CMD_RC_DEBUG == req.command)
  {
    return processRCDebug(req, rsp);
  }
  else if (CMD_RC_RESET_PARAMETERS == req.command)
  {
    return processRCResetParameters(req, rsp);
  }
  return false;
}

bool
TseAppConsole::processStop(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp)
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;

  if (shutdown("stop"))
    rsp.payload = "1";
  else
    rsp.payload = "0";

  return true;
}

bool
TseAppConsole::processRedeploy(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp)
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;

  const std::string& cmd = AppConsoleController::redeployFilename();

  // RC 0 is fail, 1 is success
  if (cmd.empty())
  {
    rsp.payload = "0";
    return true;
  }

  int status = ::system(cmd.c_str());
  if (((status >> 8) & 0xff) == 0)
    rsp.payload = "1";
  else
    rsp.payload = "0";

  return true;
}

bool
TseAppConsole::processActivate(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp)
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;

  // RC 0 is fail, 1 is success
  if (req.payload.empty())
  {
    rsp.payload = "0";
    return true;
  }

  if (!dirExists(req.payload))
  {
    rsp.payload = "0";
    return true;
  }

  if (shutdown("activate", &AppConsoleController::activateFilename(), &req.payload))
    rsp.payload = "1";
  else
    rsp.payload = "0";

  return true;
}

bool
TseAppConsole::processStats(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp)
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;

  ostringstream oss;
  TseSrvStats::dumpStats(oss);

  rsp.payload = oss.str();

  return true;
}

bool
TseAppConsole::processServicesLatency(const ac::SocketUtils::Message& req,
                                      ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;

  ostringstream oss;
  TseSrvStats::dumpServicesLatency(oss);

  rsp.payload = oss.str();

  return true;
}

bool
TseAppConsole::processGetLogLevel(const ac::SocketUtils::Message& req,
                                  ac::SocketUtils::Message& rsp)
{
  log4cxx::LoggerPtr logger = 0;

  if (req.payload.empty())
  {
    logger = log4cxx::Logger::getRootLogger();
  }
  else
  {
    logger = log4cxx::Logger::getLogger(req.payload);
  }

  if (logger == nullptr)
    return false;

  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;

  rsp.payload = logger->getEffectiveLevel()->toString();

  return true;
}

bool
TseAppConsole::processSetLogLevel(const ac::SocketUtils::Message& req,
                                  ac::SocketUtils::Message& rsp)
{
  if (req.payload.empty())
    return false;

  size_t i = req.payload.find(DELIM);
  if (i == string::npos) // lint !e530
    return false;

  std::string log = req.payload.substr(0, i);
  std::string level = req.payload.substr(i + 1);

  log4cxx::LoggerPtr logger = 0;

  if (log.empty())
    logger = log4cxx::Logger::getRootLogger();
  else
    logger = log4cxx::Logger::getLogger(log);

  log4cxx::LevelPtr levelPtr = log4cxx::Level::toLevel(level);
  logger->setLevel(levelPtr);

  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;

  rsp.payload = logger->getEffectiveLevel()->toString();

  return true;
}

bool
TseAppConsole::processSetLogLevelRegEx(const ac::SocketUtils::Message& req,
                                       ac::SocketUtils::Message& rsp)
{
  if (req.payload.empty())
    return false;

  size_t i = req.payload.rfind(DELIM);
  if (i == string::npos)
    return false;

  boost::regex regex(req.payload.substr(0, i));
  std::string level = req.payload.substr(i + 1);

  log4cxx::LoggerPtr logger = log4cxx::Logger::getRootLogger();
  log4cxx::LevelPtr levelPtr = log4cxx::Level::toLevel(level);

  log4cxx::LoggerList allLoggers = logger->getLoggerRepository()->getCurrentLoggers();

  for (auto& log : allLoggers)
  {
    boost::cmatch what;
    if (boost::regex_match(log->getName().c_str(), what, regex))
    {
      log->setLevel(levelPtr);
      rsp.payload += log->getName();
      rsp.payload += ": ";
      rsp.payload += log->getEffectiveLevel()->toString();
      rsp.payload += "\n";
    }
  }

  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;

  return true;
}

bool
TseAppConsole::processGetAllLoggers(const ac::SocketUtils::Message& req,
                                    ac::SocketUtils::Message& rsp)
{
  if (!req.payload.empty())
    return false;

  log4cxx::LoggerPtr logger = log4cxx::Logger::getRootLogger();

  log4cxx::LoggerList allLoggers = logger->getLoggerRepository()->getCurrentLoggers();

  for (auto& log : allLoggers)
  {
    rsp.payload += log->getName();
    rsp.payload += ": ";
    rsp.payload += log->getEffectiveLevel()->toString();
    rsp.payload += "\n";
  }

  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;

  return true;
}

bool
TseAppConsole::processSetAllLogLevels(const ac::SocketUtils::Message& req,
                                      ac::SocketUtils::Message& rsp)
{
  if (req.payload.empty())
    return false;

  log4cxx::LoggerPtr logger = log4cxx::Logger::getRootLogger();

  log4cxx::LoggerList allLoggers = logger->getLoggerRepository()->getCurrentLoggers();

  for (auto& log : allLoggers)
  {
    log->setLevel(log4cxx::Level::toLevel(req.payload));
    rsp.payload += log->getName();
    rsp.payload += ": ";
    rsp.payload += log->getEffectiveLevel()->toString();
    rsp.payload += "\n";
  }

  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;

  return true;
}

bool
TseAppConsole::processResetCounters(const ac::SocketUtils::Message& req,
                                    ac::SocketUtils::Message& rsp)
{
  TseSrvStats::reset();

  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  rsp.payload = "1";

  return true;
}

bool
TseAppConsole::processReloadDatabaseConfig(const ac::SocketUtils::Message& req,
                                           ac::SocketUtils::Message& rsp)
{
  DataManager& dm = DataManager::instance();

  dm.loadConfigValues();

  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  rsp.payload = "1";

  return true;
}

bool
TseAppConsole::processDetails(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp)
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;

  std::string port = portCfg.getValue();
  ostringstream oss;
  oss << AppConsoleController::process() << DELIM << AppConsoleController::user() << DELIM
      << AppConsoleController::build() << DELIM << port;

  rsp.payload = oss.str();

  return true;
}

bool
TseAppConsole::processCacheStats(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp)
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;

  const std::string& optionalCacheId = req.payload;

  ostringstream oss;

  tse::CacheRegistry& registry = tse::CacheRegistry::instance();
  LogCacheStats stats(oss);

  if (optionalCacheId.empty())
  {
    registry.forEach(stats);
  }
  else
  {
    CacheControl* ctrl = registry.getCacheControl(optionalCacheId);
    stats.streamOne(optionalCacheId, ctrl);
  }

  rsp.payload = oss.str();

  return true;
}

bool
TseAppConsole::processCompressedCacheStats(const ac::SocketUtils::Message& req,
                                           ac::SocketUtils::Message& rsp)
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  const std::string& optionalCacheId(req.payload);
  ostringstream oss;
  tse::CacheRegistry& registry(tse::CacheRegistry::instance());
  LogComprCacheStats stats(oss);
  if (optionalCacheId.empty())
  {
    registry.forEach(stats);
  }
  else
  {
    CacheControl* ctrl(registry.getCacheControl(optionalCacheId));
    stats.streamOne(optionalCacheId, ctrl);
  }
  rsp.payload = oss.str();
  return true;
}

bool
TseAppConsole::processDisposeCache(const ac::SocketUtils::Message& req,
                                   ac::SocketUtils::Message& rsp)
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  typedef boost::tokenizer<boost::char_separator<char>> Tokens;
  boost::char_separator<char> sep("/", "", boost::keep_empty_tokens);
  Tokens tokens(req.payload, sep);
  int idx(0);
  std::string cacheId;
  double fraction(0);
  for (const auto& tok : tokens)
  {
    switch (idx)
    {
    case 0:
      cacheId = tok;
      break;
    case 1:
      fraction = std::atof(tok.c_str());
      break;
    }
    ++idx;
  }
  tse::CacheRegistry& registry(tse::CacheRegistry::instance());
  if (!cacheId.empty())
  {
    CacheControl* ctrl(registry.getCacheControl(cacheId));
    if (ctrl)
    {
      ctrl->disposeCache(fraction);
    }
  }
  return true;
}

bool
TseAppConsole::processErrorCounts(const ac::SocketUtils::Message& req,
                                  ac::SocketUtils::Message& rsp)
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;

  ostringstream oss;
  TseSrvStats::dumpErrorCounts(oss);

  rsp.payload = oss.str();

  return true;
}

namespace
{
long
numberProcessors()
{
  return sysconf(_SC_NPROCESSORS_ONLN);
}
}

bool
TseAppConsole::processServerCPU(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp)
    const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  double utimems(0);
  double stimems(0);
  struct rusage rusg = {};
  if (0 == getrusage(RUSAGE_SELF, &rusg))
  {
    utimems = rusg.ru_utime.tv_sec * 1000 + rusg.ru_utime.tv_usec / 1000;
    stimems = rusg.ru_stime.tv_sec * 1000 + rusg.ru_stime.tv_usec / 1000;
  }
  static const long nprocessors(numberProcessors());
  std::ostringstream oss;
  oss << utimems << '|' << stimems << '|' << nprocessors;
  oss.str().swap(rsp.payload);
  return true;
}

bool
TseAppConsole::processServerConfig(const ac::SocketUtils::Message& req,
                                   ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::vector<tse::ConfigMan::NameValue> nameValuePairs;
  Global::configPtr dynamicCfg(Global::dynamicCfg());
  if (dynamicCfg)
  {
    dynamicCfg->getValues(nameValuePairs);
  }
  else if (Global::hasConfig())
  {
    Global::config().getValues(nameValuePairs);
  }
  std::ostringstream oss;
  std::string prvGroup;
  for (const auto& configItem : nameValuePairs)
  {
    if (prvGroup != configItem.group)
    {
      oss << "\n[" << configItem.group << "]\n";
      prvGroup = configItem.group;
    }
    oss << configItem.name << '=' << configItem.value << '\n';
  }
  oss.str().swap(rsp.payload);
  return true;
}

bool
TseAppConsole::processQueryElapsed(const ac::SocketUtils::Message& req,
                                   ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  tse::getQueryElapsedStats(rsp.payload);
  return true;
}

bool
TseAppConsole::processRCHealthCheck(const ac::SocketUtils::Message& req,
                                    ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  typedef boost::tokenizer<boost::char_separator<char>> Tokens;
  boost::char_separator<char> sep("/", "", boost::keep_empty_tokens);
  Tokens tokens(req.payload, sep);
  int idx(0);
  std::string masterHost;
  std::string masterPort;
  for (const auto& tok : tokens)
  {
    switch (idx)
    {
    case 0:
      masterHost = tok;
      break;
    case 1:
      masterPort = tok;
      break;
    }
    ++idx;
  }
  std::string msg;
  RemoteCache::ClientManager::healthcheck(masterHost, masterPort, msg);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCDisable(const ac::SocketUtils::Message& req,
                                     ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  RemoteCache::stop(msg);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCEnable(const ac::SocketUtils::Message& req,
                                    ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  typedef boost::tokenizer<boost::char_separator<char>> Tokens;
  boost::char_separator<char> sep("/", "", boost::keep_empty_tokens);
  Tokens tokens(req.payload, sep);
  int idx(0);
  std::string masterHost;
  std::string masterPort;
  std::string secondaryHost;
  std::string secondaryPort;
  for (const auto& tok : tokens)
  {
    switch (idx)
    {
    case 0:
      masterHost = tok;
      break;
    case 1:
      masterPort = tok;
      break;
    case 2:
      secondaryHost = tok;
      break;
    case 3:
      secondaryPort = tok;
      break;
    }
    ++idx;
  }
  std::string msg;
  RemoteCache::start(msg, masterHost, masterPort, secondaryHost, secondaryPort);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCPersistent(const ac::SocketUtils::Message& req,
                                        ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  RemoteCache::setPersistent(msg, true);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCNonPersistent(const ac::SocketUtils::Message& req,
                                           ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  RemoteCache::setPersistent(msg, false);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCLdcDelay(const ac::SocketUtils::Message& req,
                                      ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  int delay(std::atoi(req.payload.c_str()));
  std::string msg;
  RemoteCache::setLdcDelay(msg, delay);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCGetParameters(const ac::SocketUtils::Message& req,
                                           ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  RemoteCache::getParameters(msg);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCGetCaches(const ac::SocketUtils::Message& req,
                                       ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  RemoteCache::getCaches(msg);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCThreadPoolSize(const ac::SocketUtils::Message& req,
                                            ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  int sz(std::atoi(req.payload.c_str()));
  RemoteCache::setThreadPoolSize(msg, sz);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCMaxNumberClients(const ac::SocketUtils::Message& req,
                                              ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  int sz(std::atoi(req.payload.c_str()));
  RemoteCache::setMaxNumberClients(msg, sz);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCHealthcheckTimeout(const ac::SocketUtils::Message& req,
                                                ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  int to(std::atoi(req.payload.c_str()));
  RemoteCache::setHealthcheckTimeout(msg, to);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCAsynchronousHealthcheck(const ac::SocketUtils::Message& req,
                                                     ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  std::string payloadUpper(req.payload);
  boost::to_upper(payloadUpper);
  boost::trim(payloadUpper);
  bool parm("Y" == payloadUpper || "T" == payloadUpper || "1" == payloadUpper);
  RemoteCache::setAsynchronousHealthcheck(msg, parm);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCEnableHealthcheck(const ac::SocketUtils::Message& req,
                                               ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  std::string payloadUpper(req.payload);
  boost::to_upper(payloadUpper);
  boost::trim(payloadUpper);
  bool parm("Y" == payloadUpper || "T" == payloadUpper || "1" == payloadUpper);
  RemoteCache::setEnableHealthcheck(msg, parm);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCHealthcheckPeriod(const ac::SocketUtils::Message& req,
                                  ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  int period(std::atoi(req.payload.c_str()));
  RemoteCache::setHealthcheckPeriod(msg, period);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCQueueTolerance(const ac::SocketUtils::Message& req,
                                            ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  double tolerance(std::atof(req.payload.c_str()));
  RemoteCache::setQueueTolerance(msg, tolerance);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCClientConnectTimeout(const ac::SocketUtils::Message& req,
                                                  ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  int timeout(std::atoi(req.payload.c_str()));
  RemoteCache::setClientConnectionTimeout(msg, timeout);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCLinger(const ac::SocketUtils::Message& req,
                                    ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  std::string payloadUpper(req.payload);
  boost::to_upper(payloadUpper);
  boost::trim(payloadUpper);
  bool parm("Y" == payloadUpper || "T" == payloadUpper || "1" == payloadUpper);
  RemoteCache::setLinger(msg, parm);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCLingerTime(const ac::SocketUtils::Message& req,
                                        ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  int time(std::atoi(req.payload.c_str()));
  RemoteCache::setLingerTime(msg, time);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCKeepAlive(const ac::SocketUtils::Message& req,
                                       ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  std::string payloadUpper(req.payload);
  boost::to_upper(payloadUpper);
  boost::trim(payloadUpper);
  bool parm("Y" == payloadUpper || "T" == payloadUpper || "1" == payloadUpper);
  RemoteCache::setKeepAlive(msg, parm);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCUseClientSpecifiedTimeout(const ac::SocketUtils::Message& req,
                                                       ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  std::string payloadUpper(req.payload);
  boost::to_upper(payloadUpper);
  boost::trim(payloadUpper);
  bool parm("Y" == payloadUpper || "T" == payloadUpper || "1" == payloadUpper);
  RemoteCache::setUseClientSpecifiedTimeout(msg, parm);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCClientProcessingTimeout(const ac::SocketUtils::Message& req,
                                                     ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  int to(std::atoi(req.payload.c_str()));
  RemoteCache::setClientProcessingTimeout(msg, to);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCCheckBaseline(const ac::SocketUtils::Message& req,
                                           ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  std::string payloadUpper(req.payload);
  boost::to_upper(payloadUpper);
  boost::trim(payloadUpper);
  bool parm("Y" == payloadUpper || "T" == payloadUpper || "1" == payloadUpper);
  RemoteCache::setCheckBaseline(msg, parm);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCIgnoreDatabaseMismatch(const ac::SocketUtils::Message& req,
                                                    ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  std::string payloadUpper(req.payload);
  boost::to_upper(payloadUpper);
  boost::trim(payloadUpper);
  bool parm("Y" == payloadUpper || "T" == payloadUpper || "1" == payloadUpper);
  RemoteCache::setIgnoreDatabaseMismatch(msg, parm);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCCacheUpdateDetectionInterval(const ac::SocketUtils::Message& req,
                                                          ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  int interval(std::atoi(req.payload.c_str()));
  RemoteCache::setCacheUpdateDetectionInterval(msg, interval);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCClientPoolSamplingInterval(const ac::SocketUtils::Message& req,
                                                        ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  int interval(std::atoi(req.payload.c_str()));
  RemoteCache::setClientPoolSamplingInterval(msg, interval);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCClientPoolAdjustInterval(const ac::SocketUtils::Message& req,
                                                      ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  int interval(std::atoi(req.payload.c_str()));
  RemoteCache::setClientPoolAdjustInterval(msg, interval);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCDebug(const ac::SocketUtils::Message& req,
                                   ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  std::string payloadUpper(req.payload);
  boost::to_upper(payloadUpper);
  boost::trim(payloadUpper);
  bool parm("Y" == payloadUpper || "T" == payloadUpper || "1" == payloadUpper);
  RemoteCache::setDebug(msg, parm);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCMasterAllDatatypes(const ac::SocketUtils::Message& req,
                                                ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  std::string payloadUpper(req.payload);
  boost::to_upper(payloadUpper);
  boost::trim(payloadUpper);
  bool parm("Y" == payloadUpper || "T" == payloadUpper || "1" == payloadUpper);
  RemoteCache::setMasterAllDatatypes(msg, parm);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCStatsSamplingInterval(const ac::SocketUtils::Message& req,
                                                   ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  int interval(std::atoi(req.payload.c_str()));
  RemoteCache::setStatsSamplingInterval(msg, interval);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCStatsLoggingInterval(const ac::SocketUtils::Message& req,
                                                  ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  int interval(std::atoi(req.payload.c_str()));
  RemoteCache::setStatsLoggingInterval(msg, interval);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCIdleMasterTimeout(const ac::SocketUtils::Message& req,
                                               ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  int to(std::atoi(req.payload.c_str()));
  RemoteCache::setIdleMasterTimeout(msg, to);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCIdleSlaveTimeout(const ac::SocketUtils::Message& req,
                                              ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  int to(std::atoi(req.payload.c_str()));
  RemoteCache::setIdleSlaveTimeout(msg, to);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCServerReceiveTimeout(const ac::SocketUtils::Message& req,
                                                  ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  int to(std::atoi(req.payload.c_str()));
  RemoteCache::setServerReceiveTimeout(msg, to);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCServerSendTimeout(const ac::SocketUtils::Message& req,
                                               ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  int to(std::atoi(req.payload.c_str()));
  RemoteCache::setServerSendTimeout(msg, to);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCClientSendTimeout(const ac::SocketUtils::Message& req,
                                               ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  int to(std::atoi(req.payload.c_str()));
  RemoteCache::setClientSendTimeout(msg, to);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCResetParameters(const ac::SocketUtils::Message& req,
                                             ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  RemoteCache::resetParameters(msg);
  msg.swap(rsp.payload);
  return true;
}

bool TseAppConsole::processRCMinClientsRatio(const ac::SocketUtils::Message& req,
                                             ac::SocketUtils::Message& rsp) const
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;
  std::string msg;
  double ratio(std::atof(req.payload.c_str()));
  RemoteCache::setMinClientsRatio(msg, ratio);
  msg.swap(rsp.payload);
  return true;
}

bool
TseAppConsole::processCacheUpdates(const ac::SocketUtils::Message& req,
                                   ac::SocketUtils::Message& rsp)
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;

  ostringstream oss;

  tse::CacheRegistry& registry = tse::CacheRegistry::instance();
  LogUpdateStats stats(oss);
  registry.forEach(stats);

  rsp.payload = oss.str();

  return true;
}

bool
TseAppConsole::processDAOCoverageStats(const ac::SocketUtils::Message& req,
                                       ac::SocketUtils::Message& rsp)
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;

  ostringstream oss;

  tse::CacheRegistry& registry = tse::CacheRegistry::instance();
  LogDAOCoverageStats stats(oss);
  registry.forEach(stats);

  rsp.payload = oss.str();

  return true;
}

bool
TseAppConsole::processDatabaseConnections(const ac::SocketUtils::Message& req,
                                          ac::SocketUtils::Message& rsp)
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;

  ostringstream oss;
  DataManager::dumpDbConnections(oss);
  rsp.payload = oss.str();

  return true;
}

bool
TseAppConsole::processCacheMemory(const ac::SocketUtils::Message& req,
                                  ac::SocketUtils::Message& rsp)
{
  // Note: dumpCacheMemory does not generate a response unless DataManager logging is at least INFO
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;

  ostringstream oss;
  DataManager::dumpCacheMemory(oss, req.payload);
  rsp.payload = oss.str();

  return true;
}

bool
TseAppConsole::processPoolMemory(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp)
{
  // Note: dumpPoolMemory does not generate a response unless DataManager logging is at least INFO
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;

  ostringstream oss;
  DataManager::dumpPoolMemory(oss, req.payload);
  rsp.payload = oss.str();

  return true;
}

bool
TseAppConsole::processTOElapsed(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp)
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;

  ostringstream oss;
  TseSrvStats::dumpElapsed(oss);

  rsp.payload = oss.str();

  return true;
}

bool
TseAppConsole::processCacheFlush(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp)
{
  rsp.command = req.command;
  rsp.xmlVersion = VER;
  rsp.xmlRevision = REV;

  std::string cacheId = req.payload;
  tse::CacheRegistry& registry = tse::CacheRegistry::instance();
  tse::CacheControl* cacheControl = registry.getCacheControl(cacheId);
  if (cacheControl == nullptr)
  {
    rsp.payload = "0";
  }
  else
  {
    cacheControl->clear();
    rsp.payload = "1";
  }

  return true;
}

bool
TseAppConsole::processMemKeys(const ac::SocketUtils::Message& req,
                              ac::SocketUtils::Message& rsp,
                              bool inclValues)
{
  ostringstream oss;
  if (checkAllowCacheTools(oss))
  {
    rsp.command = req.command;
    rsp.xmlVersion = VER;
    rsp.xmlRevision = REV;

    const std::string& cacheId = req.payload;

    if (!cacheId.empty())
    {
      tse::CacheRegistry& registry = tse::CacheRegistry::instance();
      CacheControl* ctrl = registry.getCacheControl(cacheId);

      if (ctrl != nullptr)
      {
        oss << "1" << std::endl;

        std::set<std::string> list;

        ctrl->getAllFlatKeys(list, inclValues);

        std::ostringstream ostemp;
        size_t lineCounter(0);
        boost::char_separator<char> lineSep("\n", "", boost::keep_empty_tokens);
        for (const auto& entry : list)
        {
          if (entry.empty())
          {
            ostemp << std::endl;
            ++lineCounter;
          }
          else
          {
            boost::tokenizer<boost::char_separator<char>> lines(entry, lineSep);
            for (boost::tokenizer<boost::char_separator<char>>::const_iterator lne = lines.begin();
                 lne != lines.end();
                 ++lne)
            {
              ostemp << (*lne) << std::endl;
              ++lineCounter;
            }
          }
        }

        oss << lineCounter << std::endl;
        oss << ostemp.str();
      }
      else
      {
        oss << "0" << std::endl;
      }
    }
    else
    {
      oss << "0" << std::endl;
    }
  }

  rsp.payload = oss.str();
  return true;
}

bool
TseAppConsole::processDiskKeys(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp)
{
  ostringstream oss;
  if (checkAllowCacheTools(oss))
  {
    rsp.command = req.command;
    rsp.xmlVersion = VER;
    rsp.xmlRevision = REV;

    std::string cacheId;
    bool withDates(false);

    size_t i = req.payload.find('/');
    if (i == std::string::npos)
    {
      cacheId = req.payload;
    }
    else
    {
      cacheId = req.payload.substr(0, i);
      i += 1;
      if (i < req.payload.size())
      {
        std::string modifier(req.payload.substr(i));
        if (modifier == "WITHDATES")
        {
          withDates = true;
        }
      }
    }

    if (!cacheId.empty())
    {
      DiskCache::STRINGSET list;

      FORCE_HANDLES_OPEN;
      DISKCACHE.getKeys(cacheId, list, withDates);
      REVERT_HANDLES_OPEN;

      oss << "1" << std::endl;
      oss << list.size() << std::endl;

      for (const auto& flatKey : list)
      {
        oss << flatKey << std::endl;
      }
    }
    else
    {
      oss << "0" << std::endl;
    }
  }

  rsp.payload = oss.str();
  return true;
}

struct TseAppConsole::cacheControlList
    : public std::unary_function<std::pair<const std::string, CacheControl*>&, void>
{
  STRVEC& _vec;
  cacheControlList(STRVEC& vec) : _vec(vec) {}
  void operator()(std::pair<const std::string, CacheControl*>& p) const
  {
    const char* id = p.first.c_str();
    CacheControl& ctl = *(p.second);
    std::stringstream line;
    line << id << "," << ctl.getCacheType() << ",LDC=" << (ctl.ldcEnabled() ? "ON" : "OFF") << ","
         << ctl.cacheSize();
    _vec.push_back(line.str());
  }
};

bool
TseAppConsole::processLDCTypes(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp)
{
  ostringstream oss;
  if (checkAllowCacheTools(oss))
  {
    rsp.command = req.command;
    rsp.xmlVersion = VER;
    rsp.xmlRevision = REV;

    STRVEC lines;
    tse::CacheRegistry& registry = tse::CacheRegistry::instance();
    cacheControlList l(lines);
    registry.forEach(l);

    oss << "1" << std::endl;
    oss << lines.size() << std::endl;

    for (STRVEC::const_iterator cit = lines.begin(); cit != lines.end(); ++cit)
    {
      const std::string& line = (*cit);
      oss << line << std::endl;
    }
  }

  rsp.payload = oss.str();
  return true;
}

bool
TseAppConsole::processCompareValues(const ac::SocketUtils::Message& req,
                                    ac::SocketUtils::Message& rsp)
{
  ostringstream oss;
  if (checkAllowCacheTools(oss))
  {
    rsp.command = req.command;
    rsp.xmlVersion = VER;
    rsp.xmlRevision = REV;

    const std::string& cacheId(req.payload);
    tse::CacheRegistry& registry = tse::CacheRegistry::instance();
    CacheControl* ctrl = registry.getCacheControl(cacheId);
    if (ctrl != nullptr)
    {
      STRVEC results;
      uint32_t numCompared = 0;

      FORCE_HANDLES_OPEN;
      if (ctrl->compareMemCacheToLDC(results, numCompared))
      {
        oss << "1" << std::endl;
      }
      else
      {
        oss << "0" << std::endl;
      }
      REVERT_HANDLES_OPEN;

      boost::char_separator<char> lineSep("\n", "", boost::keep_empty_tokens);
      std::ostringstream ostemp;
      size_t lineCounter(0);

      for (STRVEC::const_iterator cit = results.begin(); cit != results.end(); ++cit)
      {
        const std::string& entry(*cit);
        boost::tokenizer<boost::char_separator<char>> lines(entry, lineSep);
        for (boost::tokenizer<boost::char_separator<char>>::const_iterator lne = lines.begin();
             lne != lines.end();
             ++lne)
        {
          ostemp << (*lne) << std::endl;
          ++lineCounter;
        }
      }

      oss << (lineCounter + 1) << std::endl;
      oss << numCompared << " total number of objects were compared." << std::endl;
      oss << ostemp.str();
    }
    else
    {
      oss << "0" << std::endl;
    }

    LOG4CXX_DEBUG(logger(), oss.str());
  }

  rsp.payload = oss.str();
  return true;
}

bool
TseAppConsole::processCompareDbValues(const ac::SocketUtils::Message& req,
                                      ac::SocketUtils::Message& rsp)
{
  ostringstream oss;
  if (checkAllowCacheTools(oss))
  {
    rsp.command = req.command;
    rsp.xmlVersion = VER;
    rsp.xmlRevision = REV;

    const std::string& cacheId(req.payload);
    tse::CacheRegistry& registry = tse::CacheRegistry::instance();
    CacheControl* ctrl = registry.getCacheControl(cacheId);
    if (ctrl != nullptr)
    {
      STRVEC results;
      uint32_t numCompared = 0;
      if (ctrl->compareMemCacheToDB(results, numCompared))
      {
        oss << "1" << std::endl;
      }
      else
      {
        oss << "0" << std::endl;
      }

      boost::char_separator<char> lineSep("\n", "", boost::keep_empty_tokens);
      std::ostringstream ostemp;
      size_t lineCounter(0);

      for (STRVEC::const_iterator cit = results.begin(); cit != results.end(); ++cit)
      {
        const std::string& entry(*cit);
        boost::tokenizer<boost::char_separator<char>> lines(entry, lineSep);
        for (boost::tokenizer<boost::char_separator<char>>::const_iterator lne = lines.begin();
             lne != lines.end();
             ++lne)
        {
          ostemp << (*lne) << std::endl;
          ++lineCounter;
        }
      }

      oss << (lineCounter + 1) << std::endl;
      oss << numCompared << " total number of objects were compared." << std::endl;
      oss << ostemp.str();
    }
    else
    {
      oss << "0" << std::endl;
    }

    LOG4CXX_DEBUG(logger(), oss.str());
  }

  rsp.payload = oss.str();
  return true;
}

bool
TseAppConsole::processInvalidateKey(const ac::SocketUtils::Message& req,
                                    ac::SocketUtils::Message& rsp)
{
  ostringstream oss;
  bool retval = false;
  if ((retval = checkAllowCacheTools(oss)))
  {
    std::string err;

    std::string cacheId("UNDEFINED");
    std::string cacheKey("UNDEFINED");

    rsp.command = req.command;
    rsp.xmlVersion = VER;
    rsp.xmlRevision = REV;

    size_t i = req.payload.find('/');
    if (i != std::string::npos)
    {
      cacheId = req.payload.substr(0, i);
      if (!cacheId.empty())
      {
        cacheKey = req.payload.substr(i + 1);
        if (!cacheKey.empty())
        {
          i = cacheKey.find('/');
          if (i != std::string::npos)
          {
            cacheKey = cacheKey.substr(0, i);
          }
          tse::CacheRegistry& registry = tse::CacheRegistry::instance();
          tse::CacheControl* ctrl = registry.getCacheControl(cacheId);
          if (ctrl != nullptr)
          {
            LOG4CXX_DEBUG(logger(), "Found cache control");

            uint64_t cacheSize = ctrl->cacheSize();
            CacheStats* stats = ctrl->cacheStats();

            CacheStats safetyStats;
            if (stats == nullptr)
            {
              stats = &safetyStats;
            }

            ++(stats->updates);

            LOG4CXX_DEBUG(logger(), "Invalidating for key[" << cacheKey << "]");

            FORCE_HANDLES_OPEN;
            ctrl->invalidate(cacheKey);
            REVERT_HANDLES_OPEN;

            int64_t deleteCount = cacheSize - ctrl->cacheSize();
            if (deleteCount >= 0)
            {
              stats->deletes += deleteCount;
              if (deleteCount == 0)
              {
                ++(stats->noneDeleted);
              }
              else
              {
                retval = true;
              }
            }
          }
          else
          {
            err = "Cache control not found";
          }
        }
        else
        {
          err = "No cache key specified";
        }
      }
      else
      {
        err = "No cache name specified";
      }
    }
    else
    {
      err = "Invalid input";
    }

    if (retval)
    {
      oss << "1" << std::endl; // Boolean return code
      oss << "1" << std::endl; // Number of lines to follow
      oss << "Specified cache key [" << cacheKey << "] was successfully invalidated for type ["
          << cacheId << "]." << std::endl;
    }
    else
    {
      oss << "0" << std::endl; // Boolean return code
      oss << "1" << std::endl; // Number of lines to follow
      oss << err << std::endl;
    }

    LOG4CXX_DEBUG(logger(), oss.str());
  }

  rsp.payload = oss.str();
  return retval;
}

STRVEC*
getNotificationKeyFields(ObjectKey& objectKey,
                         const std::map<std::string, STRVEC>& keyFields,
                         const std::map<std::string, STRVEC>& cacheIds)
{
  std::map<std::string, STRVEC>::const_iterator i;
  for (i = cacheIds.begin(); i != cacheIds.end(); ++i)
  {
    const std::string& notificationId((*i).first);
    const STRVEC& caches((*i).second);
    for (const auto& thisCacheId : caches)
    {
      if (strcasecmp(thisCacheId.c_str(), objectKey.tableName().c_str()) == 0)
      {
        std::map<std::string, STRVEC>::const_iterator k;
        k = keyFields.find(notificationId);
        if (k != keyFields.end())
        {
          const STRVEC& keys((*k).second);
          return const_cast<STRVEC*>(&keys);
        }
      }
    }
  }

  return nullptr;
}

STRVEC*
getNotifyKeys(ObjectKey& objectKey, TseServer* srv, log4cxx::LoggerPtr logger)
{
  STRVEC* retval(nullptr);

  static bool mapsInitialized(false);
  static std::map<std::string, STRVEC> keyFields;
  static std::map<std::string, STRVEC> cacheIds;
  static std::map<std::string, STRVEC> historicalKeys;
  static std::map<std::string, STRVEC> historicalIds;

  if (srv != nullptr)
  {
    if (!mapsInitialized)
    {
      mapsInitialized = true;
      std::string fileName = configFile.getValue();
      CacheNotifyControl cacheControl(fileName, keyFields, cacheIds, historicalKeys, historicalIds);
      cacheControl.parse();
    }

    retval = getNotificationKeyFields(objectKey, keyFields, cacheIds);
    if (retval == nullptr)
    {
      retval = getNotificationKeyFields(objectKey, historicalKeys, historicalIds);
      if (retval == nullptr)
      {
        LOG4CXX_DEBUG(logger,
                      "No cache notifications associated with cache [" << objectKey.tableName()
                                                                       << "].");
      }
    }
  }
  else
  {
    LOG4CXX_WARN(logger, "TseServer not accessible!");
  }

  return retval;
}

bool
TseAppConsole::processInsertDummyObject(const ac::SocketUtils::Message& req,
                                        ac::SocketUtils::Message& rsp)
{
  ostringstream oss;
  bool retval(false);
  if ((retval = checkAllowCacheTools(oss)))
  {
    std::string err;
    STRVEC* notifyKeys(nullptr);

    std::string cacheId(req.payload);
    std::string flatKey;
    std::string notifyKey;

    rsp.command = req.command;
    rsp.xmlVersion = VER;
    rsp.xmlRevision = REV;

    if (!cacheId.empty())
    {
      tse::CacheRegistry& registry = tse::CacheRegistry::instance();
      tse::CacheControl* ctrl = registry.getCacheControl(cacheId);
      if (ctrl != nullptr)
      {
        LOG4CXX_DEBUG(logger(), "Found cache control");
        ObjectKey objectKey;
        objectKey.tableName() = cacheId;
        retval = ctrl->insertDummyObject(flatKey, objectKey);
        if (retval)
        {
          replaceCharWithChar(flatKey, ' ', '^');
          replaceCharWithChar(flatKey, '\0', '~');

          notifyKeys = getNotifyKeys(objectKey, _srv, logger());
          if (notifyKeys != nullptr)
          {
            LOG4CXX_DEBUG(logger(),
                          "Found notification key fields for cache [" << objectKey.tableName()
                                                                      << "]:");
            bool firstDone(false);
            for (auto& notifyKey : *notifyKeys)
            {
              std::string val("");
              ObjectKey::KeyFields::iterator i(objectKey.keyFields().find(notifyKey));
              if (i != objectKey.keyFields().end())
              {
                val = (*i).second;
                LOG4CXX_DEBUG(logger(), "   [" << notifyKey << "]  exists");
              }
              else
              {
                LOG4CXX_DEBUG(logger(), "   [" << notifyKey << "]  added");
              }
              if (firstDone)
              {
                notifyKey.append("|");
              }
              else
              {
                firstDone = true;
              }
              notifyKey.append(val);
            }

            replaceCharWithChar(notifyKey, ' ', '^');
            replaceCharWithChar(notifyKey, '\0', '~');
          }
        }
        else
        {
          err = "Dummy object creation unsupported for this cache";
        }
      }
      else
      {
        err = "Cache control not found";
      }
    }
    else
    {
      err = "No cache name specified";
    }

    if (retval)
    {
      oss << "1" << std::endl; // Boolean return code
      oss << "1" << std::endl; // Number of lines to follow
      oss << "CacheKey=[" << flatKey << "] NotifyKey=[" << (notifyKeys ? notifyKey : "N/A") << "]"
          << std::endl;
    }
    else
    {
      oss << "0" << std::endl; // Boolean return code
      oss << "1" << std::endl; // Number of lines to follow
      oss << "ERROR: " << err << std::endl;
    }

    LOG4CXX_DEBUG(logger(), oss.str());
  }

  rsp.payload = oss.str();
  return retval;
}

bool
TseAppConsole::processInjectCacheNotify(const ac::SocketUtils::Message& req,
                                        ac::SocketUtils::Message& rsp)
{
  ostringstream oss;
  bool retval(false);
  if ((retval = checkAllowCacheTools(oss)))
  {
    LOG4CXX_DEBUG(logger(), "Received 'INJC' command with payload [" << req.payload << "]");

    std::string err;

    std::string notificationType("UNDEFINED");
    std::string keyString("UNDEFINED");

    rsp.command = req.command;
    rsp.xmlVersion = VER;
    rsp.xmlRevision = REV;

    size_t i = req.payload.find('/');
    if (i != std::string::npos)
    {
      notificationType = req.payload.substr(0, i);
      if (!notificationType.empty())
      {
        keyString = req.payload.substr(i + 1);
        if (!keyString.empty())
        {
          i = keyString.find('/');
          if (i != std::string::npos)
          {
            keyString = keyString.substr(0, i);
          }
          if (_srv != nullptr)
          {
            std::string modKeyString(keyString);
            replaceCharWithChar(modKeyString, '^', ' ');
            replaceCharWithChar(modKeyString, '~', '\0');
            modKeyString.append("\n^");

            _srv->injectCacheNotification(
                notificationType, modKeyString, processingDelayCfg.getValue());
            retval = true;
          }
          else
          {
            err = "No TseServer object present";
          }
        }
        else
        {
          err = "No cache key specified";
        }
      }
      else
      {
        err = "No notification type specified";
      }
    }
    else
    {
      err = "Invalid input";
    }

    if (retval)
    {
      oss << "1" << std::endl; // Boolean return code
      oss << "1" << std::endl; // Number of lines to follow
      oss << "Specified key string [" << keyString << "] was successfully injected for type ["
          << notificationType << "]." << std::endl;
    }
    else
    {
      oss << "0" << std::endl; // Boolean return code
      oss << "1" << std::endl; // Number of lines to follow
      oss << "ERROR: Specified key string [" << keyString << "] was NOT injected for type ["
          << notificationType << "]: " << err << std::endl;
    }

    LOG4CXX_DEBUG(logger(), oss.str());
  }

  rsp.payload = oss.str();
  return retval;
}

bool
TseAppConsole::processObjectExists(const ac::SocketUtils::Message& req,
                                   ac::SocketUtils::Message& rsp)
{
  ostringstream oss;
  bool retval(false);
  if ((retval = checkAllowCacheTools(oss)))
  {
    bool foundInMem(false);
    bool foundInLDC(false);
    bool foundInDB(false);
    time_t ldcTimestamp(0);
    std::string err;

    std::string cacheId("UNDEFINED");
    std::string cacheKey("UNDEFINED");

    rsp.command = req.command;
    rsp.xmlVersion = VER;
    rsp.xmlRevision = REV;

    size_t i = req.payload.find('/');
    if (i != std::string::npos)
    {
      cacheId = req.payload.substr(0, i);
      if (!cacheId.empty())
      {
        cacheKey = req.payload.substr(i + 1);
        i = cacheKey.find('/');
        if (i != std::string::npos)
        {
          cacheKey = cacheKey.substr(0, i);
        }
        tse::CacheRegistry& registry = tse::CacheRegistry::instance();
        tse::CacheControl* ctrl = registry.getCacheControl(cacheId);
        if (ctrl != nullptr)
        {
          LOG4CXX_DEBUG(logger(), "Found cache control");
          retval = true;
          foundInMem = ctrl->objectExistsInMem(cacheKey);

          FORCE_HANDLES_OPEN;
          foundInLDC = ctrl->objectExistsInLDC(cacheKey, ldcTimestamp);
          REVERT_HANDLES_OPEN;

          foundInDB = ctrl->objectExistsInDB(cacheKey);
        }
        else
        {
          err = "Cache control not found";
        }
      }
      else
      {
        err = "No cache name specified";
      }
    }
    else
    {
      err = "Invalid input";
    }

    if (retval)
    {
      oss << "1" << std::endl; // Boolean return code
      oss << "3" << std::endl; // Number of lines to follow
      oss << "Memory   = " << (foundInMem ? "TRUE" : "FALSE") << std::endl;
      ;
      oss << "LDC      = " << (foundInLDC ? "TRUE" : "FALSE");
      if (foundInLDC)
      {
        tm ltime;
        localtime_r(&ldcTimestamp, &ltime);
        DateTime dt(ltime.tm_year + 1900,
                    ltime.tm_mon + 1,
                    ltime.tm_mday,
                    ltime.tm_hour,
                    ltime.tm_min,
                    ltime.tm_sec);
        oss << " {" << dt.dateToString(YYYYMMDD, "-") << "." << dt.timeToString(HHMMSS, ":") << "}";
      }
      oss << std::endl;
      oss << "Database = " << (foundInDB ? "TRUE" : "FALSE");
    }
    else
    {
      oss << "0" << std::endl; // Boolean return code
      oss << "1" << std::endl; // Number of lines to follow
      oss << err << std::endl;
    }

    LOG4CXX_DEBUG(logger(), oss.str());
  }
  rsp.payload = oss.str();
  return retval;
}

class CacheParmPrinter
{
public:
  ostringstream& _oss;
  int totalLines;

  CacheParmPrinter(ostringstream& oss) : _oss(oss), totalLines(0) {}

  void operator()(std::pair<const std::string, CacheControl*>& p)
  {
    const std::string& cacheName(p.first);
    totalLines += CacheManager::instance().printCacheParm(cacheName, _oss);
  }
};

bool
TseAppConsole::processCacheParm(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp)
{
  ostringstream oss;
  if (checkAllowCacheTools(oss))
  {
    std::string err;
    int parmLinesPrinted(0);

    std::string optionalCacheId(req.payload);

    rsp.command = req.command;
    rsp.xmlVersion = VER;
    rsp.xmlRevision = REV;

    ostringstream os;
    tse::CacheRegistry& registry = tse::CacheRegistry::instance();
    if (optionalCacheId.empty())
    {
      CacheParmPrinter printParm(os);
      registry.forEach(printParm);
      parmLinesPrinted = printParm.totalLines;
    }
    else
    {
      parmLinesPrinted = CacheManager::instance().printCacheParm(optionalCacheId, os);
    }

    oss << "1" << std::endl; // Boolean return code
    oss << parmLinesPrinted << std::endl; // Number of lines to follow
    oss << os.str();

    LOG4CXX_DEBUG(logger(), oss.str());
  }

  rsp.payload = oss.str();
  return true;
}

bool
TseAppConsole::processQueryObject(const ac::SocketUtils::Message& req,
                                  ac::SocketUtils::Message& rsp)
{
  ostringstream oss;
  bool retval(false);
  if ((retval = checkAllowCacheTools(oss)))
  {
    std::string result;
    std::string err;
    std::string command;
    std::string format;
    std::string cacheId;
    std::string cacheKey;

    rsp.command = req.command;
    rsp.xmlVersion = VER;
    rsp.xmlRevision = REV;

    size_t idx(0);
    boost::char_separator<char> fieldSep("/", "", boost::keep_empty_tokens);
    boost::tokenizer<boost::char_separator<char>> fields(req.payload, fieldSep);
    for (boost::tokenizer<boost::char_separator<char>>::const_iterator tok = fields.begin();
         tok != fields.end();
         ++tok)
    {
      switch (idx++)
      {
      case 0:
        command = (*tok);
        break;
      case 1:
        format = (*tok);
        break;
      case 2:
        cacheId = (*tok);
        break;
      case 3:
        cacheKey = (*tok);
        break;
      }
    }

    if ((command != "getmem") && (command != "getldc") && (command != "getdb"))
    {
      err = "Invalid input: command must be 'getmem', 'getldc', or 'getdb'";
    }
    else if ((format != "xml") && (format != "text") && (format != "flat"))
    {
      err = "Invalid input: format must be 'xml', 'text', or 'flat'";
    }
    else if (cacheId.empty())
    {
      err = "Invalid input: no cache specified";
    }
    else
    {
      tse::CacheRegistry& registry = tse::CacheRegistry::instance();
      tse::CacheControl* ctrl = registry.getCacheControl(cacheId);
      if (ctrl != nullptr)
      {
        LOG4CXX_DEBUG(logger(), "Found cache control");
        retval = true;

        if (command == "getmem")
        {
          if (format == "xml")
          {
            ctrl->getMemObjectAsXML(cacheKey, result);
          }
          else if (format == "text")
          {
            ctrl->getMemObjectAsText(cacheKey, result);
          }
          else
          {
            ctrl->getMemObjectAsFlat(cacheKey, result);
          }
        }
        else if (command == "getldc")
        {
          FORCE_HANDLES_OPEN;
          if (format == "xml")
          {
            ctrl->getLDCObjectAsXML(cacheKey, result);
          }
          else if (format == "text")
          {
            ctrl->getLDCObjectAsText(cacheKey, result);
          }
          else
          {
            ctrl->getLDCObjectAsFlat(cacheKey, result);
          }
          REVERT_HANDLES_OPEN;
        }
        else if (command == "getdb")
        {
          if (format == "xml")
          {
            ctrl->getDBObjectAsXML(cacheKey, result);
          }
          else if (format == "text")
          {
            ctrl->getDBObjectAsText(cacheKey, result);
          }
          else
          {
            ctrl->getDBObjectAsFlat(cacheKey, result);
          }
        }
      }
      else
      {
        err = "Cache control not found";
      }
    }

    if (retval)
    {
      typedef std::vector<std::string> STRINGVEC;
      STRINGVEC returnLines;
      boost::char_separator<char> lineSep("\n", "", boost::keep_empty_tokens);
      boost::tokenizer<boost::char_separator<char>> lines(result, lineSep);
      for (boost::tokenizer<boost::char_separator<char>>::const_iterator lne = lines.begin();
           lne != lines.end();
           ++lne)
      {
        returnLines.push_back(*lne);
      }

      oss << "1" << std::endl; // Boolean return code
      oss << returnLines.size() << std::endl; // Number of lines to follow
      for (STRVEC::const_iterator s1 = returnLines.begin(); s1 != returnLines.end(); ++s1)
      {
        oss << (*s1) << std::endl;
      }
      oss << std::endl;
    }
    else
    {
      oss << "0" << std::endl; // Boolean return code
      oss << "1" << std::endl; // Number of lines to follow
      oss << err << std::endl;
    }

    LOG4CXX_DEBUG(logger(), oss.str());
  }
  rsp.payload = oss.str();
  return retval;
}

bool
TseAppConsole::processGetNationLoadlist(const ac::SocketUtils::Message& req,
                                        ac::SocketUtils::Message& rsp)
{
  ostringstream oss;
  if (checkAllowCacheTools(oss))
  {
    rsp.command = req.command;
    rsp.xmlVersion = VER;
    rsp.xmlRevision = REV;

    const std::string& cacheId("NATION");

    tse::CacheRegistry& registry = tse::CacheRegistry::instance();
    CacheControl* ctrl = registry.getCacheControl(cacheId);

    if (ctrl != nullptr)
    {
      DataHandle dh;
      const std::vector<Nation*> allNations(dh.getAllNation(DateTime::localTime()));

      oss << "1" << std::endl;
      oss << allNations.size() << std::endl;

      for (const auto nation : allNations)
      {
        oss << nation->description() << " = " << nation->nation() << std::endl;
      }
    }
    else
    {
      oss << "0" << std::endl;
    }
  }

  rsp.payload = oss.str();
  return true;
}

bool
TseAppConsole::processLDCStats(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp)
{
  ostringstream oss;
  bool retval(false);
  if ((retval = checkAllowCacheTools(oss)))
  {
    rsp.command = req.command;
    rsp.xmlVersion = VER;
    rsp.xmlRevision = REV;

    if (req.payload.empty())
    {
      oss << "1" << std::endl; // Boolean return code
      oss << "18" << std::endl; // Number of lines to follow

      oss << "ACTIVATED               = " << (DiskCache::isActivated() ? "Yes" : "No") << std::endl;
      oss << "BDB_DISABLED            = " << (DISKCACHE.bdbDisabled() ? "Yes" : "No") << std::endl;
      oss << "BERKELEY_REMOVE_DB_ENV  = " << (DISKCACHE.shouldRemoveBerkeleyDBEnv() ? "Yes" : "No")
          << std::endl;
      oss << "DB_VERIFY_ON_OPEN       = " << (DISKCACHE.dbVerifyOnOpen() ? "Yes" : "No")
          << std::endl;
      oss << "DIRECTORY               = " << DISKCACHE.directory() << std::endl;
      oss << "ENABLE_PRESTART_HOOK    = " << (DISKCACHE.enablePrestartHook() ? "Yes" : "No")
          << std::endl;
      oss << "FULL_VERIFY_HOUR        = " << DISKCACHE.getFullVerifyHour() << std::endl;
      oss << "KEEP_DB_HANDLES_OPEN    = " << (DISKCACHE.keepDbHandlesOpen() ? "Yes" : "No")
          << std::endl;
      oss << "KEEP_ENV_HANDLE_OPEN    = " << (DISKCACHE.keepEnvHandleOpen() ? "Yes" : "No")
          << std::endl;
      oss << "LAST_UPDATE_TIME        = " << DISKCACHE.getLastUpdateTime() << std::endl;
      oss << "MAX_CATCHUP_MINUTES     = " << DISKCACHE.getMaxCatchupMinutes() << std::endl;
      oss << "OPEN_FILE_SENTINELS     = " << (DISKCACHE.openFileSentinels() ? "Yes" : "No")
          << std::endl;
      oss << "PRESTART_DB_VERIFY      = " << (DISKCACHE.prestartDbVerify() ? "Yes" : "No")
          << std::endl;
      oss << "PRESTART_RECOVER        = " << (DISKCACHE.prestartRecover() ? "Yes" : "No")
          << std::endl;
      oss << "SERIALIZE_ENV_ACCESS    = " << (DISKCACHE.serializeEnvAccess() ? "Yes" : "No")
          << std::endl;
      oss << "SIZE_ALL_QUEUES         = " << DISKCACHE.sizeAllQueues() << std::endl;
      oss << "THREAD_INTERVAL_SECONDS = " << DISKCACHE.getThreadSleepSecs() << std::endl;
    }
    else
    {
      DiskCache::CacheTypeOptions* cto(DISKCACHE.getCacheTypeOptions(req.payload));
      if (cto != nullptr)
      {
        oss << "1" << std::endl; // Boolean return code
        oss << "11" << std::endl; // Number of lines to follow

        oss << "NAME                 = " << cto->name << std::endl;
        oss << "ACTIVATED            = " << (cto->enabled ? "Yes" : "No") << std::endl;
        oss << "DATA_FORMAT          = " << DISKCACHE.getDataFormatAsString(req.payload)
            << std::endl;
        oss << "MAX_CATCHUP_MINUTES  = " << cto->maxCatchupMinutes << std::endl;
        oss << "COMPRESSION_LIMIT    = " << cto->compressionLimit << std::endl;

        tse::CacheRegistry& registry = tse::CacheRegistry::instance();
        CacheControl* ctrl = registry.getCacheControl(cto->name);
        if (ctrl != nullptr)
        {
          oss << "QUEUE_SIZE           = " << ctrl->actionQueueSize() << std::endl;
        }
        else
        {
          oss << "QUEUE_SIZE           = !!!CACHE_REGISTRY_ERROR!!!" << std::endl;
        }

        struct stat st;

        std::string dbFileName(DISKCACHE.directory());
        dbFileName.append("/");
        dbFileName.append(DISKCACHE.constructDBFileName(req.payload));
        bool dbFileExists(access(dbFileName.c_str(), F_OK) == 0);
        bool dbFileStatOK(stat(dbFileName.c_str(), &st) == 0);
        DateTime dte;
        if (dbFileExists)
        {
          if (dbFileStatOK)
          {
            struct tm* tminfo = localtime(&(st.st_mtime));
            dte = DateTime(tminfo->tm_year + 1900,
                           tminfo->tm_mon + 1,
                           tminfo->tm_mday,
                           tminfo->tm_hour,
                           tminfo->tm_min,
                           tminfo->tm_sec);
          }
        }

        oss << "BDB_FILE_NAME        = " << dbFileName << std::endl;
        oss << "BDB_FILE_EXISTS      = " << (dbFileExists ? "Yes" : "No") << std::endl;

        if (dbFileExists && dbFileStatOK)
        {
          oss << "BDB_FILE_LAST_UPDATE = " << dte << std::endl;
        }
        else if (dbFileExists)
        {
          oss << "BDB_FILE_LAST_UPDATE = !!!FSTAT_ERROR!!!" << std::endl;
        }
        else
        {
          oss << "BDB_FILE_LAST_UPDATE = N/A" << std::endl;
        }

        oss << "USE_DMC              = " << (cto->useDistCache ? "Yes" : "No") << std::endl;
        oss << "TTL                  = " << cto->ttl << std::endl;
      }
      else
      {
        oss << "0" << std::endl; // Boolean return code
        oss << "1" << std::endl; // Number of lines to follow
        oss << "Cache type [" << req.payload << "] not found" << std::endl;
      }
    }

    LOG4CXX_DEBUG(logger(), oss.str());
  }

  rsp.payload = oss.str();
  return retval;
}

bool
TseAppConsole::processBDBPanic(const ac::SocketUtils::Message& req, ac::SocketUtils::Message& rsp)
{
  ostringstream oss;
  bool retval(false);
  if ((retval = checkAllowCacheTools(oss)))
  {
    rsp.command = req.command;
    rsp.xmlVersion = VER;
    rsp.xmlRevision = REV;

    DISKCACHE.sendPanicEvent();

    oss << "1" << std::endl; // Boolean return code
    oss << "2" << std::endl; // Number of lines to follow
    oss << "DiskCache BDB panic initiated." << std::endl;
    oss << "LDC will remain inactive until the server is restarted." << std::endl;

    LOG4CXX_DEBUG(logger(), oss.str());
  }

  rsp.payload = oss.str();
  return retval;
}

bool
TseAppConsole::processDynamicCfgLoading(const ac::SocketUtils::Message& req,
                                        ac::SocketUtils::Message& rsp)
{
  LOG4CXX_INFO(logger(), "Received Dynamic Cfg Loading Request");

  string output;

  if (!DynamicConfigLoader::isActive())
  {
    output = "Dynamic configuration loading is disabled.\nConfiguration update failed.";
    LOG4CXX_ERROR(logger(), output);
  }
  else
    DynamicConfigLoader::loadNewConfig(output);

  rsp.payload = output;
  return true;
}

bool
TseAppConsole::shutdown(const std::string& cmd,
                        const std::string* filename,
                        const std::string* contents)
{
  LOG4CXX_WARN(logger(), "Shutting down by request!");

  if (filename != nullptr && !filename->empty())
  {
    std::ofstream outfile(filename->c_str());

    if (!outfile.good())
    {
      LOG4CXX_WARN(logger(), "Unable to write to filename '" << *filename << "'");
      outfile.close();
      return false;
    }

    if (contents != nullptr)
      outfile << *contents << endl;
    else
      outfile << " " << endl;

    outfile.close();
    LOG4CXX_WARN(logger(), "Wrote to filename '" << *filename << "'");
  }

  AppConsoleController::halt(0, true);
  return true;
}

bool
TseAppConsole::dirExists(const std::string& dir)
{
  if (dir.empty())
    return false;

  DIR* dirPtr = opendir(dir.c_str());
  if (dirPtr == nullptr)
    return false;

  closedir(dirPtr);
  return true;
}

Logger
TseAppConsole::logger()
{
  static Logger logger("atseintl.Server.TseAppConsole");
  return logger;
}
