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
#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

#include <cstdint>
#include <string>

namespace tse
{

// Known Documents
//-----------------------------------------------------------------------------
// Known non-diagnostic request documents
//-----------------------------------------------------------------------------
constexpr int16_t FQ_DOCUMENT_ID = 1;
constexpr int16_t RD_DOCUMENT_ID = 2;
constexpr int16_t FT_DOCUMENT_ID = 3;
constexpr int16_t MP_DOCUMENT_ID = 4;
constexpr int16_t TX_DOCUMENT_ID = 5;
constexpr int16_t PX_DOCUMENT_ID = 6;
constexpr int16_t DEFAULT_ADDON_TEMPLATE = 18;
constexpr int16_t DEFAULT_TEMPLATE = 1;

//-----------------------------------------------------------------------------
// Known diagnostics
//-----------------------------------------------------------------------------
constexpr int16_t DIAG_195_ID = 195; // External services XML Request/Response display
constexpr int16_t DIAG_200_ID = 200;
constexpr int16_t DIAG_201_ID = 201;
constexpr int16_t DIAG_202_ID = 202;
constexpr int16_t DIAG_203_ID = 203;
constexpr int16_t DIAG_204_ID = 204;
constexpr int16_t DIAG_205_ID = 205;
constexpr int16_t DIAG_206_ID = 206;
constexpr int16_t DIAG_207_ID = 207; // inclusion code details without record 8 passenger types.
constexpr int16_t DIAG_209_ID = 209; // inclusion code details with record 8 passenger types.
constexpr int16_t DIAG_211_ID = 211; // Header messages
constexpr int16_t DIAG_212_ID = 212; // Suppression
constexpr int16_t DIAG_213_ID = 213; // Grouping Preference
constexpr int16_t DIAG_214_ID = 214; // Bypass Grouping
constexpr int16_t DIAG_215_ID = 215; // By FBR Routing
constexpr int16_t DIAG_216_ID = 216; // User Preferences
constexpr int16_t DIAG_217_ID = 217; // Multi-Transport  Grouping
constexpr int16_t DIAG_218_ID = 218; // Actual Pax Type
constexpr int16_t DIAG_220_ID = 220;
constexpr int16_t DIAG_291_ID = 291; // RD FareBasisNotFound

constexpr int16_t DIAG_854_ID = 854; // Host and DB info
constexpr int16_t DIAG_999_ID = 999; // Diag Help

constexpr int32_t MAX_DIAG_FARES = 2000;
constexpr int16_t MAX_PSS_LINE_SIZE = 64;
constexpr int16_t MIN_DIAG_NBR = 100; // Diagnostic lower bound number

//-----------------------------------------------------------------------------
// Filter constants
//-----------------------------------------------------------------------------
extern const std::string CAT_PASSED;
extern const std::string CAT_FAILED;
extern const std::string EMPTY_PAX_TYPE;
extern const std::string DASH;
extern const std::string SLASH;
extern const std::string PERIOD;
extern const std::string SPACE;
extern const std::string INFINITY_TXT;
extern const std::string ADV_PUR_TKT_UNITS_MONTH;
extern const std::string ADV_PUR_TKT_UNITS_DAY;
extern const std::string ADV_PUR_TKT_UNITS_HOUR;
extern const std::string ADV_PUR_TKT_3BLANKSPACE;
extern const std::string ADV_PUR_TKT_2BLANKSPACE;
extern const std::string ADV_PUR_TKT_SIML;
extern const std::string ADV_PUR_TKT_DEFAULT;
extern const std::string BLANK_DASH;
extern const std::string ADV_PUR_TKT_NP;
extern const std::string ASTERISK;
extern const std::string INCOMPLETE;
extern const std::string UNAVAILABLE;

constexpr char CROSS_LORRAINE = '$';
constexpr char CHAR_SLASH = '/';
constexpr char CHAR_DASH = '-';
constexpr char CHAR_BLANK = ' ';
constexpr char CHAR_ZERO = '0';
constexpr char CHAR_ONE = '1';
constexpr char CHAR_TWO = '2';
constexpr char CHAR_FOUR = '4';
constexpr char CHAR_H = 'H';
constexpr char CHAR_M = 'M';
constexpr char CHAR_S = 'S';

extern const std::string CONSTRUCTED_FARE;
extern const std::string INDUSTRY_FARE;
extern const std::string FARE_BY_RULE_FARE;
extern const std::string DISCOUNTED_FARE;
extern const std::string PUBLISHED_FARE;
extern const std::string NEGOTIATED;

extern const std::string IS_ROUTING;
extern const std::string IS_NOT_ROUTING;

// Surcharge information for display with FT and FL for online agencies
extern const std::string SURCHARGE_INFO;

// Cat35 specific
extern const std::string NET_QUALIFIER;
extern const std::string REDISTRIBUTE_QUALIFIER;
extern const std::string SELLING_QUALIFIER;
extern const std::string ORIGINAL_SELLING_QUALIFIER;
extern const std::string NET_QUALIFIER_WITH_UNIQUE_FBC;

extern const std::string AUTO_PRICE_YES;
extern const std::string AUTO_PRICE_NOT_SUPPORTED;
extern const std::string AUTO_PRICE_NO_INHIBITED;
extern const std::string AUTO_PRICE_NO_INCOMPLETE_R3;
extern const std::string AUTO_PRICE_NO_UNAVAILABLE_R1;
extern const std::string AUTO_PRICE_NO_UNAVAILABLE_R3;
extern const std::string AUTO_PRICE_NO_UNSUPPORTED_PT;

extern const std::string NOT_AUTO_PRICEABLE;

// Auto pricing
constexpr char PRICE_FIELD_N = 'N';
extern const std::string PRICE_FIELD_SPECIAL;
extern const std::string PRICE_FIELD_NORMAL;

extern const std::string ROUND_TRIP;
extern const std::string ONE_WAY_TRIP;

extern const std::string BLANK_PAX_TYPE;

constexpr char TRUE_INDICATOR = 'T';
constexpr char FALSE_INDICATOR = 'F';

constexpr char YES_INDICATOR = 'Y';
constexpr char NO_INDICATOR = 'N';

//-----------------------------------------------------------------------------
// Fail code constants
//-----------------------------------------------------------------------------
extern const std::string FC_FARE_VALID;
extern const std::string FC_ACTUAL_PAX_TYPE_NULL;
extern const std::string FC_FARE_MARKET_NULL;
extern const std::string FC_EMPTY_FARE;
extern const std::string FC_EMPTY_FARE_CLASS_APP_INFO;
extern const std::string FC_EMPTY_FARE_CLASS_APP_SEG_INFO;
extern const std::string FC_NOT_ALL_CATEGORIES_VALID;
extern const std::string FC_BOOKING_CODE_STATUS_FAILURE;
extern const std::string FC_ROUTING_FAILURE;
extern const std::string FC_FARE_IS_INVALID;
extern const std::string FC_FAILED_FARE_GROUP;
extern const std::string FC_WEB_INVALID;
extern const std::string FC_INCLUSION_CODE_FAIL;
extern const std::string FC_OUTBOUND_CURRENCY_FAIL;
extern const std::string FC_FARE_NOT_SELECTED_FOR_RD;
extern const std::string FC_FARECLASS_APP_TAG_UNAVAILABLE;
extern const std::string FC_NOT_VALID_FOR_DIRECTION;
extern const std::string FC_FAILED_FARE_CLASS_MATCH;
extern const std::string FC_FAILED_TKTDES_MATCH;
extern const std::string FC_FAILED_BOOKING_CODE_MATCH;
extern const std::string FC_FAILED_PAX_TYPE_MATCH;
extern const std::string FC_FAILED_DATE_CHECK;
extern const std::string FC_NOT_PRIVATE_FARE;
extern const std::string FC_NOT_PUBLIC_FARE;
extern const std::string FC_INDUSTRY_FARE_FILTERED_OUT;
extern const std::string FC_BASE_FARE_EXCEPTION;
extern const std::string FC_CAT_35_INVALID_AGENCY;
extern const std::string FC_OWRT_DISPLAY_TYPE;
extern const std::string FC_CAT_35_INCOMPLETE_DATA;
extern const std::string FC_CAT_35_VIEWNETINDICATOR;
extern const std::string FC_CAT_35_FAILED_RULE;
extern const std::string FC_CAT_25_FAILED_RULE;
extern const std::string FC_MISSING_FOOTNOTE_DATA;
extern const std::string FC_ZZ_GLOBAL_DIR;
extern const std::string FC_DUPE_FARE;
extern const std::string FC_MERGED_FARE;
extern const std::string FC_INVALID_FOR_CWT;
extern const std::string FC_INVALID_FARE_CURRENCY;
extern const std::string FC_MATCHED_FARE_FOCUS_RULE;
extern const std::string FC_NOT_MATCHED_FARE_RULE_TARIFF;
extern const std::string FC_NOT_MATCHED_FARE_RULE_NUMBER;
extern const std::string FC_NOT_MATCHED_FARE_TYPE_CODE;
extern const std::string FC_NOT_MATCHED_FARE_DISPLAY_TYPE;
extern const std::string FC_NOT_MATCHED_FARE_PRIVATE_INDICATOR;
extern const std::string FC_NOT_MATCHED_FARE_NEGOTIATED_SELECT;
extern const std::string FC_NOT_MATCHED_FARE_RULE_SELECT;
extern const std::string FC_NOT_MATCHED_FARE_SALE_RESTRICT;

//-----------------------------------------------------------------------------
// Rule category constants
//-----------------------------------------------------------------------------

extern const std::string INTL_CONST_CODE;
extern const std::string RETAILER_CODE;
constexpr Indicator IND_YES = 'T';

constexpr int16_t NUM_NC_CATEGORIES = 1;
constexpr int16_t NC_CATEGORIES[NUM_NC_CATEGORIES] = {10};
constexpr CatNumber SEASONS_RULE_CATEGORY = 3;
constexpr CatNumber ADVANCEPURCHASE_RULE_CATEGORY = 5;
constexpr CatNumber MINSTAY_RULE_CATEGORY = 6;
constexpr CatNumber MAXSTAY_RULE_CATEGORY = 7;
constexpr CatNumber TRANSFERS_RULE_CATEGORY = 9;
constexpr CatNumber COMBINABILITY_RULE_CATEGORY = 10;
constexpr CatNumber TRAVEL_RULE_CATEGORY = 14;
constexpr CatNumber TICKET_RULE_CATEGORY = 15;
constexpr CatNumber PENALTY_RULE_CATEGORY = 16;
constexpr CatNumber VOLUNTARY_CHANGES = 31;
constexpr CatNumber VOLUNTARY_REFUNDS = 33;
constexpr CatNumber RETAILER_CATEGORY = 90;
constexpr CatNumber IC_RULE_CATEGORY = 99;

extern const std::string COMBINABILITY_CODE_OW;
extern const std::string COMBINABILITY_CODE_OJ;
extern const std::string COMBINABILITY_CODE_RT;
extern const std::string COMBINABILITY_CODE_CT;
extern const std::string COMBINABILITY_CODE_EE;
extern const std::string COMBINABILITY_CODE_IA;

} // namespace tse

