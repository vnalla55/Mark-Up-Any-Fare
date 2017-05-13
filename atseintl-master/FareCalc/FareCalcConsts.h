//-------------------------------------------------------------------------------
// FareCalcConsts.h
//
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#pragma once

#include "Common/TsePrimitiveTypes.h"

#include <string>

namespace tse
{
class FareCalcConsts
{
public:
  static const std::string FC_NODIAG;
  static const std::string FC_APPEND_CHILD; // CH
  static const std::string FC_APPEND_INFANT; // IN
  static const std::string FC_CHILD_PAX; // pax type = CNN
  static const std::string TAX_CODE_ZP;
  static const std::string TAX_CODE_XF;
  static const std::string TAX_CODE_XT;
  static const std::string YY_CARRIER;
  static const std::string BLANK_CARRIER;

  static constexpr Indicator HORIZONTAL_FARECALC1 = '1';
  static constexpr Indicator VERTICAL_FARECALC2 = '2';
  static constexpr Indicator MIX_FARECALC3 = '3';

  static constexpr Indicator FC_ONE = '1';
  static constexpr Indicator FC_TWO = '2';
  static constexpr Indicator FC_THREE = '3';
  static constexpr Indicator FC_FOUR = '4';
  static constexpr Indicator FC_EIGHT = '8';
  static constexpr Indicator FC_NINE = '9';
  static constexpr Indicator FC_YES = 'Y';
  static constexpr Indicator FC_NO = 'N';
  static constexpr Indicator FC_TOP = 'T';
  static constexpr Indicator FC_BOTTOM = 'B';
  static constexpr Indicator FC_SPACE = ' ';

  // FareCalcLine length
  static const unsigned FCL_MAX_LINE_LENGTH = 63;

  // Ticket stock constants:
  static const int16_t WORLD_PRICING_TICKET_STOCK_NUMBER = 206;
  static const int16_t BSP_TICKET_STOCK_NUMBER = 202;
  static const int16_t BSP_MIN_TICKET_STOCK_NUMBER = 64;
  static const int16_t BSP_MAX_TICKET_STOCK_NUMBER = 121;
  static const int16_t ATB_MIN_TICKET_STOCK_NUMBER = 45;
  static const int16_t ATB_MAX_TICKET_STOCK_NUMBER = 49;

  static const size_t MAX_FARE_BASIS_SIZE = 15;
  static const size_t MAX_FARE_BASIS_SIZE_WQ_WPA = 10;

  static const std::string SERVICE_FEE_AMOUNT_LINE;
  static const std::string SERVICE_FEE_TOTAL_LINE;
};

} // namespace tse
