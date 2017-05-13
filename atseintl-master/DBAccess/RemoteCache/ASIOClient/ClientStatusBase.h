#pragma once

#include "DBAccess/RemoteCache/RemoteCacheHeader.h"
#include "DBAccess/RemoteCache/ASIOClient/RCRequest.h"

#include <atomic>
#include <ctime>
#include <mutex>
#include <string>

namespace tse
{

namespace RemoteCache
{

class ClientStatusBase
{
public:
  explicit ClientStatusBase(const std::string& dataType = "");

  std::pair<bool, StatusType> isEnabled();
  void set(RCRequestPtr request,
           StatusType status);
  static ClientStatusBase& instance(const std::string& dataType);
  const std::string& getDataType() const { return _dataType; }
protected:
  void reset();

  const std::string _dataType;
  std::atomic<bool> _enabled{true};
  StatusType _status{RC_NONE};
  std::time_t _errorTime{};
  std::time_t _retryInterval{};
  std::mutex _mutex;
};

}// RemoteCache

}// tse
