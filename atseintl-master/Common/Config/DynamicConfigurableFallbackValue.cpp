//----------------------------------------------------------------------------
//
//  Copyright Sabre 2011
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------


#ifndef CONFIG_HIERARCHY_REFACTOR

#include "Common/Config/DynamicConfigurableFallbackValue.h"

namespace tse
{
namespace fallback
{
void
DynamicConfigurableFallbackValue::set(bool value)
{
  std::lock_guard<std::mutex> guard(_mutex);

  _value = value;
  _isConfigured.store(true, std::memory_order_release);
}

void
DynamicConfigurableFallbackValue::set(const tse::ConfigMan* config)
{
  std::lock_guard<std::mutex> guard(_mutex);

  if (_isConfigured.load(std::memory_order_acquire))
    return;

  const bool prevValue = _value;

  getValueFrom(*config, _value);
  if (_value != prevValue)
    loggerMessage(_option, _value);

  _isConfigured.store(true, std::memory_order_release);
}

} // fallback

} // tse

#endif
