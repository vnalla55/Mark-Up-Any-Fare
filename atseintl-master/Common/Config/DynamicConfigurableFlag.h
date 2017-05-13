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

#pragma once

#ifdef CONFIG_HIERARCHY_REFACTOR
#include "Common/Config/ConfigurableValueAdapter.h"
#include "Common/Config/DynamicConfigurableValue.h"
#else
#include "Common/Config/DynamicConfigurableValueImpl.h"
#endif

#include "Common/TypeConvert.h"

#include <functional>

namespace tse
{

class Trx;

struct FlagOn
{
  bool operator()(const Trx*) { return true; }
};

struct FlagOff
{
  bool operator()(const Trx*) { return false; }
};

#ifdef CONFIG_HIERARCHY_REFACTOR

typedef ConfigurableValueAdapter<DynamicConfigurableValue<bool>, std::equal_to<bool>, FlagOn>
DynamicConfigurableFlagOn;
typedef ConfigurableValueAdapter<DynamicConfigurableValue<bool>, std::equal_to<bool>, FlagOff>
DynamicConfigurableFlagOff;

#else

typedef DynamicConfigurableValueImpl<bool, std::equal_to<bool>, FlagOn> DynamicConfigurableFlagOn;
typedef DynamicConfigurableValueImpl<bool, std::equal_to<bool>, FlagOff> DynamicConfigurableFlagOff;

#endif
} // tse

