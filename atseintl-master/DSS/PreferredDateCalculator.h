//----------------------------------------------------------------------------
//
//  File:        PreferredDateCalculator.h
//  Created:     06/07/2005
//  Authors:     Adrian Tovar
//
//  Description: Utilize user date inputs to return calculated preferred date range
//               adjustment values.
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

#include "Common/TseStringTypes.h"

namespace tse
{
class FareDisplayRequest;

namespace PreferredDateCalculator
{
// -------------------------------------------------------------------
//
// @MethodName  preferredDateRangeCalculator()
//
// Use preferred date and travel date range input by user to determine
// preferred date range.
//
// @param request  - pointer to Fare Display request containing user inputs
// @param options  - pointer to Fare Display options
// @param beginDateAdjValue - reference field used to return adjusted begin date value
// @param endDateAdjValue   - reference field used to return adjusted end date value
//
//
// @return bool - true is range determined, false error
// -------------------------------------------------------------------------
void
calculate(FareDisplayRequest& request, uint16_t& beginDateAdjValue, uint16_t& endDateAdjValue);
};
} // end tse namespace
