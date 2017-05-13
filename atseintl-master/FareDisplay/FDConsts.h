//----------------------------------------------------------------------------
//  Authors:        Abu
//  Description:    Common constants for FareDisplay
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

#include "Common/TseConsts.h"

namespace tse
{
// Domestic_International_Indicator for Grouping and Sorting
constexpr Indicator INDICATOR_DOMESTIC = 'D';
constexpr Indicator INDICATOR_INTERNATIONAL = 'I';
constexpr Indicator SINGLE_CXR_FD_TYPE = 'S';
constexpr Indicator MULTIPLE_CXR_FD_TYPE = 'M';
extern const std::string FARE_RB_REQUEST;
extern const std::string ALL_INCLUSION_CODE;
extern const std::string RD_REQUEST;
extern const std::string FQ_REQUEST;
extern const std::string MP_REQUEST;

enum MPType
{
  SHORT_MP,
  LONG_MP,
  NO_MARKET_MP
};
}

