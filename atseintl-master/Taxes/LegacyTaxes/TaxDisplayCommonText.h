//----------------------------------------------------------------------------
//  File:           CommonTaxText.h
//  Description:    CommonTaxText header file for ATSE International Project
//  Created:        4/02/2008
//  Authors:        Piotr Lach
//
//  Description: This Object will be used for Tax Display functionality.
//
//  Updates:
//
//  Copyright Sabre 2007
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

#ifndef TAX_DISPLAY_COMMON_TEXT_H
#define TAX_DISPLAY_COMMON_TEXT_H

#include "DBAccess/TaxCodeReg.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include <string>

namespace tse
{

class TaxDisplayCommonText
{

public:
  static constexpr Indicator TAX_TITLE = '1'; // Title Area
  static constexpr Indicator TAX_CODE = '2'; // Category0
  static constexpr Indicator TAX = '3'; // Category1
  static constexpr Indicator TAX_DETAIL = '4'; // Category2
  static constexpr Indicator SALE = '5'; // Category3
  static constexpr Indicator TRAVEL = '6'; // Category4
  static constexpr Indicator VALIDATING_CARRIER = '7'; // Category5
  static constexpr Indicator CURRENCY = '8'; // Category6
  static constexpr Indicator AIRLINE = '9'; // Category7
  static constexpr Indicator EQUIPMENT = 'A'; // Category8
  static constexpr Indicator PASSEMGER_TYPE = 'B'; // Category9
  static constexpr Indicator FARE_TYPE = 'C'; // Category10
  static constexpr Indicator TRANSIT = 'D'; // Category11
  static constexpr Indicator TICKET_DESIGNATOR = 'E'; // Category12
  static constexpr Indicator CABIN = 'F'; // Category13
  static constexpr Indicator DISCOUNT = 'G'; // Category14
  static constexpr Indicator MISCELLANEOUS = 'H'; // Category15
  static constexpr Indicator REISSUE = 'I'; // Category16
  static constexpr Indicator ROUTING = 'J'; // Category17
  static constexpr Indicator REFUND = 'K'; // Category18

  TaxDisplayCommonText();
  virtual ~TaxDisplayCommonText();

  static std::string getCommonText(TaxCodeReg& taxCodeReg, Indicator msgId);

private:
  TaxDisplayCommonText(const TaxDisplayCommonText& item);
  TaxDisplayCommonText& operator=(const TaxDisplayCommonText& item);
};
} // namespace tse

#endif // TAX_DISPLAY_COMMON_TEXT_H
