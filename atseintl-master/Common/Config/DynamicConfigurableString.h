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

#include <functional>

namespace tse
{

struct stringVal
{
  const std::string operator()(const Trx* trx) { return ""; }
};

#ifdef CONFIG_HIERARCHY_REFACTOR

typedef ConfigurableValueAdapter<DynamicConfigurableValue<std::string>,
                                 std::equal_to<std::string>,
                                 stringVal> DynamicConfigurableString;

#else

typedef DynamicConfigurableValueImpl<std::string, std::equal_to<std::string>, stringVal>
DynamicConfigurableString;

#endif
} // tse
