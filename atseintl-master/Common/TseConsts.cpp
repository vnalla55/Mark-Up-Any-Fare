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
#include "Common/TseConsts.h"

namespace tse
{
const std::string US_DOLLARS = "USD";
const VendorCode EMPTY_VENDOR;

// Consts representing invalid values
const FareType INVALID_FARETYPE = "---";
const FareType DUMMY_FARE_TYPE = "EIP";
const UserApplCode INVALID_USERAPPLCODE = "---";
const NationCode INVALID_NATIONCODE = "----";
const CurrencyCode INVALID_CURRENCYCODE = "---";
const CarrierCode INVALID_CARRIERCODE = "---";
const TaxCode INVALID_TAXCODE = "---";
const LocCode INVALID_LOCCODE = "--------";

const BrandCode NO_BRAND = "_NO_BRAND_";
const BrandCode ANY_BRAND = "_ANY_BRAND_";
const BrandCode ANY_BRAND_LEG_PARITY = "_ANY_BRAND_LEG_PARITY";

const CurrencyCode NUC = "NUC";
const CurrencyCode IDR = "IDR";
const CurrencyCode USD = "USD";

// some particular nations

const NationCode NATION_EMPTY;
const NationCode NATION_INTERNATIONAL = "INTN";
const NationCode NATION_ALL = "ZZ";

const NationCode NATION_US = "US";
const NationCode NATION_CA = "CA";
const NationCode NATION_THAILAND = "TH";
const NationCode NATION_BAHARAIN = "BH";
const NationCode NATION_KUWAIT = "KW";
const NationCode NATION_QUTAR = "QA";
const NationCode NATION_SAUDI_ARABIA = "SA";
const NationCode NATION_UNITED_ARAB_EMIRATES = "AE";
const NationCode NATION_FRANCE = "FR";
const NationCode NATION_MONACO = "MC";
const NationCode NATION_USSR = "RU";
const NationCode NATION_EUR = "XU"; // East Ural Russia
const NationCode NATION_ECUADOR = "EC";
const NationCode NATION_BRAZIL = "BR";
const NationCode NATION_PANAMA = "PA";
const NationCode NATION_ISRAEL = "IL";

const std::string ANY_POSSIBLE_ROUTING = "9999";
const std::string IATA_AREA1 = "1"; // IATA
const std::string IATA_AREA2 = "2"; // IATA
const std::string IATA_AREA3 = "3"; // IATA

const NationCode UNITED_STATES = "US";
const NationCode ALASKA = "USAK"; // state code
const NationCode HAWAII = "USHI"; // state code
const NationCode CANADA = "CA";
const NationCode NEW_BRUNSWICK = "CANB";
const NationCode NOVA_SCOTIA = "CANS";
const NationCode NEW_FOUNDLAND = "CANF";
const NationCode COLUMBIA = "CO";
const NationCode MEXICO = "MX";
const NationCode RUSSIA = "RU";
const NationCode EAST_URAL_RUSSIA = "XU";
const NationCode AUSTRALIA = "AU";
const NationCode NEW_ZEALAND = "NZ";
const NationCode NIGERIA = "NG";
const NationCode PERU = "PE";
const NationCode ST_PIERRE_MIQUELON = "PM";

// European Countries

const NationCode UNITED_KINGDOM = "GB";
const NationCode AUSTRIA = "AT";
const NationCode FRANCE = "FR";
const NationCode GERMANY = "DE";
const NationCode SPAIN = "ES";

// Asia Countries

const NationCode JAPAN = "JP";
const NationCode INDONESIA = "ID";
const NationCode KOREA = "KR";

// US Possesions

const NationCode VIRGIN_ISLAND = "USVI";
const NationCode PUERTORICO = "USPR";
const NationCode GUAM = "GU";
const NationCode AMERICAN_SAMOA = "AS";

const NationCode CANTON_AND_ENDERBURY = "CT";
const NationCode MIDWAY = "MI";
const NationCode WAKE = "WK";
const NationCode MICRONESIA = "FM";
const NationCode SAIPAN = "MP";
const NationCode NORTHERN_MARIANA = "MH";

const NationCode NORTH_AMERICA = "11"; // subIATA
const NationCode EUROPE = "21"; // SubIATA

// scandinavian countries

const NationCode
DENMARK("DK");

const NationCode
NORWAY("NO");

const NationCode
SWEDEN("SE");

const NationCode
GREENLAND("GL");

const NationCode
WILDCARD('?');

// Pseudo City Codes
const PseudoCityCode TRAVELOCITY = "W0H3"; //  mainTvlAgencyPCC for Travelocity

const PseudoCityType PCCTYPE_BRANCH = 'T'; // Branch Pseudo City
const PseudoCityType PCCTYPE_HOME = 'U'; // Home Pseudo City

// Loc

const LocCode LOC_SIN = "SIN";
const LocCode LOC_BWN = "BWN";
const LocCode LOC_KUL = "KUL";
const LocCode LOC_KCH = "KCH";
const LocCode LOC_BKI = "BKI";
const LocCode LOC_YND = "YND";
const LocCode LOC_YMQ = "YMQ";
const LocCode LOC_YQB = "YQB";
const LocCode LOC_YUL = "YUL";
const LocCode EAP_BASEL = "EAP";
const LocCode BSL_BASEL = "BSL";
const LocCode LOC_NYC = "NYC";
const LocCode LOC_EWR = "EWR";
const LocCode LOC_WAS = "WAS";
const LocCode LOC_BWI = "BWI";
const LocCode LOC_NRT = "NRT";

// General City Codes
const LocCode EastCoastCode = "ECC";
const LocCode WestCoastCode = "WCC";

// Loc in Spanish Oversea Islands
const LocCode LOC_TCI = "TCI";
const LocCode LOC_LPA = "LPA";
const LocCode LOC_FUE = "FUE";
const LocCode LOC_ACE = "ACE";
const LocCode LOC_PMI = "PMI";
const LocCode LOC_MAH = "MAH";
const LocCode LOC_IBZ = "IBZ";
const LocCode LOC_MLN = "MLN";
const LocCode LOC_JCU = "JCU";
const LocCode LOC_SPC = "SPC";
const LocCode LOC_RCE = "RCE";
const LocCode LOC_TFN = "TFN";
const LocCode LOC_TFS = "TFS";

const LocCode LOC_AGP = "AGP";
const LocCode LOC_XRY = "XRY";
const LocCode LOC_SVQ = "SVQ";

// States of Spanish Oversea Islands
const StateCode ST_RBP = "RBP";
const StateCode ST_RRC = "RRC";
const StateCode ST_RRM = "RRM";
const StateCode ST_RCE = "RCE";

const std::string FR_TYPE = "*FR";
const std::string BJ_TYPE = "*BJ";
const std::string EW_TYPE = "*EW";

// Carrier
const CarrierCode
ANY_CARRIER("**");

const CarrierCode
EMPTY_CARRIER(" ");

const CarrierCode
INDUSTRY_CARRIER("YY");

const CarrierCode
BAD_CARRIER("XX");

const CarrierCode
INBOUND_CARRIER('I');

const CarrierCode
JOINT_CARRIER("*J");

const CarrierCode
DOLLAR_CARRIER("$$");

const CarrierCode
XDOLLAR_CARRIER("X$");

const CarrierCode
MH_CARRIER("MH");

const CarrierCode
SPECIAL_CARRIER_AA("AA");

const CarrierCode
CARRIER_9B("9B");

const CarrierCode
CARRIER_2R("2R");

const CarrierCode
CARRIER_JJ("JJ");

const CarrierCode
CARRIER_WS("WS");

const CarrierCode
CARRIER_AM("AM");

const CarrierCode
CARRIER_DL("DL");

const CarrierCode
CARRIER_AIRSEYCHELLES("HM");

const EquipmentType TRAIN = "TRN";
const EquipmentType TGV = "TGV";
const EquipmentType BUS = "BUS";
const EquipmentType BOAT = "LCH";
const EquipmentType ICE = "ICE";
const EquipmentType LMO = "LMO";
const EquipmentType TRS = "TRS";

const PaxTypeCode ADULT = "ADT";
const PaxTypeCode CHILD = "CNN";
const PaxTypeCode INFANT = "INF";
const PaxTypeCode CNE = "CNE";
const PaxTypeCode NEG = "NEG";
const PaxTypeCode INE = "INE";
const PaxTypeCode CBC = "CBC";
const PaxTypeCode PFA = "PFA";
const PaxTypeCode CBI = "CBI";
const PaxTypeCode JNN = "JNN";
const PaxTypeCode JCB = "JCB";
const PaxTypeCode JNF = "JNF";
const PaxTypeCode JNS = "JNS";
const PaxTypeCode INS = "INS";
const PaxTypeCode UNN = "UNN";
const PaxTypeCode MIL = "MIL";
const PaxTypeCode SRC = "SRC";
const PaxTypeCode SNN = "SNN";
const PaxTypeCode ADR = "ADR";
const PaxTypeCode CHR = "CHR";
const PaxTypeCode INR = "INR";
const PaxTypeCode ISR = "ISR";
const PaxTypeCode UNR = "UNR";
const PaxTypeCode FLY = "FLY";
const PaxTypeCode FNN = "FNN";

const PaxTypeCode CTZ = "CTZ";
const PaxTypeCode GCF = "GCF";
const PaxTypeCode GCT = "GCT";
const PaxTypeCode GDP = "GDP";
const PaxTypeCode GEX = "GEX";
const PaxTypeCode GST = "GST";
const PaxTypeCode GVT = "GVT";
const PaxTypeCode NAT = "NAT";
const PaxTypeCode GV1 = "GV1";
const PaxTypeCode LTC = "LTC";
const PaxTypeCode SEA = "SEA";

const PaxTypeCode AST = "AST";
const PaxTypeCode CSB = "CSB";
const PaxTypeCode EMPTY_PAXTYPE_CODE = "   ";

const std::string CONFIRM_RES_STATUS = "OK";
const std::string NOSEAT_RES_STATUS = "NS";
const std::string QF_RES_STATUS = "QF";
const std::string RQ_RES_STATUS = "RQ";
// Waitlist codes - LL, BL, DL, GL, HL, HN, PN, TL, WL and DS
const std::string LL_RES_STATUS = "LL";
const std::string BL_RES_STATUS = "BL";
const std::string DL_RES_STATUS = "DL";
const std::string GL_RES_STATUS = "GL";
const std::string HL_RES_STATUS = "HL";
const std::string HN_RES_STATUS = "HN";
const std::string PN_RES_STATUS = "PN";
const std::string TL_RES_STATUS = "TL";
const std::string WL_RES_STATUS = "WL";

const std::string DS_REAL_RES_STATUS = "DS";

// Zone

const Zone SABRE_WESTERN_AFRICA = "1100";
const Zone SABRE_EU_MEMBER_STATE = "1101";

// Multi Hosts
const std::string SABRE_USER = "SABR";
const std::string INFINI_USER = "INFI";
const std::string AXESS_USER = "AXES";
const std::string ABACUS_USER = "ABAC";
const std::string SABRE_MULTIHOST_ID = "1S";
const std::string INFINI_MULTIHOST_ID = "1F";
const std::string AXESS_MULTIHOST_ID = "1J";
const std::string ABACUS_MULTIHOST_ID = "1B";

// Fare vendor codes
const std::string ATPCO_VENDOR_CODE = "ATP";
const std::string SITA_VENDOR_CODE = "SITA";
const std::string MERCH_MANAGER_VENDOR_CODE = "MMGR";
const std::string SMF_ABACUS_CARRIER_VENDOR_CODE = "SMFA";
const std::string SMF_CARRIER_VENDOR_CODE = "SMFC";
const std::string SMFO_VENDOR_CODE = "SMFO";
const std::string DFF_VENDOR_CODE = "DFF";
const std::string COS_VENDOR_CODE = "COS";

// Fare Type Designator Diagnostic values

const std::string FD_FIRST = "FI";
const std::string FD_BUSINESS = "BU";
const std::string FD_ECONOMY = "EC";
const std::string FD_EXCURSION = "EX";
const std::string FD_ONEWAY_ADVANCE_PURCHASE = "OAP";
const std::string FD_ROUNDTRIP_ADVANCE_PURCHASE = "RAP";
const std::string FD_ONEWAY_INSTANT_PURCHASE = "OIP";
const std::string FD_ROUNDTRIP_INSTANT_PURCHASE = "RIP";
const std::string FD_SPECIAL = "SP";
const std::string FD_ADDON = "AD";
const std::string FD_NET = "NET";
const std::string FD_PROMOTIONAL = "PR";
const std::string FD_PREMIUM_FIRST = "PF";
const std::string FD_PREMIUM_ECONOMY = "PE";

const std::string BSRDSP_KEYWORD = "BSRDSP";

// Record2 Info

const std::string FB_FARE_RULE_RECORD_2 = "FARE RULE RECORD 2";
const std::string FB_FOOTNOTE_RECORD_2 = "FOOTNOTE RECORD 2";
const std::string FB_GENERAL_RULE_RECORD_2 = "GENERAL RULE RECORD 2";

// CAT 31 and QREX refund request types
const std::string AUTOMATED_REISSUE = "AR"; // used by cat 31 only
const std::string FULL_EXCHANGE = "FE"; // non-cat31 QREX
const std::string PARTIAL_EXCHANGE = "PE"; // non-cat31 QREX
const std::string MULTIPLE_EXCHANGE = "ME"; // non-cat31 QREX
const std::string TAG_10_EXCHANGE = "CE"; // non-cat31 QREX
const std::string AGENT_PRICING_MASK = "AM"; // non-cat31 QREX
const std::string AUTOMATED_REFUND = "AF"; // used by cat 33 only
const std::string NON_FLIGHT_EXCHANGE = "NF"; // non-cat31 QREX
const std::string TX_TAX_INFO_REQUEST = "TX"; // only tax reissue information

// YQYR Fee codes
const TaxCode YQF = "YQF";
const TaxCode YQI = "YQI";
const TaxCode YRF = "YRF";
const TaxCode YRI = "YRI";

// Other baggage related constants
const BaggageProvisionType CARRY_ON_CHARGE = "CC";

const std::string BATCHCI_FROM_VENDR_FWS = "W";

// Pricing Orchestrator request path
const std::string PSS_PO_ATSE_PATH = "PPSS";
const std::string SWS_PO_ATSE_PATH = "PSWS";
const std::string LIBERTY_PO_ATSE_PATH = "PLIB";
const std::string AEBSO_PO_ATSE_PATH = "PBSO";
const std::string ACS_PO_ATSE_PATH = "PACS";
const std::string ANCS_PO_ATSE_PATH = "PANC";
const std::string UNKNOWN_PATH = "UNKN";

const std::string LIBERTY_ATSE_PATH = "LBTY";

const RuleNumber RULENUM_BLANK = "    ";
const RuleNumber ANY_RULE = "9999";

// Special usage codes
const Code<1>
BLANK_CODE(' ');

const Code<1>
NULL_CODE((const char)0);

const BookingCode
DUMMY_BOOKING('0');

const BookingCode
ALL_BOOKING('*');

// TravelSegments attributes

const std::string ONE_WORLD_ALLIANCE = "*O";
const std::string STAR_ALLIANCE = "*A";
const std::string SKY_TEAM_ALLIANCE = "*S";

// OB Ticketing Fees
const std::string ANY_CREDIT = "FCA";
const std::string ANY_DEBIT = "FDA";

const std::string CASH = "CA";
const std::string CREDIT_CARD = "CC";
const std::string CHECK = "CK";
const std::string GTR = "GTR";
const std::string DELIMITER = "/";

// AB 240
const std::string POSTTKT = "POST";
const std::string PRETKT = "PRET";

const std::string BR_CLIENT_PBB = "PBB";
const std::string BR_CLIENT_MIP = "MIP";
const std::string BR_CLIENT_FQ = "FQ";
const std::string BR_CLIENT_SHOPPING = "SHP";

const std::string BR_ACTION_SHOPPING = "SHP";
const std::string BR_ACTION_FQ = "FQ";

const std::string THROTTLING_CONCURRENCY = "CON";
const std::string THROTTLING_PCC = "PCC";
const std::string THROTTLING_LNIATA = "LNIATA";
const std::string THROTTLING_PNR = "PNR";
const std::string THROTTLING_PART = "PART";
const std::string THROTTLING_SEG = "SEG";
const std::string THROTTLING_SVC = "SVC";
const std::string THROTTLING_MKT = "MKT";
const std::string NO_SETTLEMENTPLAN = "NSP";
} // namespace tse
