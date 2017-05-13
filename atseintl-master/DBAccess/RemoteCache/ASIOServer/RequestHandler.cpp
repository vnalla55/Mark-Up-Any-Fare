#include "DBAccess/RemoteCache/ASIOServer/RequestHandler.h"

#include "Allocator/TrxMalloc.h"
#include "Common/Logger.h"
#include "Common/ErrorResponseException.h"
#include "Common/TSEException.h"
#include "Common/TseSrvStats.h"
#include "DBAccess/CacheControl.h"
#include "DBAccess/CacheRegistry.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/RemoteCache/CurrentDatabase.h"
#include "DBAccess/RemoteCache/ReadConfig.h"
#include "DBAccess/RemoteCache/RemoteCacheHeader.h"
#include "DBAccess/RemoteCache/RCStatistics.h"
#include "DBAccess/RemoteCache/ASIOServer/Reply.h"
#include "DBAccess/RemoteCache/ASIOServer/Request.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <string>

namespace tse
{

namespace RemoteCache
{

namespace RequestHandler
{

namespace
{

Logger logger("atseintl.DBAccess.RemoteCache.RequestHandler");

std::string localHost(boost::asio::ip::tcp::socket& socket)
{
  std::string localHost;
  boost::system::error_code ignore;
  try
  {
    localHost = boost::lexical_cast<std::string>(socket.local_endpoint(ignore).address());
  }
  catch (...)
  {
    // ignore
  }
  return localHost;
}

std::string remoteHost(boost::asio::ip::tcp::socket& socket)
{
  std::string remoteHost;
  boost::system::error_code ignore;
  try
  {
    remoteHost = boost::lexical_cast<std::string>(socket.remote_endpoint(ignore).address());
  }
  catch (...)
  {
    // ignore
  }
  return remoteHost;
}

bool cacheUpdateStopped()
{
  long detectionInterval(ReadConfig::getCacheUpdateDetectionInterval());
  std::time_t cacheUpdateTimestamp(TseSrvStats::cacheUpdateTimestamp());
  std::time_t currentTime(std::time(nullptr));
  return std::difftime(currentTime, cacheUpdateTimestamp) > detectionInterval;
}

}// namespace

void handleRequest(boost::asio::ip::tcp::socket& socket,
                   const Request& req,
                   Reply& rsp)
{
  const MallocContextDisabler context;
  static const std::string UNKNOWN("UNKNOWN");
  static const std::string thisHost(localHost(socket));
  std::string client(remoteHost(socket));
  //std::cerr << "\tremoteHost:" << client << std::endl;
  if (cacheUpdateStopped())
  {
    rsp._header._status = RC_MASTER_CACHE_UPDATE_STOPPED;
    LOG4CXX_ERROR(logger, "slave:" << client
                  << ',' << statusToString(rsp._header._status));
    RCStatistics::registerServerCall(client, UNKNOWN, rsp);
    return;
  }
  RBuffer is(req._payload);
  char persistentCh('N');
  is.read(persistentCh);
  bool persistentClient('P' == persistentCh);
  bool persistentServer(ReadConfig::usePersistentConnections());
  if (persistentClient != persistentServer)
  {
    rsp._header._status = RC_INCOMPATIBLE_MODE;
    LOG4CXX_ERROR(logger, "slave:" << client << ',' << statusToString(rsp._header._status));
    RCStatistics::registerServerCall(client, UNKNOWN, rsp);
    return;
  }
  std::string baseline;
  is.read(baseline);
  bool checkBaseline(ReadConfig::getCheckBaseline());
  if (checkBaseline && !boost::iequals(BUILD_LABEL_STRING, baseline))
  {
    rsp._header._status = RC_BASELINE_MISMATCH;
    LOG4CXX_ERROR(logger, "slave:" << client << ',' << statusToString(rsp._header._status)
                  << ",this baseline:" << BUILD_LABEL_STRING
                  << ",client baseline:" << baseline);
    RCStatistics::registerServerCall(client, UNKNOWN, rsp);
    return;
  }
  if (client == thisHost)
  {
    rsp._header._status = RC_REQUEST_FROM_SAME_HOST;
    LOG4CXX_ERROR(logger, "slave:" << client << ',' << statusToString(rsp._header._status)
                  << ",thisHost:" << thisHost);
    RCStatistics::registerServerCall(client, UNKNOWN, rsp);
    return;
  }
  std::string tableName;
  is.read(tableName);
  CacheRegistry& registry(CacheRegistry::instance());
  CacheControl* ctrl(registry.getCacheControlUpperCase(tableName));
  if (0 == ctrl)
  {
    rsp._header._status = RC_NOT_FOUND;
    LOG4CXX_ERROR(logger, "slave:" << client
                  << ',' << tableName << " is not known");
    RCStatistics::registerServerCall(client, tableName, rsp);
    return;
  }
  uint32_t serverVersion(ctrl->tableVersion());
  uint32_t clientVersion(req._header._daoVersion);
  if (serverVersion != clientVersion)
  {
    rsp._header._status = RC_DAO_VERSION_MISMATCH;
    rsp._header._daoVersion = serverVersion;
    LOG4CXX_ERROR(logger, "slave:" << client
                  << ',' << statusToString(rsp._header._status) << ',' << tableName
                  << ",serverVersion=" << serverVersion << ",clientVersion=" << clientVersion);
    RCStatistics::registerServerCall(client, tableName, rsp);
    return;
  }
  char historical('N');
  is.read(historical);
  bool isHistorical('H' == historical);
  static const bool masterHistorical(Global::allowHistorical());
  if (isHistorical && !masterHistorical)
  {
    rsp._header._status = RC_MASTER_NONHISTORICAL;
    LOG4CXX_ERROR(logger, "slave:" << client
                  << ',' << statusToString(rsp._header._status) << ',' << tableName);
    RCStatistics::registerServerCall(client, tableName, rsp);
    return;
  }
  bool inTransition(false);
  std::string serverDatabase(getCurrentDatabase(isHistorical, inTransition));
  std::string clientDatabase;
  is.read(clientDatabase);
  bool ignoreDBMismatch(ReadConfig::getIgnoreDatabaseMismatch());
  if (!ignoreDBMismatch && serverDatabase != clientDatabase)
  {
    rsp._header._status = RC_DATABASE_MISMATCH;
    LOG4CXX_ERROR(logger, "slave:" << client
                  << ',' << statusToString(rsp._header._status)
                  << ",serverDatabase:" << serverDatabase
                  << ",clientDatabase:" << clientDatabase);
    RCStatistics::registerServerCall(client, tableName, rsp);
    return;
  }
  bool fromQuery(false);
  try
  {
    sfc::CompressedDataPtr compressed(ctrl->getCompressed(is, rsp._header._status, fromQuery));
    if (RC_COMPRESSED_VALUE == rsp._header._status && compressed)
    {
      rsp._header._payloadSize = compressed->_deflated.size();
      rsp._header._inflatedSize = compressed->_inflatedSz;
      if (!compressed->_deflated.empty())
      {
        rsp._compressed = compressed;
      }
      LOG4CXX_DEBUG(logger, "rsp._header._payloadSize="
                    << rsp._header._payloadSize
                    << " rsp._header._inflatedSize=" << rsp._header._inflatedSize);
    }
    else if (RC_COMPRESSED_VALUE != rsp._header._status
             && RC_UNCOMPRESSED_VALUE != rsp._header._status)
    {
      LOG4CXX_ERROR(logger, "slave:" << client << ','
                    << statusToString(rsp._header._status) << ':' << tableName);
    }
  }
  catch (const TSEException& e)
  {
    rsp._header._status = RC_SERVER_ERROR;
    LOG4CXX_ERROR(logger, "slave:" << client
                  << ",RC: TSEException:" << e.what() << ':' << tableName);
  }
  catch (const ErrorResponseException& e)
  {
    rsp._header._status = RC_SERVER_ERROR;
    LOG4CXX_ERROR(logger, "slave:" << client
                  << ",RC: ErrorResponseException:" << e.message() << ':' << tableName);
  }
  catch (const boost::system::system_error& e)
  {
    rsp._header._status = RC_SERVER_ERROR;
    LOG4CXX_ERROR(logger, "slave:" << client
                  << ",RC: system_error:" << e.what() << ':' << tableName);
  }
  catch (const std::exception& e)
  {
    rsp._header._status = RC_SERVER_ERROR;
    LOG4CXX_ERROR(logger, "slave:" << client
                  << ",RC: std::exception:" << e.what() << ':' << tableName);
    if (0 == strlen(e.what()))
    {
      assert(false);
    }
  }
  catch (...)
  {
    rsp._header._status = RC_UNKNOWN_ERROR;
    LOG4CXX_ERROR(logger, "slave:" << client
                  << ':' << statusToString(rsp._header._status) << ':' << tableName);
    assert(false);
  }
  RCStatistics::registerServerCall(client, tableName, rsp, fromQuery);
}

}// RequestHandler

}// RemoteCache

}// tse
