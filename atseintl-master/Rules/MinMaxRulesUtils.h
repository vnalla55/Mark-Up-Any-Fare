//----------------------------------------------------------------------------
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//----------------------------------------------------------------------------

#pragma once

#include "Common/DateTime.h"
#include "Common/TsePrimitiveTypes.h"

namespace tse
{
namespace MinMaxRulesUtils
{
static constexpr char EARLIER = 'E';
static constexpr char LATER = 'L';
DateTime
determineReturnDate(const DateTime& periodReturnDate,
                    const DateTime& stayDate,
                    const Indicator earlierLaterInd);
}
}
