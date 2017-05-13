//----------------------------------------------------------------------------
//
//  File:        TseEnums.h
//  Created:     2/4/2004
//  Authors:     Stephen Suggs
//
//  Description: Common enums required for ATSE shopping/pricing.
//
//  Updates:
//s
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

namespace tse {


/*---------------------------------------------------------------------------
 * Global Direction
 *-------------------------------------------------------------------------*/
enum GlobalDirection
{
  NO_DIR = 0, /* None */
  AF = 1,   /* via Africa */
  AL,       /* FBR - AllFares incl. EH/TS */
  AP,       /* via Atlantic and Pacific */
  AT,       /* via Atlantic */
  CA,       /* Canada */
  CT,       /* circle trip */
  DI,       /* special USSR - TC3 app. British Airways */
  DO,       /* domestic */
  DU,       /* special USSR - TC2 app. British Airways */
  EH,       /* within Eastern Hemisphere */
  EM,       /* via Europe - Middle East */
  EU,       /* via Europe */
  FE,       /* Far East */
  IN,       /* FBR for intl. incl. AT/PA/WH/CT/PV */
  ME,       /* via Middle East (other than Aden) */
  NA,       /* FBR for North America incl. US/CA/TB/PV */
  NP,       /* via North or Central Pacific */
  PA,       /* via South, Central or North Pacific */
  PE,       /* TC1 - Central/Southern Africa via TC3 */
  PN,       /* between TC1 and TC3 via Pacific and via North America */
  PO,       /* via Polar Route */
  PV,       /* PR/VI - US/CA */
  RU,       /* Russia - Area 3 */
  RW,       /* round the world */
  SA,       /* South Atlantic only */
  SN,       /* via South Atlantic (routing permitted in 1 direc. via N./Mid-Atlantic) */
  SP,       /* via South Polar */
  TB,       /* transborder */
  TS,       /* via Siberia */
  TT,       /* Area 2 */
  US,       /* intra U.S. */
  WH,       /* within Western Hemisphere */
  XX,       /* Universal */
  ZZ        /* Any Global */
};

bool globalDirectionToStr( std::string& dst, const GlobalDirection src );
const std::string* globalDirectionToStr(const GlobalDirection src);
bool strToGlobalDirection( GlobalDirection& dst, const std::string& src );

/*---------------------------------------------------------------------------
 * Geo Travel Types
 *-------------------------------------------------------------------------*/
enum GeoTravelType
{
  UnknownGeoTravelType = 0,
  Domestic,
  International,
  Transborder,
  ForeignDomestic
};


/*---------------------------------------------------------------------------
 * Stop Types
 *-------------------------------------------------------------------------*/
enum StopType
{
  UnknownStopType = 0,
  StopOver,
  Connection
};

/*---------------------------------------------------------------------------
 * Travel Boundaries
 *-------------------------------------------------------------------------*/
enum Boundary
{
  USCA,
  EXCEPT_USCA,
  AREA_21,
  AREA_11,
  OTHER_SUB_IATA,
  ONE_IATA,
  TWO_IATA,
  ALL_IATA,
  UNKNOWN
};


/*---------------------------------------------------------------------------
 * Trx Request Types
 *-------------------------------------------------------------------------*/
enum TrxRequestType
{
  UnknownTrxRequestType = 0,
  StatusCheckOnly,
  Pricing,
  WP,
  WPNC,
  WPNCB,
  PFC,
  TAXES,
  TAX_DISPLAY,
  TAX_RECORD,
  CurrencyConversion,
  MileageInformation,
  Metrics
};

/*---------------------------------------------------------------------------
 * Travel Segment Types
 *-------------------------------------------------------------------------*/
enum TravelSegType
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
  Ice = 'I' ,
  Lmo = 'L'
};

/*---------------------------------------------------------------------------
 * Location Types
 *-------------------------------------------------------------------------*/
enum LocType
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
 * Fare directionality
 *-------------------------------------------------------------------------*/
enum Directionality
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
enum FCASegDirectionality
{
  DIR_IND_NOT_DEFINED = 0,
  ORIGINATING_LOC1 = 3,
  ORIGINATING_LOC2 = 4
};


/*---------------------------------------------------------------------------
 * Tariff Cross-Reference record type
 *-------------------------------------------------------------------------*/
enum RecordScope
{
  DOMESTIC = 1,
  INTERNATIONAL
};


/*---------------------------------------------------------------------------
 * Rounding Rule
 *-------------------------------------------------------------------------*/
enum RoundingRule
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
enum Record3ReturnTypes
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

enum DateFormat
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
  DDMM_FD  // Fare Display Format
};

/*---------------------------------------------------------------------------
 * Time Formats
 *-------------------------------------------------------------------------*/
enum TimeFormat
{
  HHMMSS = 0,
  HHMM,
  HHMM_AMPM
};

enum MinimumFareModule
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
    NCJ,     // Nigerian Currency Adjustment
    HRT
};

/*---------------------------------------------------------------------------
 * Combinability validation return values
 *-------------------------------------------------------------------------*/
enum CombinabilityValidationResult
{
    CVR_PASSED = 0,
    CVR_UNSPECIFIED_FAILURE,
    CVR_RT_NOT_PERMITTED,
    CVR_CT_NOT_PERMITTED,
    CVR_SOJ_NOT_PERMITTED,
    CVR_DOJ_NOT_PERMITTED,
    CVR_EOE_NOT_PERMITTED,
    CVR_EOE_REQUIRED,
    CVR_NO_REC2CAT10
};

/*---------------------------------------------------------------------------
 * Rule Types
 *-------------------------------------------------------------------------*/
enum RuleType
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
enum FCChangeStatus
{
  UK, //Unkown
  UU, //Unflown/Unchanged
  UN, //Unflown/Unchanged after point of change
  FL, //Flown
  UC,  //Unflown/Changed
  FCS_TAG_WAR_MATRIX_SIZE = 5
};

enum StatusS4Validation
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
    FAIL_ON_PCT,
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
    PROCESSING_ERROR
};

enum StatusS5Validation
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
  FAIL_S6,
  FAIL_S7
};

enum StatusS7Validation
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
  FAIL_S7_BAGGAGE_OCCURRENCE
};

enum StatusT183Security
{
  PASS_T183 = 0,
  FAIL_TVL_AGENCY,
  FAIL_CXR_GDS,
  FAIL_DUTY,
  FAIL_FUNCTION_CODE,
  FAIL_GEO,
  FAIL_TYPE_CODE,
  FAIL_BOOK_TKT_IND,
  FAIL_VIEW_IND
};

enum StatusT171
{
  PASS_T171 = 0,
  FAIL_ON_CXR,
  FAIL_ON_FARE_CLASS,
  FAIL_NO_FARE_TYPE,
  SOFTPASS_FARE_CLASS,
  SOFTPASS_FARE_TYPE
};

enum StatusT173
{
  PASS_T173 = 0,
  FAIL_NO_INPUT,
  FAIL_ON_BLANK,
  FAIL_NO_OUTPUT
};

enum StatusT186
{
  PASS_T186 = 0,
  FAIL_ON_MARK_CXR,
  FAIL_ON_OPER_CXR,
  FAIL_ON_FLIGHT,
  SOFTPASS_FLIGHT
};

enum StatusT198
{
  PASS_T198 = 0,
  FAIL_RBD_NOT_PROCESSED,
  FAIL_ON_RBD_CXR,
  FAIL_ON_RBD_CODE
};

/*---------------------------------------------------------------------------
 * Match type for Record 2
 *-------------------------------------------------------------------------*/

enum MATCHTYPE
{
    MATCHNONE = -1
  , FARERULE
  , FOOTNOTE
} ;

/*---------------------------------------------------------------------------
 * Airline Shoppping procesing direction
 *-------------------------------------------------------------------------*/
enum ProcessingDirection
{
    ONEWAY = 0
  , ROUNDTRIP_OUTBOUND
  , ROUNDTRIP_INBOUND
	, ROUNDTRIP
} ;

} // end tse namespace
