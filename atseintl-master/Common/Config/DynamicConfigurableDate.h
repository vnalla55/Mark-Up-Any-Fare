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

#include "Common/DateTime.h"
#ifdef CONFIG_HIERARCHY_REFACTOR
#include "Common/Config/ConfigurableValueAdapter.h"
#include "Common/Config/DynamicConfigurableValue.h"
#else
#include "Common/Config/DynamicConfigurableValueImpl.h"
#endif


#include <functional>

namespace tse
{

struct TicketingDate
{
  const DateTime& operator()(const Trx* trx) const { return trx->ticketingDate(); }
};

#ifdef CONFIG_HIERARCHY_REFACTOR

typedef ConfigurableValueAdapter<DynamicConfigurableValue<DateTime>,
                                 std::greater_equal<DateTime>,
                                 TicketingDate> DynamicConfigurableEffectiveDate;
typedef ConfigurableValueAdapter<DynamicConfigurableValue<DateTime>,
                                 std::less<DateTime>,
                                 TicketingDate> DynamicConfigurableDiscontinueDate;
#else

typedef DynamicConfigurableValueImpl<DateTime, std::greater_equal<DateTime>, TicketingDate>
DynamicConfigurableEffectiveDate;
typedef DynamicConfigurableValueImpl<DateTime, std::less<DateTime>, TicketingDate>
DynamicConfigurableDiscontinueDate;

#endif
} // tse

