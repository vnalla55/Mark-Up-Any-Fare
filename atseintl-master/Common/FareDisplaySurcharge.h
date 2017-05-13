//----------------------------------------------------------------------------
//
//  File:            FareDisplaySurcharge.h
//  Created:     05/17/2006
//  Authors:     Hitha Alex
//
//  Description:    Common functions required for Surcharge Calculation of ATSE fare display
//
//  Copyright Sabre 2006
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
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

#pragma once

namespace tse
{
class FareDisplayTrx;
class Money;
class PaxTypeFare;
class SurchargeData;

namespace FareDisplaySurcharge
{
// Get the oneway surcharge. This includes only outbound surcharges.
bool
getTotalOWSurcharge(const FareDisplayTrx& trx,
                    const PaxTypeFare& paxTypeFare,
                    MoneyAmount& owSurchargeAmount);

// Get Total round trip surcharge. This includes inbound and outbound surcharges
bool
getTotalRTSurcharge(const FareDisplayTrx& trx,
                    const PaxTypeFare& paxTypeFare,
                    MoneyAmount& rtSurchargeAmount);
// Convert the surcharge to display currency
MoneyAmount
convertCurrency(const FareDisplayTrx* trx,
                const PaxTypeFare& paxTypeFare,
                SurchargeData* surcharge);
// Use nuc converter to convert to and from NUC currency
void
convertNUC(const FareDisplayTrx* trx,
           const PaxTypeFare& paxTypeFare,
           const Money& source,
           Money& destination);

// Round the total surcharge
void
roundAmount(const FareDisplayTrx* trx, MoneyAmount& surchargeAmount);
}
} // namespace
