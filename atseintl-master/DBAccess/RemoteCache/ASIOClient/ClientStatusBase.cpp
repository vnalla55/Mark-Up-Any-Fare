#include "DBAccess/RemoteCache/ASIOClient/ClientStatusBase.h"

#include "Common/Logger.h"
#include "DBAccess/RemoteCache/ReadConfig.h"

#include <map>
#include <memory>

namespace tse
{

namespace RemoteCache
{

namespace
{

Logger logger("atseintl.DBAccess.RemoteCache.ClientStatusBase");

typedef std::shared_ptr<ClientStatusBase> ClientStatusBasePtr;
typedef std::map<std::string, ClientStatusBasePtr> StatusMap;

StatusMap statusMap;

std::mutex mapMutex;

}// namespace

ClientStatusBase::ClientStatusBase(const std::string& dataType)
  : _dataType(dataType)
{
}

std::pair<bool, StatusType>  ClientStatusBase::isEnabled()
{
  if (_enabled)
  {
    return { _enabled, RC_NONE };
  }
  std::unique_lock<std::mutex> lock(_mutex);
  std::time_t currentTime(std::time(nullptr));
  if (currentTime >= _errorTime + _retryInterval)
  {
    reset();
    return { _enabled, RC_NONE };
  }
  return { _enabled, _status };
}

void ClientStatusBase::set(RCRequestPtr request,
                           StatusType status)
{
  bool persistent(ReadConfig::usePersistentConnections());
  bool canUseSecondary(persistent && ReadConfig::isHostStatus(status));
  std::unique_lock<std::mutex> lock(_mutex);
  if (canUseSecondary)
  {
    if (_enabled)
    {
      _status = status;
      _enabled = false;
      _retryInterval = 0;
    }
    return;
  }
  _errorTime = std::time(nullptr);
  _enabled = false;
  if (_status != status)
  {
    _status = status;
    _retryInterval = ReadConfig::getClientRetryInterval(status);
    std::ostringstream os;
    os << request->_requestId << ',' << _dataType << ",status=" << status << ','
       << statusToString(status) << ",retry in " << _retryInterval << 's';
    if (RC_CLIENT_PROCESSING_TIMEOUT == status)
    {
      LOG4CXX_INFO(logger, os.str());
    }
    else
    {
      LOG4CXX_WARN(logger, os.str());
    }
  }
  if (RC_CLIENT_PROCESSING_TIMEOUT == status)
  {
    request->checkTrxTimeout();
  }
}

void ClientStatusBase::reset()
{
  _enabled = true;
  _status = RC_NONE;
  _errorTime = 0;
  _retryInterval = 0;
}

ClientStatusBase& ClientStatusBase::instance(const std::string& dataType)
{
  std::unique_lock<std::mutex> lock(mapMutex);
  auto it(statusMap.find(dataType));
  if (it != statusMap.end())
  {
    return *it->second;
  }
  else
  {
    ClientStatusBasePtr item(new ClientStatusBase(dataType));
    auto pr(statusMap.emplace(dataType, item));
    return *pr.first->second;
  }
}

}// RemoteCache

}// tse
