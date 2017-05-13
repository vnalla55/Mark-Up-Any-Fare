#pragma once

#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

#include <set>

namespace tse
{
namespace RuleConst
{
//  ATPCO CATEGORY RULES
constexpr uint16_t DUMMY_RULE = 0;
constexpr uint16_t ELIGIBILITY_RULE = 1;
constexpr uint16_t DAY_TIME_RULE = 2;
constexpr uint16_t SEASONAL_RULE = 3;
constexpr uint16_t FLIGHT_APPLICATION_RULE = 4;
constexpr uint16_t ADVANCE_RESERVATION_RULE = 5;
constexpr uint16_t MINIMUM_STAY_RULE = 6;
constexpr uint16_t MAXIMUM_STAY_RULE = 7;
constexpr uint16_t STOPOVER_RULE = 8;
constexpr uint16_t TRANSFER_RULE = 9;
constexpr uint16_t COMBINABILITY_RULE = 10;
constexpr uint16_t BLACKOUTS_RULE = 11;
constexpr uint16_t SURCHARGE_RULE = 12;
constexpr uint16_t ACCOMPANIED_PSG_RULE = 13;
constexpr uint16_t TRAVEL_RESTRICTIONS_RULE = 14;
constexpr uint16_t SALE_RESTRICTIONS_RULE = 15;
constexpr uint16_t PENALTIES_RULE = 16;
constexpr uint16_t HIP_RULE = 17;
constexpr uint16_t TICKET_ENDORSMENT_RULE = 18;
constexpr uint16_t CHILDREN_DISCOUNT_RULE = 19;
constexpr uint16_t TOUR_DISCOUNT_RULE = 20;
constexpr uint16_t AGENTS_DISCOUNT_RULE = 21;
constexpr uint16_t OTHER_DISCOUNT_RULE = 22;
constexpr uint16_t MISC_FARE_TAG = 23;
constexpr uint16_t FARE_BY_RULE = 25;
constexpr uint16_t TOURS_RULE = 27;
constexpr uint16_t VOLUNTARY_EXCHANGE_RULE = 31;
constexpr uint16_t VOLUNTARY_REFUNDS_RULE = 33;
constexpr uint16_t NEGOTIATED_RULE = 35;
constexpr uint16_t INVALID_SHOPPING_FLIGHT_BITMAP = 50;
constexpr uint16_t FLIGHT_EXCEEDS_CONXN_POINT_LIMIT = 51;
constexpr uint16_t MAXIMUM_RULE_CATEGORY_COUNT = 32;

constexpr int16_t MAX_NUMBER_XX = 33;
constexpr TariffCategory PUBLIC_TARIFF = 0;
constexpr TariffCategory PRIVATE_TARIFF = 1;
constexpr char PUBLIC_VENDOR = 'P'; // ATP, SITA
constexpr char SMF_VENDOR = 'T'; // Sabre MyFare vendor
constexpr char CARRIER_VENDOR = 'C'; // carrier vendor (APO, Sabre Sonic)

constexpr char STRING_DOES_NOT_APPLY = 'X';
constexpr char RECORD_0_NOT_APPLY = 'N';
constexpr char NCROUTING_MATCH_CHAR = 'S';
constexpr char MPM_ROUTING_MATCH_CHAR = 'M'; // Any MPM routing
constexpr char ANY_SPECIFIED_ROUTING_MATCH_CHAR = 'R'; // Any specified routing
constexpr char DOLLAR_SIGN = '$';
constexpr char ANY_LOCATION_TYPE = ' ';

constexpr char FARE_MARKET_OUTBOUND = 'O';
constexpr char FARE_MARKET_INBOUND = 'I';
constexpr char ALWAYS_APPLIES = ' ';
constexpr char FROM_LOC1_TO_LOC2 = '1';
constexpr char TO_LOC1_FROM_LOC2 = '2';
constexpr char ORIGIN_FROM_LOC1_TO_LOC2 = '3';
constexpr char ORIGIN_FROM_LOC2_TO_LOC1 = '4';

extern const RuleNumber NULL_GENERAL_RULE;
extern const Zone NOT_APPLICABLE_ZONE;

extern const std::set<uint16_t> CATEGORY_NOT_FOOTNOTE;
extern const std::set<uint16_t> CATEGORY_GRULE_PROCESS;

// Defined Constants for the Web Fare Processing
enum WebAgentLocation
{ TRAVELOCITY,
  NONTRAVELOCITY };

extern const std::set<LocCode> WEB_AGENCIES;
extern const std::set<LocCode> NON_TVL_WEB_AGENCIES;

bool
isWebAgencyPresent(const LocCode& agency, WebAgentLocation agentLocation);

// Defined Constants for the Discounted fare processing
constexpr Indicator CALCULATED = 'C';
constexpr Indicator SPECIFIED = 'S';
constexpr Indicator CREATE_RT_FROM_OW = 'D';
constexpr Indicator SELECT_HIGHEST = 'H';
constexpr Indicator SELECT_LOWEST = 'L';
constexpr Indicator ADD_SPECIFIED_TO_CALCULATED = 'A';
constexpr Indicator SUBTRACT_SPECIFIED_FROM_CALCULATED = 'M';
constexpr Indicator NO_CURRENCY = '*';
constexpr Percent HUNDRED_PERCENTS = 100.00;

// Defined Constants for the FlightApplication Cat4:
constexpr int ANY_FLIGHT = -1;

// Defined Constants for the SalesRestriction Cat15:
constexpr Indicator NOT_ALLOWED = 'N';
constexpr Indicator REQUIRED = 'R';
constexpr Indicator NO_SEGMENT_OF_TICKET = 'N';
constexpr Indicator NO_SEGMENT_AT_THIS_FARE = 'F';
constexpr Indicator COUNTRY_OF_ORIGIN = 'O';
constexpr Indicator COUNTRY_OF_DESTINATION = 'D';
constexpr Indicator COUNTRY_OF_ORI_AND_DEST = 'B';
constexpr Indicator CTR_CURR_OF_ORI_AND_DEST = 'E';
constexpr Indicator FROMTO = 'F';
constexpr Indicator TOFROM = 'T';
constexpr Indicator MAY_NOT_BE_SOLD_BY_TA = 'N';
constexpr Indicator MAY_ONLY_BE_SOLD_BY_TA = 'Y';
constexpr Indicator MUST_BE_SOLD_VIA_CARRIER = 'X';
constexpr Indicator MUST_BE_SOLD_VIA_CRS = 'C';
constexpr Indicator TKTS_MAY_ONLY_BE_SOLD = 'Y';
constexpr Indicator TKTS_MAY_NOT_BE_SOLD = 'N';
constexpr Indicator TKTS_MAY_ONLY_BE_ISSUED = 'T';
constexpr Indicator TKTS_MAY_NOT_BE_ISSUED = 'X';
constexpr Indicator VALIDATIING_CXR_RESTR_SALE_BASED = 'S';
constexpr Indicator VALIDATING_CXR_RESTR_EXCLUDE_PUBLISHING = 'O';
extern const LocCode DFW_CITY;
extern const LocCode HDQ_CITY;

// also used by Cat35
constexpr Indicator HOME_IATA_TVL_AGENCY_NO = 'H';
constexpr Indicator IATA_TVL_AGENCY_NO = 'I';
constexpr Indicator TRAVEL_AGENCY = 'T';
constexpr Indicator HOME_TRAVEL_AGENCY = 'U';
constexpr Indicator FUNCTIONAL_CODE = 'V';
constexpr Indicator DUTY_CODE = 'X';
constexpr Indicator CRS_CRX_DEPT_CODE = 'V';

extern const CarrierCode SABRE1B;
extern const CarrierCode SABRE1J;
extern const CarrierCode SABRE1W;
extern const CarrierCode SABRE1F;
extern const CarrierCode SABRE1S;
extern const CarrierCode JOINT_CARRIER;
extern const PseudoCityCode WEB_FARE_PCC;
extern const PaxTypeCode MATCH_NEG_PAX_TYPE;

constexpr Percent PERCENT_NO_APPL = 999.9999;

// Rec1 Cat25 consts (also used by Cat35)
constexpr Indicator SELLING_FARE = 'L';
constexpr Indicator NET_SUBMIT_FARE = 'T';
constexpr Indicator NET_SUBMIT_FARE_UPD = 'C';

// Commission indicator for cat35 (when 'L' with table 979)
constexpr Indicator SELLING_FARE_NOT_FOR_SEC = 'N';

// Fare Ind for Cat35
constexpr Indicator NF_CALC_PERCENT = 'C';
constexpr Indicator NF_SPECIFIED = 'S';
constexpr Indicator NF_ADD = 'A';
constexpr Indicator NF_MINUS = 'M';
constexpr Indicator NF_RANGE_PERCENT = 'P';
constexpr Indicator NF_RANGE_SPECIFIED = 'R';
constexpr Indicator NF_ADD_RANGE = 'N';
constexpr Indicator NF_MINUS_RANGE = 'T';
constexpr Indicator NF_NO_CALC_DATA = 'Z'; /* not ATPCO, internal flag*/

// FARE DISPLAY indicators for Cat 35
constexpr Indicator NET_FARE = 'N';
constexpr Indicator SELLING_CARRIER_FARE = 'S';
constexpr Indicator SELLING_MARKUP_FARE = 'M';
constexpr Indicator REDISTRIBUTED_FARE = 'R';

// METHOD TYPE for NET REMIT Cat 35
constexpr Indicator NRR_METHOD_1 = '1';
constexpr Indicator NRR_METHOD_2 = '2'; // Net Submit Fare Amount must be.
constexpr Indicator NRR_METHOD_3 = '3';
constexpr Indicator NRR_METHOD_4 = '4';
constexpr Indicator NRR_METHOD_5 = '5';
constexpr Indicator NRR_METHOD_BLANK = ' '; // Net Remit Reportiong is not being used

// Ticketed Fare Data Indicator for byte 101 for Net Remit Cat35
constexpr Indicator NR_VALUE_F = 'F';
constexpr Indicator NR_VALUE_A = 'A';
constexpr Indicator NR_VALUE_B = 'B';
constexpr Indicator NR_VALUE_N = '*';

// NET GROSS indicator for Cat 35
constexpr Indicator NGI_NET_AMOUNT = 'N';
constexpr Indicator NGI_GROSS_AMOUNT = 'G';

// Tour Code/Value Code Display Option for Cat35
constexpr Indicator DISPLAY_OPTION_ALL = 'B';
constexpr Indicator DISPLAY_OPTION_1ST = '1';
constexpr Indicator DISPLAY_OPTION_2ND = '2';
constexpr Indicator DISPLAY_OPTION_BLANK = ' ';

// Tour Code/Value Code Print Option for Cat35
constexpr Indicator PRINT_OPTION_1 = '1';
constexpr Indicator PRINT_OPTION_2 = '2';
constexpr Indicator PRINT_OPTION_3 = '3';
constexpr Indicator PRINT_OPTION_4 = '4';

// Defined Constants for the Surcharge Cat12:
constexpr Indicator DATA_UNAVAILABLE = 'X';
constexpr Indicator TEXT_ONLY = 'Y';

constexpr Indicator BLANK = ' ';
constexpr Indicator AIRPORT = 'A';
constexpr Indicator BUSINESCLASS = 'B';
constexpr Indicator SUPERSONIC = 'C';
constexpr Indicator PEAKTRAVEL = 'D';
constexpr Indicator EQUIPMENT = 'E';
constexpr Indicator FUEL = 'F';
constexpr Indicator PEAK = 'G';
constexpr Indicator HOLIDAY = 'H';
constexpr Indicator SIDETRIP = 'I';
constexpr Indicator SEASONAL = 'J';
constexpr Indicator WEEKEND = 'K';
constexpr Indicator SLEEPERETTE = 'L';
constexpr Indicator WAIVERADVRES = 'M';
constexpr Indicator NAVIGATION = 'N';
constexpr Indicator SECURITY = 'O';
constexpr Indicator WAIVERMAXSTAY = 'P';
constexpr Indicator SURFACE = 'Y';
constexpr Indicator RBD = 'Z';
constexpr Indicator OTHER = 'Q';

//  Defined Constants for the start/stop Time Field
constexpr Indicator DAYLY = 'D';
constexpr Indicator RANGE = 'R';
constexpr Indicator FARE = 'F';

// Defined Constants for the Equipment type:
extern const CarrierCode AA_CARRIER;
extern const EquipmentType M80_EQUIPMENT_TYPE; // M80
extern const EquipmentType S80_EQUIPMENT_TYPE; // S80

// Defined Constants for the Travel Portion Field:
constexpr Indicator ONEWAY = '1';
constexpr Indicator ROUNDTRIP = '2';
constexpr Indicator PERTRANSFER = '3';
constexpr Indicator PERTICKET = '4';
constexpr Indicator PERCOUPON = '5';
constexpr Indicator PERDIRECTION = '6';

// Defined Constants for the Application field (vendor=ATP)
constexpr char ANY_PAX_SURCHARGE = ' ';
constexpr char CHILD_SURCHARGE = '1';
constexpr char ADT_CHILD_INFANT_SURCHARGE = '2';
constexpr char ADT_CHILD_DISC_INFANT_DISC_SURCHARGE = '3';
constexpr char ADT_SURCHARGE = '4';

// Defined Constants for the MiscellaneousFareTags Cat23:
constexpr Indicator MUST_BE_USED = 'R';
constexpr Indicator MAY_NOT_BE_USED = 'N';
constexpr Indicator MAY_BE_USED = 'Y';
constexpr Indicator PROPORTIONAL = 'P';

constexpr uint32_t STOPOVER_SEC_DOMESTIC = 4 * SECONDS_PER_HOUR;
constexpr uint32_t STOPOVER_SEC_INTL = 24 * SECONDS_PER_HOUR;

constexpr Indicator STOPOVER_TIME_UNIT_MINUTES = 'N';
constexpr Indicator STOPOVER_TIME_UNIT_HOURS = 'H';
constexpr Indicator STOPOVER_TIME_UNIT_DAYS = 'D';
constexpr Indicator STOPOVER_TIME_UNIT_MONTHS = 'M';
constexpr Indicator STOPOVER_TIME_UNIT_BLANK = ' ';

constexpr Indicator CHARGE_PAX_ANY = ' ';
constexpr Indicator CHARGE_PAX_CHILD = '1';
constexpr Indicator CHARGE_PAX_ADULT_CHILD = '2';
constexpr Indicator CHARGE_PAX_ADULT_CHILD_DISC = '3';
constexpr Indicator CHARGE_PAX_ADULT = '4';
constexpr Indicator CHARGE_PAX_ADULT_CHILD_DISC_INFANT_DISC = '5';
constexpr Indicator CHARGE_PAX_ADULT_CHILD_INFANT_FREE = '6';
constexpr Indicator CHARGE_PAX_ADULT_CHILD_DISC_INFANT_FREE = '7';
constexpr Indicator CHARGE_PAX_INFANT = '8';

//  Defined Constants for the Shopping flightBitmap
constexpr uint8_t ROUTING_FAIL = 'R';
constexpr uint8_t GLOBALDIR_FAIL = 'G';

constexpr uint8_t BOOKINGCODE_FAIL = 'B';
constexpr uint8_t QUALIFYCAT4_FAIL = 'Q';
constexpr uint8_t CAT2_FAIL = '2';
constexpr uint8_t CAT4_FAIL = '4';
constexpr uint8_t CAT9_FAIL = '9';
constexpr uint8_t CAT14_FAIL = 'T';
constexpr uint8_t CAT8_FAIL = '8';
constexpr uint8_t CAT3_FAIL = '3';
constexpr uint8_t CAT5_FAIL = '5';
constexpr uint8_t CAT6_FAIL = '6';
constexpr uint8_t CAT7_FAIL = '7';
constexpr uint8_t CAT11_FAIL = 'O';
constexpr uint8_t CAT15_FAIL = 'X';
constexpr uint8_t SKIP = 'S'; // = 'S'
constexpr uint8_t NO_DATEPAIR_FOUND = 'D';
constexpr uint8_t EFFECTIVE_DATE_FAIL = 'E';
constexpr uint8_t EXPIRED_DATE_FAIL = 'F';
constexpr uint8_t ALL_BITS_FAIL = 'A';
constexpr uint8_t DUMMY_SOP = 'M'; // use by FF/BFF
constexpr uint8_t NOT_APPLICABLE = 'N'; // used by SOL
constexpr uint8_t EXCEED_BIT_LIMIT_FAIL = 'L';
constexpr uint8_t AIRPORT_NOT_APPLICABLE = 'P'; // used by IS/SOL
constexpr uint8_t SPANISH_DISCOUNT_NOT_APPLICABLE = 'U';
constexpr uint8_t DUPLICATED_MULTI_CORPID_FARE = 'C';
// Additional flags used only by ESV
constexpr uint8_t CAT_10_FAIL = 'I';
constexpr uint8_t PASSED = 'J';
constexpr uint8_t CAT_10_RESTRICTIONS = 'H';
constexpr uint8_t GSA_FAIL = 'V'; // used by Validating Carrier and GSA

// -----------------------------
// ---- TSI enums/constants ----
// -----------------------------
constexpr int NOT_APPL_GEOTBL_N0 = 0;

// Match Criteria text descriptions
extern const std::string MATCH_STOP_OVER_DESC;
extern const std::string MATCH_INBOUND_DESC;
extern const std::string MATCH_OUTBOUND_DESC;
extern const std::string MATCH_FURTHEST_DESC;
extern const std::string MATCH_DOMESTIC_DESC;
extern const std::string MATCH_ONE_COUNTRY_DESC;
extern const std::string MATCH_INTERNATIONAL_DESC;
extern const std::string MATCH_GATEWAY_DESC;
extern const std::string MATCH_ORIG_GATEWAY_DESC;
extern const std::string MATCH_DEST_GATEWAY_DESC;
extern const std::string MATCH_TRANS_ATLANTIC_DESC;
extern const std::string MATCH_TRANS_PACIFIC_DESC;
extern const std::string MATCH_TRANS_OCEANIC_DESC;
extern const std::string MATCH_INTERCONTINENTAL_DESC;
extern const std::string MATCH_OVER_WATER_DESC;
extern const std::string MATCH_INTL_DOM_TRANSFER_DESC;

extern const std::string NO_MATCH_TURNAROUND_POINT;
extern const std::string NO_MATCH_LAST_SEGMENT;

// Defined Constant for Fare Display (inhibit indicator):
constexpr Indicator FARE_FOR_DISPLAY_ONLY = 'D';
constexpr Indicator INHIBIT_FAIL = 'F';
constexpr Indicator INHIBIT_IGNORE = 'I';
constexpr Indicator INHIBIT_ASSUMPTION = 'S';
constexpr Indicator FOOT_NOTE = 'N';
constexpr Indicator FARE_RULE = 'F';
constexpr Indicator GENERAL_RULE = 'G';

// Constants for duty / function code validation
constexpr Indicator DB_SABRE_A = 'A';
constexpr Indicator DB_SABRE_B = 'B';
constexpr Indicator DB_SABRE_C = 'C';
constexpr Indicator DB_SABRE_D = 'D';
constexpr Indicator DB_SABRE_E = 'E';

constexpr Indicator DB_SABRE_DOLLAR = '$'; // maps to 'A'
constexpr Indicator DB_SABRE_AMP = '@'; // maps to 'B'
constexpr Indicator DB_SABRE_STAR = '*'; // maps to 'C'
constexpr Indicator DB_SABRE_DASH = '-'; // maps to 'D'
constexpr Indicator DB_SABRE_SLASH = '/'; // maps to 'E'

// Constants indicator values for cat 7
constexpr Indicator RETURN_TRV_COMMENCE_IND = 'C';
constexpr Indicator RETURN_TRV_COMPLETE_IND = 'P';

//
// Geo Required: Identifies if Geo type and Locale are required
//
enum TSIGeoRequired
{
  TSI_GEO_BOTH_REQUIRED = 'R', // Both geo type & locale required
  TSI_GEO_GEOTYPE_REQUIRED = 'T', // Geo type required, locale must
  //  be blank
  TSI_GEO_GET_FROM_ITIN = 'G', // Get locale from itinerary
  TSI_GEO_BLANK = ' ' // Blank
};

//
// Geo Not Type: Identifies any Geo Type that are not allowed
//
enum TSIGeoNotType
{
  TSI_GEO_NOT_TYPE_CITY = 'C', // City is not allowed
  TSI_GEO_NOT_TYPE_BOTH = 'B', // Neither City nor Airport allowed
  TSI_GEO_NOT_TYPE_THREE = 'T', // Three items not allowed... City,
  //  Airport or Zone
  TSI_GEO_NOT_TYPE_BLANK = ' ' // Blank
};

//
// Geo Out: Identifies whether the itinerary should be within the
//           Geo Locales or outside the Geo Locales
//
enum TSIGeoOut
{
  TSI_GEO_OUT_INCLUDE = ' ', // Default is to include
  TSI_GEO_OUT_EXCLUDE = 'E', // Exclude: If the itinerary item is
  //  outside the Geo Locale, then it is
  //  a match
  TSI_GEO_OUT_BLANK = ' ' // Blank. Same as INCLUDE
};

//
// Geo Itin Part: Identifies the piece of itinerary to complete a
//                 geographic locale requirement
//
enum TSIGeoItinPart
{
  TSI_GEO_ITIN_PART_ORIG = 'O', // Use the Origin info
  TSI_GEO_ITIN_PART_DEST = 'D', // Use the Destination info
  TSI_GEO_ITIN_PART_TURNAROUND = 'T', // Use the Turnaround point
  TSI_GEO_ITIN_PART_BLANK = ' ' // Blank
};

//
// Loop Direction: Controls loop direction and fare breaks
//
enum TSILoopDirection
{
  TSI_LOOP_FORWARD = 'F', // Loop forward
  TSI_LOOP_BACKWARD = 'B', // Loop backward
  TSI_LOOP_FORWARD_BREAK_ARRIVAL = 'A', // Loop forward and check
  //  only the first item
  // in each fare component
  TSI_LOOP_BACKWARD_BREAK_DEPART = 'D' // Loop backward and check
  //  only the last item
  // in each fare component
};

//
// Loop Item to Set: Controls which item(s) to mark as matches
//
enum TSILoopItemToSet
{
  TSI_LOOP_SET_PREVIOUS = -1, // Mark previous item only
  TSI_LOOP_SET_CURRENT = 0, // Current item only
  TSI_LOOP_SET_NEXT = 1, // Next item only
  TSI_LOOP_SET_CUR_NEXT = 101, // Current and next items
  TSI_LOOP_SET_CUR_PREV = 99 // Current and previous items
};

//
// Loop Match: Controls when to stop searching for matches
//
enum TSILoopMatch
{
  TSI_MATCH_ALL = 'A', // Check all items
  TSI_MATCH_FIRST = 'F', // Stop after first match
  TSI_MATCH_ONCE = 'O', // Only check one item regardless of
  //  whether a match is found or not
  TSI_MATCH_LAST = 'L', // Only the last match found
  TSI_MATCH_FIRST_ALL = 'I', // Stop on first match for fare
  //  component, all for subjourney
  TSI_MATCH_SOFT_MATCH = 'S', // Stop on first match on criteria,
  //  regardless of whether is matches
  //  any geolocale requirements
  TSI_MATCH_SECOND_FIRST = 'E' // Soft match first two criteria,
  //  then match third criteria
};

//
// Scope Type: Controls the scope type of the TSI
//
enum TSIScopeType
{
  TSI_SCOPE_JOURNEY = 'J', // Journey
  TSI_SCOPE_SUB_JOURNEY = 'S', // Subjourney
  TSI_SCOPE_FARE_COMPONENT = 'F', // Fare Component
  TSI_SCOPE_SJ_AND_FC = 'A' // Subjourney and Fare Component
};

//
// Scope Param Type: Valid values of defaultScope parameter
//                    for scopeTSIGeo method
//
enum TSIScopeParamType
{
  TSI_SCOPE_PARAM_JOURNEY = 'J', // Journey
  TSI_SCOPE_PARAM_SUB_JOURNEY = 'S', // Subjourney
  TSI_SCOPE_PARAM_FARE_COMPONENT = 'F' // Fare Component
};

//
// Application Type: Identifies which piece of the city-pair to check
//
enum TSIApplicationType
{
  TSI_APP_CHECK_ORIG = 'O', // Check the origin
  TSI_APP_CHECK_DEST = 'D', // Check the destination
  TSI_APP_CHECK_ORIG_DEST = 'B' // Check both origin and destination
};

enum TSIMatch
{ TSI_MATCH,
  TSI_NOT_MATCH,
  TSI_SOFT_MATCH };

enum TSITravelSegDirection
{ INBOUND,
  OUTBOUND };

// ------------------------------------
// ---- End of TSI enums/constants ----
// ------------------------------------

enum DOWType
{ DOW_WEEKEND = 'W',
  DOW_MIDWEEK = 'M',
  DOW_NIGHT = 'N',
  DOW_OFF_PEAK_TIME = 'O',
  DOW_WEEKEND_NIGHT = 'Y',
  DOW_MIDWEEK_NIGHT = 'Z',
  DOW_BLANK = ' ' };

extern const std::string DIAGNOSTIC_INCLUDE_GEO;
}
}
