//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include <string>

namespace tse
{

class ShoppingResponseNames
{
public:
  static const std::string SHOPPING_RESPONSE_TAG;
  static const std::string TOKEN_ATTR;
  static const std::string Q0S_ATTR;
  static const std::string Q0F_ATTR;

  static const std::string DIAGNOSTIC_RESPONSE_TAG;
  static const std::string DIAGNOSTIC_CODE_ATTR;

  static const std::string ITINERARY_TAG;
  static const std::string NUMBER_ATTR;
  static const std::string HAS_ANY_BRAND;
  static const std::string LEG_TAG;
  static const std::string LEG_ID_ATTR;
  static const std::string GROUPFARE_TAG;
  static const std::string BRAND_ERROR_ATTR;
  static const std::string PROGRAM_INFO_TAG;
  static const std::string FLEXFARE_ID_ATTR;
  static const std::string FLEXFARE_ERROR_ATTR;
};

} // namespace tse

