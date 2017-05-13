//----------------------------------------------------------------------------
//
//  File:           TemplateEnums.h
//  Created:        7/26/2005
//  Authors:        Mike Carroll
//
//  Description:    Template enumerations
//
//  Updates:
//
//  Copyright Sabre 2005
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

namespace tse
{

//-----------------------------------------------------------------------------
// Known diagnostic common fare field elements
//-----------------------------------------------------------------------------
enum TemplateType
{
  SINGLE_CARRIER = 'S',
  MULTI_CARRIER = 'M',
  ADDON = 'A',
  TAX = 'T'
};

//-----------------------------------------------------------------------------
// Known diagnostic common fare field elements
//-----------------------------------------------------------------------------
enum DiagFieldElement
{
  UNKNOWN_DIAG_ELEMENT = 0,
  LINE_NUMER = 1,
  PAX_TYPE = 2,
  FARE_CLASS = 3,
  SURCHARGES = 4,
  FARE_BASIS_TKT_DSG = 5,
  BASE_FARE = 6,
  US_TAX = 7,
  TOTAL_FARE = 8,
  JOURNEY_TYPE = 9,
  FARE_TYPE_CODE = 10,
  NET_IND = 11,
  DISPLAY_TYPE = 12,
  CURRENCY_CODE = 13,
  CONVERTED_FARE = 14,
  DISPLAY_CURRENCY_CODE = 15,
  FARE_CURRENCY_CODE = 16,
  FAIL_CODE = 17,
  ORIGINAL_FARE = 18,
  CAT_BASE = 100,
  CAT_1_IND = 101,
  CAT_2_IND = 102,
  CAT_3_IND = 103,
  CAT_5_IND = 105,
  CAT_6_IND = 106,
  CAT_7_IND = 107,
  CAT_11_IND = 111,
  CAT_14_IND = 114,
  CAT_15_IND = 115,
  CAT_16_IND = 116,
  CAT_23_IND = 123
};

//-----------------------------------------------------------------------------
// Field justification
//-----------------------------------------------------------------------------
enum JustificationType
{
  LEFT = 1,
  RIGHT = 2,
  LOGIC = 3
};

//-----------------------------------------------------------------------------
// Field value types
//-----------------------------------------------------------------------------
enum FieldValueType
{
  BOOL_VALUE = 0,
  INT_VALUE = 1,
  STRING_VALUE = 2,
  MONEY_AMOUNT_VALUE = 3
};

} // namespace tse

