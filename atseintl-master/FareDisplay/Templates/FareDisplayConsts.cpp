//----------------------------------------------------------------------------
//
//  File:           FareDisplayConsts.h
//  Created:        7/26/2005
//  Authors:        Mike Carroll
//
//  Description:    General FD constants
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

#include "FareDisplay/Templates/FareDisplayConsts.h"

namespace tse
{
//-----------------------------------------------------------------------------
// Filter constants
//-----------------------------------------------------------------------------
const std::string CAT_PASSED = "P";
const std::string CAT_FAILED = "F";
const std::string EMPTY_PAX_TYPE = "***";
const std::string DASH = "-";
const std::string SLASH = "/";
const std::string PERIOD = ".";
const std::string SPACE = " ";
const std::string INFINITY_TXT = "INFIN";
const std::string ADV_PUR_TKT_UNITS_MONTH = "M";
const std::string ADV_PUR_TKT_UNITS_DAY = "D";
const std::string ADV_PUR_TKT_UNITS_HOUR = "H";
const std::string ADV_PUR_TKT_3BLANKSPACE = "   ";
const std::string ADV_PUR_TKT_2BLANKSPACE = "  ";
const std::string ADV_PUR_TKT_SIML = "SIML ";
const std::string ADV_PUR_TKT_DEFAULT = "///";
const std::string BLANK_DASH = " -";
const std::string ADV_PUR_TKT_NP = " NP";
const std::string ASTERISK = "*";
const std::string INCOMPLETE = "I";
const std::string UNAVAILABLE = "U";

const std::string CONSTRUCTED_FARE = "CNS";
const std::string INDUSTRY_FARE = "IND";
const std::string FARE_BY_RULE_FARE = "C25";
const std::string DISCOUNTED_FARE = "DIS";
const std::string PUBLISHED_FARE = "PUB";
const std::string NEGOTIATED = "C35";

const std::string IS_ROUTING = "R";
const std::string IS_NOT_ROUTING = "M";

// Surcharge information for display with FT and FL for online agencies
const std::string SURCHARGE_INFO = "BASE FARE IN FQ CONSISTS OF FILED AMOUNT PLUS ANY SURCHARGES";

// Cat35 specific
const std::string NET_QUALIFIER = "N";
const std::string REDISTRIBUTE_QUALIFIER = "R";
const std::string SELLING_QUALIFIER = "S";
const std::string ORIGINAL_SELLING_QUALIFIER = "O";
const std::string NET_QUALIFIER_WITH_UNIQUE_FBC = "NU";

const std::string AUTO_PRICE_YES = "YES";
const std::string AUTO_PRICE_NOT_SUPPORTED = "NOT SUPPORTED";
const std::string AUTO_PRICE_NO_INHIBITED = "NO/INHIBITED";
const std::string AUTO_PRICE_NO_INCOMPLETE_R3 = "NO/INCOMPLETE R3";
const std::string AUTO_PRICE_NO_UNAVAILABLE_R1 = "NO/UNAVAILABLE R1";
const std::string AUTO_PRICE_NO_UNAVAILABLE_R3 = "NO/UNAVAILABLE R3";
const std::string AUTO_PRICE_NO_UNSUPPORTED_PT = "NO/UNSUPPORTED PT";

const std::string NOT_AUTO_PRICEABLE = "NP";

// Auto pricing
const std::string PRICE_FIELD_SPECIAL = "SPECIAL FARE";
const std::string PRICE_FIELD_NORMAL = "NORMAL FARE";

const std::string ROUND_TRIP = "RT";
const std::string ONE_WAY_TRIP = "OW";

const std::string BLANK_PAX_TYPE = "   ";

//-----------------------------------------------------------------------------
// Fail code constants
//-----------------------------------------------------------------------------
const std::string FC_FARE_VALID = "0";
const std::string FC_ACTUAL_PAX_TYPE_NULL = "1";
const std::string FC_FARE_MARKET_NULL = "2";
const std::string FC_EMPTY_FARE = "3";
const std::string FC_EMPTY_FARE_CLASS_APP_INFO = "4";
const std::string FC_EMPTY_FARE_CLASS_APP_SEG_INFO = "5";
const std::string FC_NOT_ALL_CATEGORIES_VALID = "6";
const std::string FC_BOOKING_CODE_STATUS_FAILURE = "7";
const std::string FC_ROUTING_FAILURE = "8";
const std::string FC_FARE_IS_INVALID = "9";
const std::string FC_FAILED_FARE_GROUP = "10";
const std::string FC_WEB_INVALID = "11";
const std::string FC_INCLUSION_CODE_FAIL = "12";
const std::string FC_OUTBOUND_CURRENCY_FAIL = "13";
const std::string FC_FARE_NOT_SELECTED_FOR_RD = "14";
const std::string FC_FARECLASS_APP_TAG_UNAVAILABLE = "15";
const std::string FC_NOT_VALID_FOR_DIRECTION = "16";
const std::string FC_FAILED_FARE_CLASS_MATCH = "17";
const std::string FC_FAILED_TKTDES_MATCH = "18";
const std::string FC_FAILED_BOOKING_CODE_MATCH = "19";
const std::string FC_FAILED_PAX_TYPE_MATCH = "20";
const std::string FC_FAILED_DATE_CHECK = "21";
const std::string FC_NOT_PRIVATE_FARE = "22";
const std::string FC_NOT_PUBLIC_FARE = "23";
const std::string FC_INDUSTRY_FARE_FILTERED_OUT = "24";
const std::string FC_BASE_FARE_EXCEPTION = "25";
const std::string FC_CAT_35_INVALID_AGENCY = "26";
const std::string FC_OWRT_DISPLAY_TYPE = "27";
const std::string FC_CAT_35_INCOMPLETE_DATA = "28";
const std::string FC_CAT_35_VIEWNETINDICATOR = "29";
const std::string FC_CAT_35_FAILED_RULE = "30";
const std::string FC_CAT_25_FAILED_RULE = "31";
const std::string FC_MISSING_FOOTNOTE_DATA = "32";
const std::string FC_ZZ_GLOBAL_DIR = "33";
const std::string FC_DUPE_FARE = "34";
const std::string FC_MERGED_FARE = "35";
const std::string FC_INVALID_FOR_CWT = "36";
const std::string FC_INVALID_FARE_CURRENCY = "37";
const std::string FC_MATCHED_FARE_FOCUS_RULE = "38";
const std::string FC_NOT_MATCHED_FARE_RULE_TARIFF = "39";
const std::string FC_NOT_MATCHED_FARE_RULE_NUMBER = "40";
const std::string FC_NOT_MATCHED_FARE_TYPE_CODE = "41";
const std::string FC_NOT_MATCHED_FARE_DISPLAY_TYPE = "42";
const std::string FC_NOT_MATCHED_FARE_PRIVATE_INDICATOR = "43";
const std::string FC_NOT_MATCHED_FARE_NEGOTIATED_SELECT = "44";
const std::string FC_NOT_MATCHED_FARE_RULE_SELECT = "45";
const std::string FC_NOT_MATCHED_FARE_SALE_RESTRICT = "46";

//-----------------------------------------------------------------------------
// Rule category constants
//-----------------------------------------------------------------------------

const std::string INTL_CONST_CODE = "IC";
const std::string RETAILER_CODE = "RR";

const std::string COMBINABILITY_CODE_OW = "OW";
const std::string COMBINABILITY_CODE_OJ = "OJ";
const std::string COMBINABILITY_CODE_RT = "RT";
const std::string COMBINABILITY_CODE_CT = "CT";
const std::string COMBINABILITY_CODE_EE = "EE";
const std::string COMBINABILITY_CODE_IA = "IA";
} // namespace tse
