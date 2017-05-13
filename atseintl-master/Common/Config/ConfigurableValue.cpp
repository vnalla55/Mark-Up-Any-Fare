// ----------------------------------------------------------------
//
//   Copyright Sabre 2014
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------


#ifndef CONFIG_HIERARCHY_REFACTOR

#include "Common/Config/ConfigurableValue.h"

namespace tse
{
Logger
ConfigurableValueBase::_logger("atseintl.Common.ConfigurableValue");

void
ConfigurableValueBase::resetConfigured(const bool dynamicOnly)
{
  std::lock_guard<std::mutex> guard(this->_mutex);
  if (dynamicOnly && !isDynamic())
    return;
  _isConfigured.store(false, std::memory_order_release);
}

std::string
ConfigurableValueBase::getConfigMsg(const Trx* trx, const bool dynamicOnly)
{
  if (dynamicOnly && !isDynamic())
    return std::string();
  return getConfigMsgImpl(trx);
}
}

#endif
