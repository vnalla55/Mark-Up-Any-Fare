#include "DBAccess/RemoteCache/ASIOClient/RCClient.h"

#include "Common/FallbackUtil.h"
#include "Common/TSSCacheCommon.h"
#include "DBAccess/RemoteCache/RCStatistics.h"
#include "DBAccess/RemoteCache/ASIOClient/AsyncClient.h"
#include "DBAccess/RemoteCache/ASIOClient/ClientManager.h"

namespace tse
{

FIXEDFALLBACK_DECL(fallbackRCMultiTransportFix);

namespace RemoteCache
{

namespace
{

Logger logger("atseintl.DBAccess.RemoteCache.RCClient");

}// namespace

namespace RCClient
{

Logger& getLogger()
{
  return logger;
}

bool isFallbackRCMultiTransportFix()
{
  return fallback::fixed::fallbackRCMultiTransportFix();
}

void get(RCRequestPtr request,
         RCServerAttributes& serverAttributes)
{
  RCRequest::checkTimeout();
  bool persistent(ReadConfig::usePersistentConnections());
  if (persistent)
  {
    if (ClientManager::enqueue(serverAttributes, request))
    {
      request->wait();
    }
  }
  else
  {
    int maxNumberClients(ReadConfig::getMaxNumberClients());
    int numberClients(AsyncClient::getNumberClients());
    if (numberClients > maxNumberClients)
    {
      request->_responseHeader._status = RC_MAX_NUMBER_CLIENTS_EXCEEDED;
      RCStatistics::registerClientCall(serverAttributes, *request);
      return;
    }
    static const size_t DEFAULT_BUFFER_SIZE(262144);
    long clientConnectionTimeout(ReadConfig::getClientConnectionTimeout() * 1000);
    std::vector<char>* pooledBuffer(tsscache::getMemoryBuffer());
    std::vector<char> localBuffer;
    std::vector<char>* buffer(0);
    if (pooledBuffer)
    {
      buffer = pooledBuffer;
    }
    if (0 == buffer || buffer->empty())
    {
      localBuffer.resize(DEFAULT_BUFFER_SIZE);
      buffer = &localBuffer;
    }
    assert(buffer != 0);
    bool linger(ReadConfig::isLinger());
    unsigned lingerTime(ReadConfig::getLingerTime());
    AsyncClient client(serverAttributes.primary()._host,
                       serverAttributes.primary()._port,
                       clientConnectionTimeout,
                       request->_maxProcessingTime,
                       linger,
                       lingerTime,
                       *buffer);
    client.process(*request);
    assert(request->_requestId == request->_responseHeader._requestId);
  }
  RCStatistics::registerClientCall(serverAttributes, *request);
}

}// RCClient

}// RemoteCache

}// tse
