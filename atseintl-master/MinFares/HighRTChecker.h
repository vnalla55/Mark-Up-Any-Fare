//----------------------------------------------------------------------------
//
//
//  File:     HighRTChecker.h
//  Created:  2/28/2011
//  Authors:
//
//  Description	:
//
//  Copyright Sabre 2004
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

#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

namespace tse
{
class FareUsage;
class PricingUnit;
class MinFarePlusUpItem;
/**
 * @class HighRTChecker - utility class to assist in checking
 * exemptions and calculation of HRT fare for diff trip types (OJ, RT etc).
*/

namespace HighRTChecker
{
bool
qualifyCat10Subcat101Byte13(const PricingUnit& pu);
bool
qualifyCat10Subcat102Byte13(const PricingUnit& pu);
bool
isSameCabin(const PricingUnit& pu);
const FareUsage*
getHighestRoundTripFare(const PricingUnit& pu, MoneyAmount& RT_FareAmt);

const FareUsage*
getHighestRoundTripNetFare(const PricingUnit& pu, MoneyAmount& RT_NetFareAmt);

void
getHrtPlusUp(const MoneyAmount& HRT_FareAmt,
             const MoneyAmount& RT_FareAmt,
             const FareUsage* highestRtFu,
             MinFarePlusUpItem& hrtPlusUp);

bool
calculateFareAmt(const PricingUnit& pu, MoneyAmount& ojFareAmt);
bool
calculateNetFareAmt(const PricingUnit& pu, MoneyAmount& ojNetFareAmt);
} // namespace HighRTChecker
} // namespace tse
