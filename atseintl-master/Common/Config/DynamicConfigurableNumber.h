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

struct IntegerNumber
{
  const uint32_t operator()(const Trx* trx) { return 0; }
};

#ifdef CONFIG_HIERARCHY_REFACTOR
typedef ConfigurableValueAdapter<DynamicConfigurableValue<uint32_t>,
                                 std::less<uint32_t>,
                                 IntegerNumber> DynamicConfigurableNumber;

#else

typedef DynamicConfigurableValueImpl<uint32_t, std::less<uint32_t>, IntegerNumber>
DynamicConfigurableNumber;

#endif
} // tse

