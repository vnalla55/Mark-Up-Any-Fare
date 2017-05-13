#include "DBAccess/RemoteCache/RCStatistics.h"

#include "DBAccess/RemoteCache/RCServerAttributes.h"
#include "DBAccess/RemoteCache/ReadConfig.h"
#include "DBAccess/RemoteCache/ASIOServer/Connection.h"
#include "DBAccess/RemoteCache/ASIOServer/Session.h"
#include "DBAccess/RemoteCache/ASIOClient/AsyncClient.h"
#include "DBAccess/RemoteCache/ASIOClient/Client.h"
#include "DBAccess/RemoteCache/ASIOClient/RCRequest.h"

#include "Common/Logger.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <map>
#include <mutex>
#include <vector>

namespace tse
{

namespace RemoteCache
{

namespace RCStatistics
{

enum StatsType
{
  SERVER
  , CLIENT
};

namespace
{

Logger statsLogger("atseintl.RemoteCache.RCStatistics");

std::atomic<uint64_t> numberServerCalls(0);

std::atomic<uint64_t> numberClientCalls(0);

std::atomic<uint64_t> numberFailed(0);

std::atomic<uint64_t> numberThrottled(0);

std::mutex mutex;

typedef std::vector<StatusType> StatusVector;

struct StatsData
{
  StatsData()
    : _totalLatency(0)
    , _totalQueries(0)
  {
  }
  StatusVector _statusVector;
  std::uint64_t _totalLatency;
  std::uint64_t _totalQueries;
};

typedef std::map<std::string, StatsData> DataMap;
typedef std::map<std::string, DataMap> StatisticsMap;

StatisticsMap serverMap;
StatisticsMap clientMap;

void outputStats(StatsType type)
{
  std::ostringstream os;
  std::atomic<uint64_t>& counter = SERVER == type ? numberServerCalls : numberClientCalls;
  const std::string title(SERVER == type ? "Server" : "Client");
  os << "RC " << title << " : total calls=" << counter;
  bool persistent(ReadConfig::usePersistentConnections());
  if (CLIENT == type)
  {
    int connectios(persistent ? Client::getNumberClients() : AsyncClient::getNumberClients());
    os << " throttled=" << numberThrottled << " failed=" << numberFailed << " connections=" << connectios;
  }
  else if (SERVER == type)
  {
    int connections(persistent ? Session::getNumberSessions() : Connection::getNumberConnections());
    os << " sessions=" << connections;
  }
  os << "\n\n";
  StatisticsMap& statisticsMap(SERVER == type ? serverMap : clientMap);
  std::string align(SERVER == type ? "  " : "");
  for (const auto& entry : statisticsMap)
  {
    size_t numberSamples(0);
    for (const auto& pair : entry.second)
    {
      numberSamples += pair.second._statusVector.size();
    }
    const std::string section(SERVER == type ? "client" : "server");
    os << ' ' << section << ": " << entry.first << ": " << numberSamples << " samples\n";
    for (const auto& pair : entry.second)
    {
      uint64_t success(0);
      uint64_t fail(0);
      for (auto status : pair.second._statusVector)
      {
        switch (status)
        {
        case RC_COMPRESSED_VALUE:
        case RC_UNCOMPRESSED_VALUE:
          ++success;
          break;
        default:
          ++fail;
          break;
        }
      }
      std::size_t numberSamples(pair.second._statusVector.size());
      long meanLatency(0 == numberSamples ? 0 : pair.second._totalLatency / numberSamples);
      os << align << pair.first << " : " << success << " success, " << fail << " fail,";
      if (SERVER == type)
      {
        os << ' ' << pair.second._totalQueries << " queries,";
      }
      os << ' ' << meanLatency << "us\n";
    }
  }
  os << "\n       ===============================================\n";
  LOG4CXX_INFO(statsLogger, os.str());
}

void registerCall(const std::string& host,
                  const std::string& dataType,
                  StatusType status,
                  const std::chrono::steady_clock::time_point& start,
                  StatsType type,
                  bool fromQuery = false)
{
  int loggingInterval(ReadConfig::getStatsLoggingInterval());
  int samplingInterval(ReadConfig::getStatsSamplingInterval());
  std::atomic<uint64_t>& counter(SERVER == type ? numberServerCalls : numberClientCalls);
  ++counter;
  if (CLIENT == type)
  {
    switch (status)
    {
      case RC_NONE:
      case RC_REQUEST_CANCELED:
        ++numberThrottled;
        break;
      case RC_COMPRESSED_VALUE:
      case RC_UNCOMPRESSED_VALUE:
        break;
      default:
        ++numberFailed;
        break;
    }
  }
  StatisticsMap& statisticsMap(SERVER == type ? serverMap : clientMap);
  if (loggingInterval > 0 && 0 == counter % samplingInterval)
  {
    std::chrono::steady_clock::time_point finish(std::chrono::steady_clock::now());
    long latency(std::chrono::duration_cast<std::chrono::microseconds>(finish - start).count());
    std::unique_lock<std::mutex> lock(mutex);
    DataMap emptyMap;
    auto pair(statisticsMap.emplace(host, emptyMap));
    StatsData emptyData;
    auto pair2(pair.first->second.emplace(dataType, emptyData));
    pair2.first->second._statusVector.emplace_back(status);
    pair2.first->second._totalLatency += latency;
    pair2.first->second._totalQueries += fromQuery ? 1 : 0;
  }
  static std::atomic<std::time_t> lastLoggingServer(std::time(nullptr));
  static std::atomic<std::time_t> lastLoggingClient(std::time(nullptr));
  std::atomic<std::time_t>& lastLogging = SERVER == type ? lastLoggingServer : lastLoggingClient;
  std::time_t now(std::time(nullptr));
  if (now != -1)
  {
    double diff(std::difftime(now, lastLogging));
    if (diff > loggingInterval)
    {
      std::unique_lock<std::mutex> lock(mutex);
      lastLogging = now;
      outputStats(type);
    }
  }
}

}// namespace

void registerServerCall(const std::string& client,
                        const std::string& dataType,
                        const Reply& reply,
                        bool fromQuery)
{
  StatusType status(reply._header._status);
  registerCall(client, dataType, status, reply._start, SERVER, fromQuery);
}

void registerClientCall(const RCServerAttributes& serverAttributes,
                        const RCRequest& request)
{
  const std::string& server(serverAttributes.hostPort()._host);
  const std::string& dataType(request._dataType);
  StatusType status(request._responseHeader._status);
  registerCall(server, dataType, status, request._start, CLIENT);
}

}// RCStatistics

} // RemoteCache

} // tse
