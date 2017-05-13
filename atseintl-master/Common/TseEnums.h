//----------------------------------------------------------------------------
//
//  File:        TseEnums.h
//  Created:     2/4/2004
//  Authors:     Stephen Suggs
//
//  Description: Common enums required for ATSE shopping/pricing.
//
//  Updates:
// s
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

#include <string>
#include <sstream>

namespace tse
{
enum GlobalDirection : uint8_t
{
  NO_DIR = 0, /* None */
  AF = 1, /* via Africa */
  AL, /* FBR - AllFares incl. EH/TS */
  AP, /* via Atlantic and Pacific */
  AT, /* via Atlantic */
  CA, /* Canada */
  CT, /* circle trip */
  DI, /* special USSR - TC3 app. British Airways */
  DO, /* domestic */
  DU, /* special USSR - TC2 app. British Airways */
  EH, /* within Eastern Hemisphere */
  EM, /* via Europe - Middle East */
  EU, /* via Europe */
  FE, /* Far East */
  IN, /* FBR for intl. incl. AT/PA/WH/CT/PV */
  ME, /* via Middle East (other than Aden) */
  NA, /* FBR for North America incl. US/CA/TB/PV */
  NP, /* via North or Central Pacific */
  PA, /* via South, Central or North Pacific */
  PE, /* TC1 - Central/Southern Africa via TC3 */
  PN, /* between TC1 and TC3 via Pacific and via North America */
  PO, /* via Polar Route */
  PV, /* PR/VI - US/CA */
  RU, /* Russia - Area 3 */
  RW, /* round the world */
  SA, /* South Atlantic only */
  SN, /* via South Atlantic (routing permitted in 1 direc. via N./Mid-Atlantic) */
  SP, /* via South Polar */
  TB, /* transborder */
  TS, /* via Siberia */
  TT, /* Area 2 */
  US, /* intra U.S. */
  WH, /* within Western Hemisphere */
  XX, /* Universal */
  ZZ /* Any Global */
};

bool
globalDirectionToStr(std::string& dst, const GlobalDirection src);
const std::string*
globalDirectionToStr(const GlobalDirection src);
bool
strToGlobalDirection(GlobalDirection& dst, const std::string& src);

/*---------------------------------------------------------------------------
 * Geo Travel Types
 *-------------------------------------------------------------------------*/
enum class GeoTravelType : uint8_t
{
  UnknownGeoTravelType = 0,
  Domestic,
  International,
  Transborder,
  ForeignDomestic
};

enum class StopType : uint8_t
{ UnknownStopType = 0,
  StopOver,
  Connection };

enum class TariffType : uint8_t
{ Unknown = 0,
  Private,
  Published,
  Mixed };

/*---------------------------------------------------------------------------
 * Travel Boundaries
 *-------------------------------------------------------------------------*/
enum class Boundary : uint8_t
{ USCA,
  EXCEPT_USCA,
  AREA_21,
  AREA_11,
  OTHER_SUB_IATA,
  ONE_IATA,
  TWO_IATA,
  ALL_IATA,
  UNKNOWN };

/*---------------------------------------------------------------------------
 * Travel Segment Types
 *-------------------------------------------------------------------------*/
enum TravelSegType : uint8_t
{
  UnknownTravelSegType = 0,
  Air = 'A',
  Arunk = 'K',
  Open = 'O',
  Surface = 'S',
  Bus = 'B',
  Train = 'T',
  Tgv = 'G',
  Boat = 'U',
  Ice = 'I',
  Lmo = 'L'
};

/*---------------------------------------------------------------------------
 * Travel Segment Journey Types
 *-------------------------------------------------------------------------*/
enum class TravelSegJourneyType : uint8_t
{ ArunkSegment = 'A',
  NonJourneySegment = 'N',
  FlowJourneySegment = 'F',
  LocalJourneySegment = 'L',
  InterlineJourneySegment = 'I' };

/*---------------------------------------------------------------------------
 * Location Types
 *-------------------------------------------------------------------------*/
enum LocType : uint8_t
{
  UNKNOWN_LOC = 0,
  IATA_AREA = 'A',
  SUBAREA = '*',
  MARKET = 'C',
  NATION = 'N',
  STATE_PROVINCE = 'S',
  ZONE = 'Z'
};

/*---------------------------------------------------------------------------
 * Program, fare market and reduced fare directionality
 *-------------------------------------------------------------------------*/
enum class Direction : uint8_t
{
  ORIGINAL,
  REVERSED,
  BOTHWAYS
};
std::string directionToIndicator(const Direction direction);
std::string directionToString(const Direction direction);
std::ostream& operator<<(std::ostream& out, const Direction direction);

/*---------------------------------------------------------------------------
 * Fare directionality
 *-------------------------------------------------------------------------*/
enum Directionality : uint8_t
{
  FROM = 1,
  TO,
  BETWEEN,
  WITHIN,
  BOTH,
  ORIGIN,
  TERMINATE
};

/*---------------------------------------------------------------------------
 * FareClassAppseg directional indicator
 *-------------------------------------------------------------------------*/
enum FCASegDirectionality : uint8_t
{
  DIR_IND_NOT_DEFINED = 0,
  ORIGINATING_LOC1 = 3,
  ORIGINATING_LOC2 = 4
};

/*---------------------------------------------------------------------------
 * Tariff Cross-Reference record type
 *-------------------------------------------------------------------------*/
enum RecordScope : uint8_t
{
  DOMESTIC = 1,
  INTERNATIONAL
};

/*---------------------------------------------------------------------------
 * Rounding Rule
 *-------------------------------------------------------------------------*/
enum RoundingRule : uint8_t
{
  DOWN,
  UP,
  NEAREST,
  NONE,
  EMPTY
};

/*---------------------------------------------------------------------------
 * Record 3 Return Types
 *-------------------------------------------------------------------------*/
enum Record3ReturnTypes : uint8_t
{
  NOTPROCESSED = 1,
  FAIL,
  PASS,
  SOFTPASS,
  SKIP,
  STOP,
  STOP_SKIP,
  STOP_SOFT
};

/*---------------------------------------------------------------------------
 * Calendar stuff
 *-------------------------------------------------------------------------*/

enum DateFormat : uint8_t
{
  YYYYMMDD = 0,
  YYYYMmmDD,
  YYYYMMMDD,
  MMDDYYYY,
  MMDDYY,
  MmmDDYYYY,
  MMMDDYYYY,
  MMMDDYY,
  DDMMYYYY,
  DDMmmYYYY,
  DDMMMYYYY,
  DDMMMYY,
  DDMMY,
  DDMMM,
  DDMM,
  DDMM_FD // Fare Display Format
};

/*---------------------------------------------------------------------------
 * Time Formats
 *-------------------------------------------------------------------------*/
enum TimeFormat : uint8_t
{
  HHMMSS = 0,
  HHMM,
  HHMM_AMPM
};

enum MinimumFareModule : uint8_t
{
  HIP,
  BHC,
  CTM,
  COM,
  DMC,
  COP,
  OSC,
  RSC,
  CPM,
  OJM,
  NCJ, // Nigerian Currency Adjustment
  HRT,
  MAX_FARE_MODULE_IND
};

/*---------------------------------------------------------------------------
 * Combinability validation return values
 *-------------------------------------------------------------------------*/
enum CombinabilityValidationResult : uint8_t
{
  CVR_PASSED = 0,
  CVR_UNSPECIFIED_FAILURE,
  CVR_RT_NOT_PERMITTED,
  CVR_CT_NOT_PERMITTED,
  CVR_SOJ_NOT_PERMITTED,
  CVR_DOJ_NOT_PERMITTED,
  CVR_EOE_NOT_PERMITTED,
  CVR_EOE_REQUIRED,
  CVR_NO_REC2CAT10,
  CVR_CONTINUE
};

/*---------------------------------------------------------------------------
 * Rule Types
 *-------------------------------------------------------------------------*/
enum RuleType : uint8_t
{
  FareByRule = 0,
  FootNote,
  FareRule,
  Combinability,
  GeneralRule,
  BFFootNote,
  BFFareRule,
  BFGeneralRule
};

/*---------------------------------------------------------------------------
 * Fare Component change status for Port-Exchange/Cat31 transactions
 *-------------------------------------------------------------------------*/
enum FCChangeStatus : uint8_t
{
  UK, // Unkown
  UU, // Unflown/Unchanged
  UN, // Unflown/Unchanged after point of change
  FL, // Flown
  UC, // Unflown/Changed
  FCS_TAG_WAR_MATRIX_SIZE = 5
};

enum class OBFeeSubType : uint8_t
{ OB_UNKNOWN = 0,
  OB_F_TYPE,
  OB_T_TYPE,
  OB_R_TYPE };

enum StatusS4Validation : uint8_t
{
  PASS_S4 = 0,
  FAIL_SVC_TYPE,
  FAIL_PAX_TYPE,
  FAIL_ACC_T172,
  FAIL_TDSG_T173,
  FAIL_SECUR_T183,
  FAIL_TKT_DATE,
  FAIL_TRV_DATE,
  FAIL_GEO_BTW,
  FAIL_FARE_BASIS,
  FAIL_ON_GEO_LOC,
  DUPLICATE_S4,
  INTERNAL_ERROR,
  TJR_NOT_APPLY_TKTFEE,
  OB_NOT_ACTIVE_YET,
  VALIDATING_CXR_EMPTY,
  ALL_SEGS_OPEN,
  NOT_ALL_SEGS_CONFIRM,
  PASS_FOP,
  PROCESSING_ERROR,
  FAIL_FOP_BIN,
  NON_CREDIT_CARD_FOP,
  R_TYPE_NOT_REQUIRED,
  T_TYPE_NOT_REQUIRED,
  F_TYPE_NOT_REQUIRED,
  SKIP_TTYPE_NOT_EXCHANGE_REFUND
};

enum StatusS5Validation : uint8_t
{
  PASS_S5 = 0,
  OC_NOT_ACTIVE_SHOPPING,
  OC_NOT_ACTIVE_YET,
  ANCILLARY_NOT_ACTIVE,
  ANCILLARY_WP_DISPLAY_NOT_ACTIVE,
  ANCILLARY_R7_NOT_ACTIVE,
  FAIL_S5_TKT_DATE,
  FAIL_S5_FLIGHT_RELATED,
  FAIL_S5_INDUSTRY_INDICATOR,
  FAIL_S5_DISPLAY_CATEGORY,
  FAIL_S5_EMD_TYPE,
  FAIL_S5_EMD_AGREEMENT_CHECK,
  FAIL_S6,
  FAIL_S7
};

enum StatusS7Validation : uint8_t
{
  PASS_S7 = 0,
  PASS_S7_DEFER,
  FAIL_S7_INPUT_TVL_DATE,
  FAIL_S7_TVL_DATE,
  FAIL_S7_INPUT_PSG_TYPE,
  FAIL_S7_ACCOUNT_CODE,
  FAIL_S7_INPUT_TKT_DESIGNATOR,
  FAIL_S7_OUTPUT_TKT_DESIGNATOR,
  FAIL_S7_SECUR_T183,
  FAIL_S7_UPGRADE,
  FAIL_S7_SECTOR_PORTION,
  FAIL_S7_FROM_TO_WITHIN,
  FAIL_S7_INTERMEDIATE_POINT,
  FAIL_S7_STOP_CNX_DEST,
  FAIL_S7_CABIN,
  FAIL_S7_RBD_T198,
  FAIL_S7_SFC_T170,
  FAIL_S7_FREQ_FLYER_STATUS,
  FAIL_S7_SEQUENCE,
  FAIL_S7_EQUIPMENT,
  FAIL_S7_TOURCODE,
  FAIL_S7_START_STOP_TIME,
  FAIL_S7_DOW,
  FAIL_S7_ADVPUR,
  FAIL_S7_NOT_AVAIL_NO_CHANGE,
  FAIL_S7_DEFER_BAGGAGE_RULE,
  PASS_S7_NOT_AVAIL,
  PASS_S7_FREE_SERVICE,
  FAIL_S7_RESULT_FC_T171,
  FAIL_S7_MILEAGE_FEE,
  FAIL_S7_FEE_APPL,
  FAIL_S7_RULE_TARIFF_IND,
  FAIL_S7_RULE_TARIFF,
  FAIL_S7_RULE,
  FAIL_S7_FARE_IND,
  FAIL_S7_CXR_FLT_T186,
  FAIL_S7_COLLECT_SUBTRACT,
  FAIL_S7_NET_SELL,
  FAIL_S7_ADV_PURCHASE_TKT_IND,
  SOFT_PASS_S7,
  FAIL_S7_AND_OR_IND,
  FAIL_S7_BAGGAGE_WEIGHT_UNIT,
  FAIL_S7_FEE_APPLICATION,
  FAIL_S7_BAGGAGE_OCCURRENCE,
  FAIL_S7_FREE_BAG_PIECES,
  FAIL_S7_TAX_APPLICATION,
  FAIL_S7_AVAILABILITY,
  FAIL_S7_RULE_BUSTER_RMFC,
  FAIL_S7_PAX_MIN_MAX_AGE,
  FAIL_S7_PAX_OCCURRENCE,
  FAIL_S7_UPGRADE_T198,
  FAIL_S7_BTA,
  FAIL_S7_INTERLINE_IND
};

enum StatusT183Security : uint8_t
{
  PASS_T183 = 0,
  FAIL_TVL_AGENCY,
  FAIL_CXR_GDS,
  FAIL_DUTY,
  FAIL_FUNCTION_CODE,
  FAIL_GEO,
  FAIL_TYPE_CODE,
  FAIL_BOOK_TKT_IND,
  FAIL_VIEW_IND,
  NO_VAL
};

enum StatusT171 : uint8_t
{
  PASS_T171 = 0,
  FAIL_ON_CXR,
  FAIL_ON_FARE_CLASS,
  FAIL_NO_FARE_TYPE,
  SOFTPASS_FARE_CLASS,
  SOFTPASS_FARE_TYPE
};

enum StatusT173 : uint8_t
{
  PASS_T173 = 0,
  FAIL_NO_INPUT,
  FAIL_ON_BLANK,
  FAIL_NO_OUTPUT
};

enum StatusT186 : uint8_t
{
  PASS_T186 = 0,
  FAIL_ON_MARK_CXR,
  FAIL_ON_OPER_CXR,
  FAIL_ON_FLIGHT,
  SOFTPASS_FLIGHT
};

enum StatusT189 : uint8_t
{
  PASS_T189 = 0,
  FAIL_FARECLASS,
  FAIL_FARETYPE,
  FAIL_PAXTYPE,
  FAIL_ROUTING,
  FAIL_PRIME_RBD,
  APPL_NEGATIVE,
  FAIL_OWRT,
  FAIL_SOURCE,
  FAIL_RULE_TARIFF,
  FAIL_RULE,
  FAIL_SEC_T189,
  SOFTPASS_RBD,
  NO_STATUS,
  PASS_RANGE,
  FAIL_RANGE1_CURR,
  FAIL_RANGE1_DECIMAL,
  FAIL_RANGE1_MIN,
  FAIL_RANGE1_MAX,
  FAIL_RANGE2_CURR,
  FAIL_RANGE2_DECIMAL,
  FAIL_RANGE2_MIN,
  FAIL_RANGE2_MAX,
  FAIL_RULE_TARIFF_IND
};

enum StatusT198 : uint8_t
{
  PASS_T198 = 0,
  FAIL_RBD_NOT_PROCESSED,
  FAIL_ON_RBD_CXR,
  FAIL_ON_RBD_CODE,
  FAIL_UPGR_RBD_T198
};

enum class StatusBrandingService : uint8_t
{
  NO_BS_ERROR = 0,
  BS_UNAVAILABLE,
  BS_INVALID_RESPONSE
};

enum class IbfErrorMessage : uint8_t
{
  IBF_EM_NO_FARE_FILED = 0,
  IBF_EM_EARLY_DROP,
  IBF_EM_NOT_OFFERED,
  IBF_EM_NOT_AVAILABLE,
  IBF_EM_NO_FARE_FOUND,
  IBF_EM_NOT_SET
};

std::ostream& operator<<(std::ostream& out, IbfErrorMessage m);

enum BookingCodeValidationStatus : uint8_t
{
  BOOKING_CODE_PASSED = 0,
  BOOKING_CODE_NOT_OFFERED,
  BOOKING_CODE_IS_EXCLUDED,
  BOOKING_CODE_NOT_AVAILABLE,
  BOOKING_CODE_STATUS_NOT_SET,
  BOOKING_CODE_CABIN_NOT_MATCH
};

enum class TnShoppingBrandingMode : uint8_t
{ NO_BRANDS = 0,
  RETRIEVE_ONLY,
  SINGLE_BRAND,
  MULTIPLE_BRANDS };

enum class TnShoppingBrandsLimit : uint8_t
{ UNLIMITED_BRANDS = 0 };

enum class BrandRetrievalMode : uint8_t
{ PER_O_AND_D = 0,
  PER_FARE_COMPONENT };

/*---------------------------------------------------------------------------
 * ATPCO RBD by Cabin Answer table
 *-------------------------------------------------------------------------*/
enum RBDByCabinCall : uint8_t
{ NO_CALL = 0,
  ITIN_SHP_PYA,
  ITIN_SHP_SVC,
  SOLD_OUT,
  DSS_RSP,
  AVAIL_RSP,
  CONTENT_SVC,
  CONTENT_SVC_FAKE,
  FAMILY_LOGIC,
  OPTIONAL_SVC,
  ANC_BAG_RQ,
  PRICING_RQ,
  T999_VAL,
  RBD_CAT31,
  PREFERRED_CABIN,
  SHP_HANDLER,
  SHP_WPNI,
  RBD_VAL};

enum RBDByCabinStatus : uint8_t
{ PASS_SEQ = 0,
  PASS_CXR,
  PASS_YY,
  PASS_DEFAULT,
  FAIL_GLOBAL_IND,
  FAIL_TRAVEL_DATE,
  FAIL_TICKET_DATE,
  FAIL_GEO_LOC,
  FAIL_EQUP,
  FAIL_FLIGHT,
  SKIP_SEQ,
  UNKNOWN_STATUS };

/*---------------------------------------------------------------------------
 * Match type for Record 2
 *-------------------------------------------------------------------------*/

enum MATCHTYPE : int8_t
{
  MATCHNONE = -1,
  FARERULE,
  FOOTNOTE
};

/*---------------------------------------------------------------------------
 * Airline Shoppping procesing direction
 *-------------------------------------------------------------------------*/
enum class ProcessingDirection : uint8_t
{ ONEWAY = 0,
  ROUNDTRIP_OUTBOUND,
  ROUNDTRIP_INBOUND,
  ROUNDTRIP };

enum RuleCategories : uint8_t
{
  ELIGIBILITY_RULE = 1,
  DAY_TIME_RULE = 2,
  SEASONAL_RULE = 3,
  FLIGHT_APPLICATION_RULE = 4,
  ADVANCE_RESERVATION_RULE = 5,
  MINIMUM_STAY_RULE = 6,
  MAXIMUM_STAY_RULE = 7,
  STOPOVER_RULE = 8,
  TRANSFER_RULE = 9,
  COMBINABILITY_RULE = 10,
  BLACKOUTS_RULE = 11,
  SURCHARGE_RULE = 12,
  ACCOMPANIED_PSG_RULE = 13,
  TRAVEL_RESTRICTIONS_RULE = 14,
  SALE_RESTRICTIONS_RULE = 15,
  PENALTIES_RULE = 16,
  HIP_RULE = 17,
  TICKET_ENDORSMENT_RULE = 18,
  CHILDREN_DISCOUNT_RULE = 19,
  TOUR_DISCOUNT_RULE = 20,
  AGENTS_DISCOUNT_RULE = 21,
  OTHER_DISCOUNT_RULE = 22,
  MISC_FARE_TAG = 23,
  FARE_BY_RULE = 25,
  TOURS_RULE = 27,
  VOLUNTARY_EXCHANGE_RULE = 31,
  VOLUNTARY_REFUNDS_RULE = 33,
  NEGOTIATED_RULE = 35
};

enum StatusS8 : uint8_t
{
  PASS_S8 = 0,
  FAIL_S8_PASSENGER_TYPE,
  FAIL_S8_ACCOUNT_CODE,
  FAIL_S8_MARKET,
  FAIL_S8_PCC,
  FAIL_S8_CARRIER,
  FAIL_S8_SALES_DATE,
  FAIL_S8_TRAVEL_DATE,
  FAIL_S8_IATA_NUMBER,
  FAIL_S8_CARRIER_GDS,
  FAIL_S8_AGENT_LOCATION,
  FAIL_S8_DEPARTMENT_CODE,
  FAIL_S8_OFFICE_DESIGNATOR,
  FAIL_S8_SECURITY,
  FAIL_S8_VIEW_BOOK_TICKET
};

enum StatusFFRuleValidation : uint8_t
{
  PASS_FF = 0,
  FAIL_FF_VENDOR,
  FAIL_FF_GEO,
  FAIL_FF_DIRECTIONALITY,
  FAIL_FF_CARRIER,
  FAIL_FF_SOURCE_PCC,
  FAIL_FF_RULE,
  FAIL_FF_RULE_TARIFF,
  FAIL_FF_FARE_CLASS,
  FAIL_FF_FARE_TYPE,
  FAIL_FF_BOOKING_CODE,
  FAIL_FF_MATCH_TRAVEL_DATE_RANGE_X5,
  FAIL_FF_OWRT,
  FAIL_FF_DCT,
  FAIL_FF_PUBLIC_IND,
  FAIL_FF_LOOKUP_EMPTY,
  FAIL_FF_RULES_EMPTY,
  FAIL_FF_RULES_SECURITY_HANDSHAKE,
  FAIL_FF_SECURITY
};

enum StatusFRRuleValidation : uint8_t
{
  PASS_FR = 0,
  FAIL_FR_VENDOR,
  FAIL_FR_CARRIER,
  FAIL_FR_GEO,
  FAIL_FR_EXCLUDE_GEO,
  FAIL_FR_DIRECTIONALITY,
  FAIL_FR_FARE_TYPE,
  FAIL_FR_FARE_CLASS,
  FAIL_FR_EXCLUDE_FARE_CLASS,
  FAIL_FR_RULE,
  FAIL_FR_RULE_TARIFF,
  FAIL_FR_BOOKING_CODE,
  FAIL_FR_SECURITY,
  FAIL_FR_MATCH_TRAVEL_DATE_RANGE_X5,
  FAIL_FR_ACCOUNTCD,
  FAIL_FR_EXCLUDE_DCT,
  FAIL_FR_PASSENGERTYPECODE,
  FAIL_FR_PUBLIC_PRIVATE,
  FAIL_FR_CAT35_DISPLAY_TYPE,
  FAIL_FR_MATCH_OWRT,
  FAIL_FR_MATCH_RETAILERCODE,
  FAIL_FR_ALL
};

enum Continent : uint8_t
{
  UNKNOWN_CONTINENT = 0,
  CONTINENT_AFRICA = 1,
  CONTINENT_ASIA = 2,
  CONTINENT_EUROPE = 3,
  CONTINENT_EUROPE_MIDDLE_EAST = 4,
  CONTINENT_NORTH_AMERICA = 5,
  CONTINENT_SOUTH_AMERICA = 6,
  CONTINENT_SOUTH_WEST_PACIFIC = 7,
  CONTINENT_SOUTH_CENTRAL_AMERICA = 8
};

enum CommissionValidationStatus : uint8_t
{
  PASS_CR = 0,
  SKIP_CR,
  FAIL_CR_FARE_BASIS_INCL,
  FAIL_CR_FARE_BASIS_EXCL,
  FAIL_CR_CLASS_OF_SERVICE_INCL,
  FAIL_CR_CLASS_OF_SERVICE_EXCL,
  FAIL_CR_OPER_CARRIER_INCL,
  FAIL_CR_OPER_CARRIER_EXCL,
  FAIL_CR_MARKET_CARRIER_INCL,
  FAIL_CR_MARKET_CARRIER_EXCL,
  FAIL_CR_TICKET_CARRIER_INCL,
  FAIL_CR_TICKET_CARRIER_EXCL,
  FAIL_CR_INTERLINE_CONNECTION,
  FAIL_CR_ROUNDTRIP,
  FAIL_CR_FARE_AMOUNT_MIN,
  FAIL_CR_FARE_AMOUNT_MIN_NODEC,
  FAIL_CR_FARE_AMOUNT_MAX,
  FAIL_CR_FARE_AMOUNT_MAX_NODEC,
  FAIL_CR_FARE_CURRENCY,
  FAIL_CR_FBC_FRAGMENT_INCL,
  FAIL_CR_FBC_FRAGMENT_EXCL,
  FAIL_CR_REQ_TKT_DESIGNATOR,
  FAIL_CR_EXCL_TKT_DESIGNATOR,
  FAIL_CR_REQ_CONN_AIRPORT,
  FAIL_CR_EXCL_CONN_AIRPORT,
  FAIL_CR_REQ_MKT_GOV_CXR,
  FAIL_CR_EXCL_MKT_GOV_CXR,
  FAIL_CR_PSGR_TYPE,
  FAIL_CR_REQ_OPER_GOV_CXR,
  FAIL_CR_EXCL_OPER_GOV_CXR,
  FAIL_CR_REQ_NON_STOP,
  FAIL_CR_REQ_CABIN,
  FAIL_CR_EXCL_CABIN,
  FAIL_CR_EXCL_TOUR_CODE,
  FAIL_CP_POINT_OF_SALE,
  FAIL_CP_POINT_OF_ORIGIN,
  FAIL_CP_TRAVEL_DATE,
  FAIL_CP_TICKET_DATE,
  FAIL_CP_MAX_CONNECTION_TIME,
  FAIL_CP_MARKET,
  FAIL_CP_NOT_VALID
};

//-----------------------------------------------------------------------------
// Known table driven common fare field elements
//-----------------------------------------------------------------------------
enum FieldColumnElement : uint8_t
{
  UNKNOWN_ELEMENT = 0,
  LINE_NUMBER = 1,
  SAME_DAY_IND = 2,
  VENDOR_TRAVELOCITY_CODE = 3,
  PRIVATE_FARE_IND = 4,
  FARE_BASIS_TKT_DESIG = 5,
  BOOKING_CODE = 6,
  JOURNEY_IND = 7,
  FD_FARE_AMOUNT = 8,
  OW_FARE_AMOUNT = 9,
  RT_FARE_AMOUNT = 10,
  NET_FARE_IND = 11,
  TRAVEL_TICKET = 12,
  SEASONS = 13,
  ADVANCE_PURCHASE = 14,
  MIN_MAX_STAY = 15,
  ROUTING = 16,
  CONSTRUCTED_FARE_IND = 17,
  ONE_WAY_TAX = 18,
  ONE_WAY_TOTAL = 19,
  RT_TAX = 20,
  RT_TOTAL = 21,
  DIRECTIONAL_IND = 22,
  CARRIER = 23,
  ADDON_DIRECTION = 24,
  IN_OUT_INDICATOR = 27,
  NON_COC_CURRENCY = 28,
  NET_FARE_NON_COC_IND = 29,
  FARE_BY_RULE_IND = 31,
  RULE_TARIFF = 32,
  FARE_RULE = 33,
  FARE_TYPE = 34,
  ROUTING_TYPE_NUMBER = 35,
  REDISTRIBUTION = 36,
  DISPLAY_CAT_TYPE = 37,
  FARE_CONSTRUCTION_IND = 51
};

enum PbbProcessing : uint8_t
{
  NOT_PBB_RQ = 0,
  PBB_RQ_PROCESS_BRANDS = 1,
  PBB_RQ_DONT_PROCESS_BRANDS = 2
};

enum BrandSource : uint8_t
{
  BRAND_SOURCE_S8 = 0,
  BRAND_SOURCE_CBAS = 1
};

enum class HasherMethod : uint16_t
{
  METHOD_0,
  METHOD_1,
  METHOD_2,
  METHOD_3,
  METHOD_4,
  METHOD_5,
  METHOD_6,
  METHOD_7
};

enum FMTravelBoundary
{ TravelWithinUSCA = 0x01,
  TravelWithinSameCountryExceptUSCA = 0x02,
  TravelWithinOneIATA = 0x04,
  TravelWithinTwoIATA = 0x08,
  TravelWithinAllIATA = 0x10,
  TravelWithinSubIATA11 = 0x20,
  TravelWithinSubIATA21 = 0x40,
  TravelWithinSameSubIATAExcept21And11 = 0x80
};

enum class FMDirection : char
{
  UNKNOWN = 0,
  INBOUND = 'I',
  OUTBOUND = 'O'
};

Continent
getContinentByAllianceContinentCode(int allianceContinentCode);

void
getBrandingServiceErrorMessage(StatusBrandingService status, std::string& errorMessage);

} // end tse namespace

