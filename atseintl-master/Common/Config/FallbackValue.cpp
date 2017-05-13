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

#include "Common/Config/FallbackValue.h"

#include "Common/Config/ConfigMan.h"
#include "Common/TseUtil.h"

namespace tse
{

namespace fallback
{

void
FallbackValue::set(const tse::ConfigMan* config)
{
  std::string optionValue;
  if (config->getValue(_option, optionValue, CONFIG_SECTION))
  {
    bool prevValue = _value;
    _value = TseUtil::boolValue(optionValue);
    if (_value != prevValue)
      loggerMessage(_option, _value);
  }
}

} // fallback

} // tse


#endif
