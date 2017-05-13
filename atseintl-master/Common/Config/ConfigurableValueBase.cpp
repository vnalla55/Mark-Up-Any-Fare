#ifdef CONFIG_HIERARCHY_REFACTOR

#include "Common/Config/ConfigurableValueBase.h"

#include <algorithm>

namespace tse
{
Logger
ConfigurableValueBase::_logger("atseintl.Common.ConfigurableValue");
Logger
ConfigurableValueBase::_tseServerlogger("atseintl.Server.TseServer");

std::string
ConfigurableValueBase::getConfigMsg(const Trx* trx, const bool dynamicOnly)
{
  if (dynamicOnly && !isDynamic())
    return std::string();

  std::string configMsg(_section);
  configMsg += '/';
  configMsg += _option;
  configMsg += '/';
  configMsg += getValueString(trx);
  std::replace(configMsg.begin(), configMsg.end(), '_', ' ');
  return configMsg;
}
}

#endif
