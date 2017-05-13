#include "DBAccess/RemoteCache/RCServerAttributes.h"
#include "DBAccess/RemoteCache/ReadConfig.h"

namespace tse
{

namespace RemoteCache
{

RCServerAttributes::RCServerAttributes(const RCServerAttributes& other)
  : _primary(other._primary)
  , _secondary(other._secondary)
{
}

bool RCServerAttributes::isMaster(const std::string& dataType)
{
  bool rcEnabled(ReadConfig::isEnabled());
  if (rcEnabled)
  {
    const RCServerAttributes& attributes(ReadConfig::getServer(dataType));
    const HostPort& primary(attributes._primary);
    const HostPort& secondary(attributes._secondary);
    return primary._sameHost || secondary._sameHost;
  }
  return false;
}

} // RemoteCache

} // tse
