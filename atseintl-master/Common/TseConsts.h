//----------------------------------------------------------------------------
//
//  File:           TseConsts.h
//  Created:        2/26/2004
//  Authors:        Vadim Nikushin
//
//  Description:    Common constants
//
//  Updates:
//
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

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

namespace tse
{
constexpr double EPSILON = 0.00001;
constexpr double HUNDRED = 100.00;

extern const std::string US_DOLLARS;
extern const VendorCode EMPTY_VENDOR;

// Consts representing invalid values
extern const FareType INVALID_FARETYPE;
extern const FareType DUMMY_FARE_TYPE;
extern const UserApplCode INVALID_USERAPPLCODE;
extern const NationCode INVALID_NATIONCODE;
extern const CurrencyCode INVALID_CURRENCYCODE;
extern const CarrierCode INVALID_CARRIERCODE;
extern const TaxCode INVALID_TAXCODE;
extern const LocCode INVALID_LOCCODE;
constexpr Indicator INVALID_INDICATOR = '\0';
constexpr uint16_t INVALID_BRAND_INDEX = 65535;
constexpr CurrencyNoDec INVALID_CURRENCY_NO_DEC = -1;
constexpr IntIndex INVALID_INT_INDEX = -1;
extern const BrandCode NO_BRAND;
extern const BrandCode ANY_BRAND;
extern const BrandCode ANY_BRAND_LEG_PARITY;

extern const CurrencyCode NUC;
extern const CurrencyCode IDR;
extern const CurrencyCode USD;

// some particular nations

extern const NationCode NATION_EMPTY;
extern const NationCode NATION_INTERNATIONAL;
extern const NationCode NATION_ALL;

extern const NationCode NATION_US;
extern const NationCode NATION_CA;
extern const NationCode NATION_THAILAND;
extern const NationCode NATION_BAHARAIN;
extern const NationCode NATION_KUWAIT;
extern const NationCode NATION_QUTAR;
extern const NationCode NATION_SAUDI_ARABIA;
extern const NationCode NATION_UNITED_ARAB_EMIRATES;
extern const NationCode NATION_FRANCE;
extern const NationCode NATION_MONACO;
extern const NationCode NATION_USSR;
extern const NationCode NATION_EUR; // East Ural Russia
extern const NationCode NATION_ECUADOR;
extern const NationCode NATION_BRAZIL;
extern const NationCode NATION_PANAMA;
extern const NationCode NATION_ISRAEL;

extern const std::string ANY_POSSIBLE_ROUTING;
extern const std::string IATA_AREA1; // IATA
extern const std::string IATA_AREA2; // IATA
extern const std::string IATA_AREA3; // IATA

extern const NationCode UNITED_STATES;
extern const NationCode ALASKA; // state code
extern const NationCode HAWAII; // state code
extern const NationCode CANADA;
extern const NationCode NEW_BRUNSWICK;
extern const NationCode NOVA_SCOTIA;
extern const NationCode NEW_FOUNDLAND;
extern const NationCode COLUMBIA;
extern const NationCode MEXICO;
extern const NationCode RUSSIA;
extern const NationCode EAST_URAL_RUSSIA;
extern const NationCode AUSTRALIA;
extern const NationCode NEW_ZEALAND;
extern const NationCode NIGERIA;
extern const NationCode PERU;
extern const NationCode ST_PIERRE_MIQUELON;

// European Countries

extern const NationCode UNITED_KINGDOM;
extern const NationCode AUSTRIA;
extern const NationCode FRANCE;
extern const NationCode GERMANY;
extern const NationCode SPAIN;

// Asia Countries

extern const NationCode JAPAN;
extern const NationCode INDONESIA;
extern const NationCode KOREA;

// US Possesions

extern const NationCode VIRGIN_ISLAND;
extern const NationCode PUERTORICO;
extern const NationCode GUAM;
extern const NationCode AMERICAN_SAMOA;

extern const NationCode CANTON_AND_ENDERBURY;
extern const NationCode MIDWAY;
extern const NationCode WAKE;
extern const NationCode MICRONESIA;
extern const NationCode SAIPAN;
extern const NationCode NORTHERN_MARIANA;

extern const NationCode NORTH_AMERICA; // subIATA
extern const NationCode EUROPE; // SubIATA

inline const IATASubAreaCode&
IATA_SUB_AREA_11()
{
  static const IATASubAreaCode code = "11"; // North America
  return code;
}

inline const IATASubAreaCode&
IATA_SUB_AREA_12()
{
  static const IATASubAreaCode code = "12"; // Carabbean
  return code;
}

inline const IATASubAreaCode&
IATA_SUB_AREA_13()
{
  static const IATASubAreaCode code = "13"; // Central America
  return code;
}

inline const IATASubAreaCode&
IATA_SUB_AREA_14()
{
  static const IATASubAreaCode code = "14"; // South America
  return code;
}

inline const IATASubAreaCode&
IATA_SUB_AREA_21()
{
  static const IATASubAreaCode code = "21"; // Europe
  return code;
}

inline const IATASubAreaCode&
IATA_SUB_AREA_22()
{
  static const IATASubAreaCode code = "22"; // Middle East
  return code;
}

inline const IATASubAreaCode&
IATA_SUB_AREA_23()
{
  static const IATASubAreaCode code = "23"; // Africa
  return code;
}

inline const IATASubAreaCode&
IATA_SUB_ARE_31()
{
  static const IATASubAreaCode code = "31"; // South Asian Sub-continent
  return code;
}

inline const IATASubAreaCode&
IATA_SUB_ARE_32()
{
  static const IATASubAreaCode code = "32"; // South East Asia
  return code;
}

inline const IATASubAreaCode&
IATA_SUB_ARE_33()
{
  static const IATASubAreaCode code = "33"; // Japan/Korea
  return code;
}

inline const IATASubAreaCode&
IATA_SUB_ARE_34()
{
  static const IATASubAreaCode code = "34"; // South West Pacific
  return code;
}

// scandinavian countries

extern const NationCode DENMARK;
extern const NationCode NORWAY;
extern const NationCode SWEDEN;
extern const NationCode GREENLAND;

extern const NationCode WILDCARD;

// Mileage
constexpr Indicator TPM = 'T';
constexpr Indicator MPM = 'M';
constexpr Indicator SOM = 'S';
constexpr Indicator GCM = 'G';

inline const std::string&
EMPTY_STRING()
{
  static const std::string empty = "";
  return empty;
}

constexpr LocTypeCode LOCTYPE_AREA = 'A';
constexpr LocTypeCode LOCTYPE_SUBAREA = '*';
constexpr LocTypeCode LOCTYPE_CITY = 'C';
constexpr LocTypeCode LOCTYPE_AIRPORT = 'P';
constexpr LocTypeCode LOCTYPE_NATION = 'N';
constexpr LocTypeCode LOCTYPE_STATE = 'S';
constexpr LocTypeCode LOCTYPE_ZONE = 'Z';
constexpr LocTypeCode LOCTYPE_FMSZONE = '1';
constexpr LocTypeCode LOCTYPE_NONE = ' ';
constexpr LocTypeCode LOCTYPE_PCC = 'X';
constexpr LocTypeCode LOCTYPE_PCC_ARC = 'Y';
constexpr LocTypeCode LOCTYPE_USER = 'U';

//  fare type family
extern const std::string FR_TYPE;
extern const std::string BJ_TYPE;
extern const std::string EW_TYPE;

constexpr char R_TYPE = 'R';
constexpr char F_TYPE = 'F';
constexpr char B_TYPE = 'B';
constexpr char E_TYPE = 'E';
constexpr char W_TYPE = 'W';
constexpr char X_TYPE = 'X';
constexpr char S_TYPE = 'S';
constexpr char P_TYPE = 'P';
constexpr char A_TYPE = 'A';
constexpr char Y_TYPE = 'Y';
constexpr char Z_TYPE = 'Z';
constexpr char J_TYPE = 'J';
constexpr char ALL_TYPE = '*';

// Pseudo City Codes
extern const PseudoCityCode TRAVELOCITY; //  mainTvlAgencyPCC for Travelocity

extern const PseudoCityType PCCTYPE_BRANCH; // Branch Pseudo City
extern const PseudoCityType PCCTYPE_HOME; // Home Pseudo City

// Loc

extern const LocCode LOC_SIN;
extern const LocCode LOC_BWN;
extern const LocCode LOC_KUL;
extern const LocCode LOC_KCH;
extern const LocCode LOC_BKI;
extern const LocCode LOC_YND;
extern const LocCode LOC_YMQ;
extern const LocCode LOC_YQB;
extern const LocCode LOC_YUL;
extern const LocCode EAP_BASEL;
extern const LocCode BSL_BASEL;
extern const LocCode LOC_NYC;
extern const LocCode LOC_EWR;
extern const LocCode LOC_WAS;
extern const LocCode LOC_BWI;
extern const LocCode LOC_NRT;

// General City Codes
extern const LocCode EastCoastCode;
extern const LocCode WestCoastCode;

// Loc in Spanish Oversea Islands
extern const LocCode LOC_TCI;
extern const LocCode LOC_LPA;
extern const LocCode LOC_FUE;
extern const LocCode LOC_ACE;
extern const LocCode LOC_PMI;
extern const LocCode LOC_MAH;
extern const LocCode LOC_IBZ;
extern const LocCode LOC_MLN;
extern const LocCode LOC_JCU;
extern const LocCode LOC_SPC;
extern const LocCode LOC_RCE;
extern const LocCode LOC_TFN;
extern const LocCode LOC_TFS;

extern const LocCode LOC_AGP;
extern const LocCode LOC_XRY;
extern const LocCode LOC_SVQ;

// States of Spanish Oversea Islands
extern const StateCode ST_RBP;
extern const StateCode ST_RRC;
extern const StateCode ST_RRM;
extern const StateCode ST_RCE;

// Group location for fare focus and fare retailer
constexpr Indicator GROUP_LOCATION = 'G';

// Carrier
extern const CarrierCode ANY_CARRIER;
extern const CarrierCode EMPTY_CARRIER;
extern const CarrierCode INDUSTRY_CARRIER;
extern const CarrierCode BAD_CARRIER;
extern const CarrierCode INBOUND_CARRIER;
extern const CarrierCode JOINT_CARRIER;
extern const CarrierCode DOLLAR_CARRIER;
extern const CarrierCode XDOLLAR_CARRIER;
extern const CarrierCode MH_CARRIER;
extern const CarrierCode SPECIAL_CARRIER_AA;
extern const CarrierCode CARRIER_9B;
extern const CarrierCode CARRIER_2R;
extern const CarrierCode CARRIER_JJ;
extern const CarrierCode CARRIER_WS;
extern const CarrierCode CARRIER_AM;
extern const CarrierCode CARRIER_DL;
extern const CarrierCode CARRIER_AIRSEYCHELLES;

extern const EquipmentType TRAIN;
extern const EquipmentType TGV;
extern const EquipmentType BUS;
extern const EquipmentType BOAT;
extern const EquipmentType ICE;
extern const EquipmentType LMO;
extern const EquipmentType TRS;

extern const PaxTypeCode ADULT;
extern const PaxTypeCode CHILD;
extern const PaxTypeCode INFANT;
extern const PaxTypeCode CNE;
extern const PaxTypeCode NEG;
extern const PaxTypeCode INE;
extern const PaxTypeCode CBC;
extern const PaxTypeCode PFA;
extern const PaxTypeCode CBI;
extern const PaxTypeCode JNN;
extern const PaxTypeCode JCB;
extern const PaxTypeCode JNF;
extern const PaxTypeCode JNS;
extern const PaxTypeCode INS;
extern const PaxTypeCode UNN;
extern const PaxTypeCode MIL;
extern const PaxTypeCode SRC;
extern const PaxTypeCode SNN;
extern const PaxTypeCode ADR;
extern const PaxTypeCode CHR;
extern const PaxTypeCode INR;
extern const PaxTypeCode ISR;
extern const PaxTypeCode UNR;
extern const PaxTypeCode FLY;
extern const PaxTypeCode FNN;

extern const PaxTypeCode CTZ;
extern const PaxTypeCode GCF;
extern const PaxTypeCode GCT;
extern const PaxTypeCode GDP;
extern const PaxTypeCode GEX;
extern const PaxTypeCode GST;
extern const PaxTypeCode GVT;
extern const PaxTypeCode NAT;
extern const PaxTypeCode GV1;
extern const PaxTypeCode LTC;
extern const PaxTypeCode SEA;

extern const PaxTypeCode AST;
extern const PaxTypeCode CSB;
extern const PaxTypeCode EMPTY_PAXTYPE_CODE;

constexpr ZoneType RESERVED = 'R';
constexpr ZoneType USER_DEFINED = 'U';
constexpr ZoneType MANUAL = 'M';
constexpr ZoneType FUNCTIONAL = 'F';
constexpr ZoneType TAX_ZONE = 'T';

extern const std::string CONFIRM_RES_STATUS;
extern const std::string NOSEAT_RES_STATUS;
extern const std::string QF_RES_STATUS;
extern const std::string RQ_RES_STATUS;
// Waitlist codes - LL, BL, DL, GL, HL, HN, PN, TL, WL and DS
extern const std::string LL_RES_STATUS;
extern const std::string BL_RES_STATUS;
extern const std::string DL_RES_STATUS;
extern const std::string GL_RES_STATUS;
extern const std::string HL_RES_STATUS;
extern const std::string HN_RES_STATUS;
extern const std::string PN_RES_STATUS;
extern const std::string TL_RES_STATUS;
extern const std::string WL_RES_STATUS;

extern const std::string DS_REAL_RES_STATUS;

// ATPCO OWRT indicator

constexpr Indicator ALL_WAYS = ' ';
constexpr Indicator ONE_WAY_MAY_BE_DOUBLED = '1';
constexpr Indicator ROUND_TRIP_MAYNOT_BE_HALVED = '2';
constexpr Indicator ONE_WAY_MAYNOT_BE_DOUBLED = '3';

//DFF FRR OWRT indicator
constexpr Indicator ANY_ONE_WAY = '4';

// SITA extra OWRT indicators for add-ons

constexpr Indicator SITA_OWRT_ADDON_4 = '4';
constexpr Indicator SITA_OWRT_ADDON_5 = '5';

// Zone

extern const Zone SABRE_WESTERN_AFRICA;
extern const Zone SABRE_EU_MEMBER_STATE;

// User
constexpr Indicator CRS_USER_APPL = 'C';
constexpr Indicator MULTIHOST_USER_APPL = 'M';
constexpr Indicator NO_PARAM = ' ';

// Multi Hosts
extern const std::string SABRE_USER;
extern const std::string INFINI_USER;
extern const std::string AXESS_USER;
extern const std::string ABACUS_USER;
extern const std::string SABRE_MULTIHOST_ID;
extern const std::string INFINI_MULTIHOST_ID;
extern const std::string AXESS_MULTIHOST_ID;
extern const std::string ABACUS_MULTIHOST_ID;

// Fare vendor codes
extern const std::string ATPCO_VENDOR_CODE;
extern const std::string SITA_VENDOR_CODE;
extern const std::string MERCH_MANAGER_VENDOR_CODE;
extern const std::string SMF_ABACUS_CARRIER_VENDOR_CODE;
extern const std::string SMF_CARRIER_VENDOR_CODE;
extern const std::string SMFO_VENDOR_CODE;
extern const std::string DFF_VENDOR_CODE;
extern const std::string COS_VENDOR_CODE;

constexpr uint16_t DAYS_PER_WEEK = 7;

constexpr uint16_t INTERNAL_DIAG_RANGE_BEGIN = 100;

constexpr uint16_t INTERNAL_DIAG_RANGE_END = 199;

constexpr uint16_t FARES_DIAG_RANGE_BEGIN = 200;
constexpr uint16_t FARES_DIAG_RANGE_END = 299;

constexpr uint16_t RULES_DIAG_RANGE_BEGIN = 300;
constexpr uint16_t RULES_DIAG_RANGE_END = 399;

constexpr uint16_t BOOKING_CODE_DIAG_RANGE_BEGIN = 400;
constexpr uint16_t BOOKING_CODE_DIAG_RANGE_END = 449;

constexpr uint16_t ROUTING_DIAG_RANGE_BEGIN = 450;
constexpr uint16_t ROUTING_DIAG_RANGE_END = 499;

constexpr uint16_t RULECNTL_DIAG_RANGE_BEGIN = 500;

constexpr uint16_t RULECNTL_DIAG_RANGE_END = 599;

constexpr uint16_t PRICING_DIAG_RANGE_BEGIN = 600;
constexpr uint16_t PRICING_DIAG_RANGE_END = 799;

constexpr uint16_t TAXES_DIAG_RANGE_BEGIN = 800;
constexpr uint16_t TAXES_DIAG_RANGE_END = 849;

constexpr uint16_t FARE_CALC_DIAG_RANGE_BEGIN = 850;
constexpr uint16_t FARE_CALC_DIAG_RANGE_END = 874;

constexpr uint16_t SERVICE_FEES_DIAG_RANGE_BEGIN = 875;
constexpr uint16_t SERVICE_FEES_DIAG_RANGE_END = 887;

constexpr uint16_t BRANDED_FARES_DIAG_RANGE_BEGIN = 888;
constexpr uint16_t BRANDED_FARES_DIAG_RANGE_END = 899;

constexpr uint16_t MIPS_DIAG_RANGE_BEGIN = 970;
constexpr uint16_t MIPS_DIAG_RANGE_END = 999;

constexpr uint16_t ALT_PRICING_DIAG_RANGE_BEGIN = 1000;
constexpr uint16_t ALT_PRICING_DIAG_RANGE_END = 1999;

constexpr Indicator YES = 'Y';
constexpr Indicator NO = 'N';

// Fare Type Designator Diagnostic values

extern const std::string FD_FIRST;
extern const std::string FD_BUSINESS;
extern const std::string FD_ECONOMY;
extern const std::string FD_EXCURSION;
extern const std::string FD_ONEWAY_ADVANCE_PURCHASE;
extern const std::string FD_ROUNDTRIP_ADVANCE_PURCHASE;
extern const std::string FD_ONEWAY_INSTANT_PURCHASE;
extern const std::string FD_ROUNDTRIP_INSTANT_PURCHASE;
extern const std::string FD_SPECIAL;
extern const std::string FD_ADDON;
extern const std::string FD_NET;
extern const std::string FD_PROMOTIONAL;
extern const std::string FD_PREMIUM_FIRST;
extern const std::string FD_PREMIUM_ECONOMY;

extern const std::string BSRDSP_KEYWORD;

// Record2 Info

extern const std::string FB_FARE_RULE_RECORD_2;
extern const std::string FB_FOOTNOTE_RECORD_2;
extern const std::string FB_GENERAL_RULE_RECORD_2;

// Return All Data indicator(returnAllData)

constexpr Indicator NORMAL = '1';
constexpr Indicator GDS = '2';
constexpr Indicator WD = '3';
constexpr Indicator FP = '4';
constexpr Indicator NCB = '5';

// record inhibits

constexpr Indicator INHIBIT_N = 'N'; // allowed
constexpr Indicator INHIBIT_D = 'D'; // only allowed for fare display
constexpr Indicator INHIBIT_F = 'F'; // failed (must no be used anywhere)

// Allow or disallow Equinox Negotiated Passenger Types for Fare Collection

constexpr Indicator NEG_FARES_PERMITTED_NO_MESSAGE = '1';
constexpr Indicator NEG_FARES_PERMITTED_WITH_MESSAGE = '2';

// CAT 31 and QREX refund request types
extern const std::string AUTOMATED_REISSUE; // used by cat 31 only
extern const std::string FULL_EXCHANGE; // non-cat31 QREX
extern const std::string PARTIAL_EXCHANGE; // non-cat31 QREX
extern const std::string MULTIPLE_EXCHANGE; // non-cat31 QREX
extern const std::string TAG_10_EXCHANGE; // non-cat31 QREX
extern const std::string AGENT_PRICING_MASK; // non-cat31 QREX
extern const std::string AUTOMATED_REFUND; // used by cat 33 only
extern const std::string NON_FLIGHT_EXCHANGE; // non-cat31 QREX
extern const std::string TX_TAX_INFO_REQUEST; // only tax reissue information

// CAT 31 and QREX Exchange vs Reissue
constexpr Indicator REISSUE = '1';
constexpr Indicator EXCHANGE = '2';

// WQ NEG mapping values
constexpr Indicator MAP_ADT_TO_NEG = '1';
constexpr Indicator MAP_ALL_PTC_TO_NEG = '2';
constexpr Indicator NEVER_MAP_TO_NEG = '3';

// YQYR Fee codes
extern const TaxCode YQF;
extern const TaxCode YQI;
extern const TaxCode YRF;
extern const TaxCode YRI;

// Account code maximum length
constexpr uint16_t ACCOUNT_CODE_LEN = 20;

// Ticketing Fee fare basis match indicator
constexpr Indicator TKT_ALL = 'A';
constexpr Indicator TKT_ANY = 'S';
constexpr Indicator TKT_FREQ_FLYER = 'F';

// Optional services fltTktMerch indicator
constexpr Indicator FLIGHT_RELATED_SERVICE = 'F';
constexpr Indicator TICKET_RELATED_SERVICE = 'T';
constexpr Indicator MERCHANDISE_SERVICE = 'M';
constexpr Indicator RULE_BUSTER_SERVICE = 'R';
constexpr Indicator PREPAID_BAGGAGE = 'P';
constexpr Indicator BAGGAGE_ALLOWANCE = 'A';
constexpr Indicator CARRY_ON_ALLOWANCE = 'B';
constexpr Indicator BAGGAGE_CHARGE = 'C';
constexpr Indicator BAGGAGE_EMBARGO = 'E';

// Other baggage related constants
constexpr uint32_t MAX_BAG_PIECES = 4;
extern const BaggageProvisionType CARRY_ON_CHARGE;

constexpr uint64_t PRODUCT_ID_4 = 4;

extern const std::string BATCHCI_FROM_VENDR_FWS;

// Pricing Orchestrator request path
extern const std::string PSS_PO_ATSE_PATH;
extern const std::string SWS_PO_ATSE_PATH;
extern const std::string LIBERTY_PO_ATSE_PATH;
extern const std::string AEBSO_PO_ATSE_PATH;
extern const std::string ACS_PO_ATSE_PATH;
extern const std::string ANCS_PO_ATSE_PATH;
extern const std::string UNKNOWN_PATH;

extern const std::string LIBERTY_ATSE_PATH;

// Fare Basis Code usage constants
constexpr char FILTER_FBC = 'F';
constexpr char BRANDED_FBC = 'B';
constexpr char COMMAND_PRICE_FBC = 'C';

// Segments Original Id defaults
constexpr uint16_t TRAVEL_SEG_DEFAULT_ID = 65535;
constexpr uint16_t ARUNK_SEG_DEFAULT_ID = 65534;

// consts for OptimusNvbNva
constexpr Indicator NVB_EMPTY = ' ';
constexpr Indicator NVB_1ST_SECTOR = 'S';
constexpr Indicator NVB_1ST_INTL_SECTOR = 'I';
constexpr Indicator NVB_ENTIRE_OUTBOUND = 'O';
constexpr Indicator NVB_ENTIRE_JOURNEY = 'J';
constexpr Indicator NVA_EMPTY = ' ';
constexpr Indicator NVA_1ST_SECTOR_EARLIEST = '1';
constexpr Indicator NVA_1ST_INTL_SECTOR_EARLIEST = '2';
constexpr Indicator NVA_ENTIRE_OUTBOUND_EARLIEST = '3';

extern const RuleNumber RULENUM_BLANK;
extern const RuleNumber ANY_RULE;

// Shopping delay fare validation
constexpr uint32_t DELAY_VALIDATION_OFF = 10000;

constexpr int32_t SECONDS_IN_MINUTE = 60;

// Special usage codes
extern const Code<1> BLANK_CODE;
extern const Code<1> NULL_CODE;

extern const BookingCode DUMMY_BOOKING;
extern const BookingCode ALL_BOOKING;

// TravelSegments attributes
constexpr uint16_t TRIP_BEGIN = 0x0001;
constexpr uint16_t TRIP_END = 0x0002;
constexpr uint16_t ARUNK = 0x0004;
constexpr uint16_t OPEN = 0x0008;
constexpr uint16_t SIDE_TRIP_BEGIN = 0x0010;
constexpr uint16_t SIDE_TRIP_ELEMENT = 0x0020;
constexpr uint16_t SIDE_TRIP_END = 0x0040;
constexpr uint16_t FORCED_SIDE_TRIP_ELEMENT = 0x0080;
constexpr uint16_t FORCED_FARE_BREAK = 0x0100;
constexpr uint16_t FORCED_NO_FARE_BREAK = 0x0200;

constexpr char IBF_EC_NOT_OFFERED = 'O';
constexpr char IBF_EC_NOT_AVAILABLE = 'A';
constexpr char IBF_EC_NO_FARE_FOUND = 'F';
constexpr char IBF_EC_NO_FARE_FILED = 'X';
constexpr char IBF_EC_NOT_SET = 'N';
constexpr char IBF_EC_EARLY_DROP = 'E';

extern const std::string ONE_WORLD_ALLIANCE;
extern const std::string STAR_ALLIANCE;
extern const std::string SKY_TEAM_ALLIANCE;

// Flex Fare
constexpr char FLEX_FARE_GROUP_NOT_OFFERED = 'O';

constexpr int16_t ARUNK_PNR_SEGMENT_ORDER = 255;
//OB Ticketing Fees
extern const std::string ANY_CREDIT;
extern const std::string ANY_DEBIT;
constexpr uint16_t FOP_BIN_SIZE = 6;
extern const std::string CASH;
extern const std::string CREDIT_CARD;
extern const std::string CHECK;
extern const std::string GTR;
extern const std::string DELIMITER;

// SOL
constexpr size_t SOL_MAX_LEGS = 2u;

// AB 240
extern const std::string POSTTKT;
extern const std::string PRETKT;

extern const std::string BR_CLIENT_PBB;
extern const std::string BR_CLIENT_MIP;
extern const std::string BR_CLIENT_FQ;
extern const std::string BR_CLIENT_SHOPPING;

extern const std::string BR_ACTION_SHOPPING;
extern const std::string BR_ACTION_FQ;

extern const std::string THROTTLING_CONCURRENCY;
extern const std::string THROTTLING_PCC;
extern const std::string THROTTLING_LNIATA;
extern const std::string THROTTLING_PNR;
extern const std::string THROTTLING_PART;
extern const std::string THROTTLING_SEG;
extern const std::string THROTTLING_SVC;
extern const std::string THROTTLING_MKT;

constexpr size_t MAX_PAX_COUNT = 4;
constexpr size_t DISPLAY_MAX_SIZE = 63;
extern const std::string NO_SETTLEMENTPLAN;

constexpr TariffCategory PRIVATE_TARIFF = 1;
constexpr TariffCategory PUBLIC_TARIFF = 0;
} // namespace tse
