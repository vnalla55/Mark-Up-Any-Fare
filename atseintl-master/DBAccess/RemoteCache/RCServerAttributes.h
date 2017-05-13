#pragma once

#include <atomic>
#include <string>

namespace tse
{

namespace RemoteCache
{

struct HostPort
{
  bool operator == (const HostPort& other) const
  {
    return _host == other._host && _port == other._port;
  }

  std::string _host;
  std::string _port;
  bool _sameHost{false};

  bool empty() const
  {
    return _host.empty() || _port.empty();
  }
};

class RCServerAttributes
{
public:
  RCServerAttributes() {}
  RCServerAttributes(const RCServerAttributes& other);

  bool operator == (const RCServerAttributes& other) const
  {
    return _primary == other._primary && _secondary == other._secondary;
  }

  bool empty() const { return primary().empty() && secondary().empty(); }

  const HostPort& hostPort() const { return _useSecondary ? _secondary : _primary; }
  HostPort& hostPort() { return _useSecondary ? _secondary : _primary; }

  const HostPort& primary() const { return _primary; }
  HostPort& primary() { return _primary; }

  const HostPort& secondary() const { return _secondary; }
  HostPort& secondary() { return _secondary; }

  bool isSecondary() const { return _useSecondary; }
  void setSecondary() { _useSecondary = true; }
  void setPrimary() { _useSecondary = false; }

  static bool isMaster(const std::string& dataType);

private:
  HostPort _primary;
  HostPort _secondary;
  std::atomic<bool> _useSecondary{false};
};

} // RemoteCache

} // tse
