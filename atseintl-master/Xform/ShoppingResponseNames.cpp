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

#include "Xform/ShoppingResponseNames.h"

namespace tse
{

const std::string ShoppingResponseNames::SHOPPING_RESPONSE_TAG = "ShoppingResponse";
const std::string ShoppingResponseNames::TOKEN_ATTR = "STK";
const std::string ShoppingResponseNames::Q0S_ATTR = "Q0S";
const std::string ShoppingResponseNames::Q0F_ATTR = "Q0F";

const std::string ShoppingResponseNames::DIAGNOSTIC_RESPONSE_TAG = "DIA";
const std::string ShoppingResponseNames::DIAGNOSTIC_CODE_ATTR = "Q0A";

const std::string ShoppingResponseNames::ITINERARY_TAG = "ITN";
const std::string ShoppingResponseNames::NUMBER_ATTR = "NUM";
const std::string ShoppingResponseNames::HAS_ANY_BRAND = "ANY";
const std::string ShoppingResponseNames::LEG_TAG = "LEG";
const std::string ShoppingResponseNames::LEG_ID_ATTR = "Q14";
const std::string ShoppingResponseNames::GROUPFARE_TAG = "GRI";
const std::string ShoppingResponseNames::BRAND_ERROR_ATTR = "SBL";
const std::string ShoppingResponseNames::PROGRAM_INFO_TAG = "PRG";
const std::string ShoppingResponseNames::FLEXFARE_ID_ATTR = "Q17";
const std::string ShoppingResponseNames::FLEXFARE_ERROR_ATTR = "SGL";

} // namespace tse
