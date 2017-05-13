#pragma once

#include "Common/Logger.h"
#include "DBAccess/CreateResult.h"
#include "DBAccess/RemoteCache/ASIOClient/ClientStatusBase.h"
#include "DBAccess/RemoteCache/ASIOClient/RCRequest.h"
#include "DBAccess/RemoteCache/RCServerAttributes.h"
#include "DBAccess/RemoteCache/ReadConfig.h"
#include <boost/algorithm/string/predicate.hpp>

namespace tse
{

class Logger;

namespace RemoteCache
{

namespace RCClient
{

Logger& getLogger();
bool isFallbackRCMultiTransportFix();

template <typename DAO> class ClientStatus : public ClientStatusBase
{
public:

  ClientStatus(const DAO&,
               const std::string& dataType)
    : ClientStatusBase(dataType)
  {
  }
};

void get(RCRequestPtr request,
         RCServerAttributes& serverAttributes);

template <typename DAO, typename Key, typename T>
void get(DAO& dao,
         const Key& key,
         CreateResult<T>& ret,
         bool isHistorical)
{
  if (!ReadConfig::isEnabled())
  {
    return;
  }
  const uint32_t daoVersion(dao.tableVersion());
  const std::string dataType(boost::to_upper_copy(dao.name()));
  // copy
  auto serverAttributes(ReadConfig::getServer(dataType));
  const auto& hp(serverAttributes.hostPort());
  const auto& hppr(serverAttributes.primary());
  const auto& hpsec(serverAttributes.secondary());
  if (!hp.empty() && !hppr._sameHost && !hpsec._sameHost)
  {
    static ClientStatus<DAO> clientStatusOld(dao, dataType);
    auto& clientStatusNew(ClientStatusBase::instance(dataType));
    auto& clientStatus(isFallbackRCMultiTransportFix() ? clientStatusOld : clientStatusNew);
    if (clientStatus.isEnabled().first)
    {
      RCRequestPtr request(new RCRequest(dataType));
      if (populateRequest(request, key, isHistorical, daoVersion))
      {
        get(request, serverAttributes);
      }
      StatusType status(request->_responseHeader._status);
      ret._status = status;
      if (RC_COMPRESSED_VALUE == status)
      {
        ret._compressed = request->processResponse();
        if (ret._compressed)
        {
          static const bool debugEnabled(getLogger()->isDebugEnabled());
          if (debugEnabled)
          {
            LOG4CXX_DEBUG(getLogger(), __FUNCTION__
                              << " dataType:" << dataType << ",key:" << key << ",status=" << status
                              << ',' << statusToString(status)
                              << ",ret._compressed->_deflated.size()="
                              << ret._compressed->_deflated.size()
                              << ",ret._compressed->_inflatedSz=" << ret._compressed->_inflatedSz
                              << ",displaying not empty items");
          }
          ret._ptr = dao.uncompress(*ret._compressed);
        }
      }
      else if (RC_UNCOMPRESSED_VALUE == status)
      {
        ret._ptr = new T;
      }
      else
      {
        clientStatus.set(request, status);
      }
    }
  }
  else
  {
    ret._status = RC_NOT_CLIENT_OR_DISABLED;
  }
}

}// RCClient

}// RemoteCache

}// tse
