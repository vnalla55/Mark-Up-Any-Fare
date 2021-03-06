// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#pragma once

#include "DataModel/Common/SafeEnums.h"
#include "DataModel/Common/Types.h"

namespace tax
{

type::MoneyAmount
doRound(type::MoneyAmount amount,
        const type::MoneyAmount& unit,
        const type::TaxRoundingDir& dir,
        bool doTruncation);
} // end of tax namespace
