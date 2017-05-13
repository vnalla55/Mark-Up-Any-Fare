//-------------------------------------------------------------------
//
//  File:        Message.cpp
//  Created:     Feb 8, 2006
//  Authors:     Andrea Yang
//
//  Description: Mapping ATSE V2 Error Code to Legacy Error Code for
//               Ticketing
//
//  Updates:
//
//  Copyright Sabre 2006
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

#include "Common/ErrorResponseException.h"
#include "Common/Message.h"

namespace tse
{

std::map<ErrorResponseException::ErrorResponseCode, uint16_t> Message::_msgErrCodeMap;

uint16_t
Message::errCode(const ErrorResponseException::ErrorResponseCode& errCode)
{
  return _msgErrCodeMap[errCode];
}

// Match ErrorResponseCode to Legacy Pricing Fail Code
void
Message::fillMsgErrCodeMap()
{
  // Note!!! Error Code with 0 is place holder for unmatched from PSS error or
  // not sure if PSS uses it. Error Code range in PSS is 0 to 255.

  //
  // FQ / Fare Display codes start at 4000
  //
  _msgErrCodeMap[ErrorResponseException::NO_QUOTABLE_FARES] = 0; //"NO QUOTABLE FARES FOUND";
  _msgErrCodeMap[ErrorResponseException::INVALID_INCLUSION_CODE] = 0; //"$INVALID INCLUSION CODE$";
  _msgErrCodeMap[ErrorResponseException::INVALID_PAX_TYPE_CODE] = 0; //"$FORMAT-PSGR TYPE$";
  _msgErrCodeMap[ErrorResponseException::INVALID_WEB_MULTICARRIER] =
      0; //"$WEB REQUEST MUST BE SINGLE CARRIER$"
  _msgErrCodeMap[ErrorResponseException::INVALID_WEB_AND_PAXTYPE] =
      0; //"$PASSENGER TYPE NOT ALLOWED WITH WEB INCLUSION CODE$"
  _msgErrCodeMap[ErrorResponseException::PRCRD_FARE_BASIS_NOT_FOUND] =
      0; //$FARE BASIS NOT FOUND FOR CITYPAIR/DATE$;
  _msgErrCodeMap[ErrorResponseException::LONGRD_FARE_BASIS_NOT_FOUND] =
      0; //$FARE BASIS NOT FOUND FOR CITYPAIR/DATE$;
  _msgErrCodeMap[ErrorResponseException::SHORTRD_FARE_BASIS_NOT_FOUND] =
      0; //$FARE BASIS NOT FOUND FOR CITYPAIR/DATE$;
  _msgErrCodeMap[ErrorResponseException::FARE_BASIS_NOT_FOUND] =
      0; //$FARE BASIS NOT FOUND FOR CITYPAIR/DATE$;

  //
  // WP / Pricing codes start at 5000
  //
  _msgErrCodeMap[ErrorResponseException::NO_FARE_REQUESTED] = 2; //"NO FARE REQUESTED";
  _msgErrCodeMap[ErrorResponseException::PRICING_REST_BY_GOV] =
      5; //"PRICING RESTRICTED BY GOVT ORDER";
  _msgErrCodeMap[ErrorResponseException::NO_FARE_FOR_CLASS_USED] = 11; //"NO FARE FOR CLASS USED";
  _msgErrCodeMap[ErrorResponseException::CITY_PAIR_RESTRICTED_BY_FBM] =
      12; //"CITY PAIR RESTRICTED BY FBM";
  _msgErrCodeMap[ErrorResponseException::CANNOT_COMPUTE_TAX] = 13; //"UNABLE TO COMPUTE TAX";
  _msgErrCodeMap[ErrorResponseException::AIRPORT_CODE_NOT_IN_SYS] = 17; //"AIRPORT CODE NOT IN SYS";
  _msgErrCodeMap[ErrorResponseException::FARE_NOT_IN_SYS] = 18; //"FARE NOT IN SYSTEM";
  _msgErrCodeMap[ErrorResponseException::CHECK_DATE_TIME_CONTINUITY_OF_FLIGHTS] =
      19; //"CHECK DATE/TIME CONTINUITY OF FLIGHTS";
  _msgErrCodeMap[ErrorResponseException::JT_RESTRICTED_RTG_INVALID] =
      20; //"JT RESTRICTED RTG INVALID";
  _msgErrCodeMap[ErrorResponseException::TRIP_EXCEEDS_MPM_SOM] = 23; //"TRIP EXCEEDS MPM SOM";
  _msgErrCodeMap[ErrorResponseException::MAX_NUMBER_COMBOS_EXCEEDED] =
      25; //"MAX NBR COMBINATIONS EXCEEDED/USE SEGMENT SELECT";
  _msgErrCodeMap[ErrorResponseException::FREE_SURCHARGED_STOPOVER_MAY_APPLY] =
      26; //"FREE/SURCHARGED STOPOVER/S/ MAY APPLY";
  _msgErrCodeMap[ErrorResponseException::TOO_MANY_SURFACE_SEGS] = 29; //"TOO MANY SURFACE SEGS";
  _msgErrCodeMap[ErrorResponseException::NO_DATE_IN_OPEN_SEG] = 30; //"NO DATE IN OPEN SEGMENT";
  _msgErrCodeMap[ErrorResponseException::CHECK_LINE_OF_FLIGHT] = 31; //"CHECK LINE OF FLIGHT";

  _msgErrCodeMap[ErrorResponseException::STOPOVER_SURCHARGE_MAY_APPLY] =
      32; //"STOPOVER SURCHARGE MAY APPLY";
  _msgErrCodeMap[ErrorResponseException::NO_RULES_FOR_PSGR_TYPE_OR_CLASS] =
      33; //"NO RULES VALID FOR PASSENGER TYPE/CLASS OF SERVICE";
  _msgErrCodeMap[ErrorResponseException::NO_THRU_CLASS_FOR_ENROUTE_CHANGE_FLT] =
      34; //"UNABLE TO FIND THRU CLASS FOR ENROUTE CHANGE FLT";
  _msgErrCodeMap[ErrorResponseException::FIRST_SEG_OPEN] = 35; //"1ST SEG OPEN";
  _msgErrCodeMap[ErrorResponseException::RETRY_IN_ONE_MINUTE] = 36; //"RETRY IN ONE MINUTE";
  _msgErrCodeMap[ErrorResponseException::NO_TIMES_IN_OA_SEG] = 37; //"NO TIMES IN OA SEG";
  _msgErrCodeMap[ErrorResponseException::SYSTEM_ERROR] = 38; //"SYSTEM ERROR";
  _msgErrCodeMap[ErrorResponseException::FARE_RESTRICTED_FROM_PRICING] =
      41; //"FARE RSTRCTD FRM PRICING-MAY APPLY";
  _msgErrCodeMap[ErrorResponseException::INVALID_ROUTING_OR_SEG_BETWEEN_CO_TERM] =
      43; //"INVALID RTG OR SEG BTWN CO-TERM";
  _msgErrCodeMap[ErrorResponseException::PRICING_AT_PEAK_USAGE] = 100; //"UTP-REPEAT ENTRY";
  _msgErrCodeMap[ErrorResponseException::MIXED_CLASS_TRIP] = 63; //"MIXED CLS TRIP-PRICING RSTRCTD";
  _msgErrCodeMap[ErrorResponseException::MAX_PERMITTED_MILEAGE_NOT_AVAIL] =
      46; //"MAXIMUM PERMITTED MILEAGE NOT AVAILABLE";
  _msgErrCodeMap[ErrorResponseException::MAX_SEGS_EXCEEDED] =
      47; //"MAXIMUM EIGHT SEGMENTS FOR ENTRY";
  _msgErrCodeMap[ErrorResponseException::TOTAL_FARE_TOO_LARGE] = 49; //"TOTAL FARE TOO LARGE";
  _msgErrCodeMap[ErrorResponseException::SYSTEM_DATA_ERROR] = 50; //"SYSTEM DATA ERROR";
  _msgErrCodeMap[ErrorResponseException::FAILED_DUE_TO_COMBO_RESTRICTIONS] =
      0; //"NO PROMO FARE ON TRIP BASIS";
  _msgErrCodeMap[ErrorResponseException::NO_COMBINABLE_FARES_FOR_CLASS] =
      53; //"NO COMBINABLE FARES FOR CLASS USED";
  _msgErrCodeMap[ErrorResponseException::FARE_BASIS_EXCEED_7CHAR_FOR_TAT_TM] =
      54; //"FARE BASIS EXCEED 7CHAR FOR TAT/TM";
  _msgErrCodeMap[ErrorResponseException::MAX_COMBOS_EXCEEDED_FOR_AUTO_PRICING] =
      56; //"MAX COMBINATIONS EXCEEDED FOR AUTO PRICING";
  _msgErrCodeMap[ErrorResponseException::CANNOT_FORMAT_TAX] =
      57; //"UNABLE TO CALCULATE TAX - APPLY MANUALLY";
  _msgErrCodeMap[ErrorResponseException::NBR_PSGRS_EXCEEDS_OA_AVAIL] =
      59; //"CPA IS RESTRICTED BY FBM";
  _msgErrCodeMap[ErrorResponseException::NO_FARE_VALID_FOR_PSGR_TYPE] =
      60; //"NO FARE VALIOD FOR PSGR TYPE";
  _msgErrCodeMap[ErrorResponseException::EXCEEDS_ALLOWED_SEGS_CHANGED_CLASS_FLIGHTS] =
      62; //"EXCEEDS ALLOWABLE SEGS DUE TO CHG IN CLS FLTS";
  _msgErrCodeMap[ErrorResponseException::CHECK_FLIGHT] = 64; //"CHECK FLIGHT";
  _msgErrCodeMap[ErrorResponseException::FARE_CALC_TOO_LARGE_FOR_ATB] =
      91; //"FARE CALC TOO LARGE FOR ATB";
  _msgErrCodeMap[ErrorResponseException::CANNOT_CALC_SD_FARE] = 92; //"CANNOT CALC SD FARE";
  _msgErrCodeMap[ErrorResponseException::TICKET_DESIGNATOR_NOT_ALLOWED] =
      94; //"TICKET DESIGNATOR NOT ALLOWED";
  _msgErrCodeMap[ErrorResponseException::WPNJ_NOT_VALID_ON_THIS_ITIN] =
      95; //"WPNJ NOT VALID ON THIS ITIN";
  _msgErrCodeMap[ErrorResponseException::CANNOT_CALCULATE_CURRENCY] =
      96; //"CANNOT CALCULATE CURRENCY";
  _msgErrCodeMap[ErrorResponseException::CANNOT_ROUND_CURRENCY] = 97; //"CANNOT ROUND CURRENCY";
  _msgErrCodeMap[ErrorResponseException::BREAK_FARE_INVALID] = 99; //"BREAK FARE INVALID";
  _msgErrCodeMap[ErrorResponseException::RETRY] = 100; //"RETRY";
  _msgErrCodeMap[ErrorResponseException::NEED_COMMISSION] = 101; //"NEED COMMISSION";
  _msgErrCodeMap[ErrorResponseException::OPEN_JAW_MAY_APPLY] = 104; //"OPEN JAW MAY APPLY";
  _msgErrCodeMap[ErrorResponseException::FLT_CHK_DATE_TIME_CONTINUITY_OF_FLTS] =
      105; //"FLT CHK DATE TIME CONTINUITY OF FLTS";
  _msgErrCodeMap[ErrorResponseException::FARE_BASIS_NOT_AVAIL] = 106; //"FARE BASIS NOT AVAIL";
  _msgErrCodeMap[ErrorResponseException::WORLDFARE_AT_PEAK_USE] = 112; //"WORLDFARE AT PEAK USE";
  _msgErrCodeMap[ErrorResponseException::WORLDFARE_UNAVAILABLE] = 113; //"WORLDFARE UNAVAILABLE";
  _msgErrCodeMap[ErrorResponseException::TKT_DES_FAILE_RULES_CHECK] =
      114; //"TKT DES FAILE RULES CHECK";
  _msgErrCodeMap[ErrorResponseException::TKT_DES_RECORD_RETRIEVAL_ERROR] =
      115; //"TKT DES RECORD RETRIEVAL ERROR";
  _msgErrCodeMap[ErrorResponseException::PROCESSING_ERROR_DETECTED] =
      116; //"PROCESSING ERROR DETECTED";
  _msgErrCodeMap[ErrorResponseException::PENALTY_DATA_INCOMPLETE] =
      119; //"PENALTY DATA INCOMPLETE";
  _msgErrCodeMap[ErrorResponseException::LOW_FARE_NOT_ALLOWED_FOR_CLASS_USED] =
      124; //"LOW FARE NOT ALLOWED FOR CLASS USED";
  _msgErrCodeMap[ErrorResponseException::WRONG_NUMBER_OF_FARES] = 0; //"WRONG NUMBER OF FARES";
  _msgErrCodeMap[ErrorResponseException::DISCOUNT_AMT_EXCEEDS_FARE_COMPONENT_TOTAL] =
      126; //"DISCOUNT AMT EXCEEDS FARE COMPONENT TOTAL";
  _msgErrCodeMap[ErrorResponseException::ENTRIES_NOT_COMBINABLE_WITH_DISCOUNT] =
      143; //"ENTRIES NOT COMBINABLE WITH DISCOUNT";
  _msgErrCodeMap[ErrorResponseException::OPEN_RETURN_REQUIRED] = 129; //"OPEN RETURN REQUIRED";
  _msgErrCodeMap[ErrorResponseException::NO_FARE_BASIS_FOR_AIRPORT_PAIR_AGENCY] =
      130; //"NO FARE BASIS FOR AIRPORT PAIR AGENCY";
  _msgErrCodeMap[ErrorResponseException::CANNOT_COMPUTE_TAX_COMPUTE_MANUALLY] =
      131; //"CANNOT COMPUTE TAX COMPUTE MANUALLY";
  _msgErrCodeMap[ErrorResponseException::CANNOT_TAX_INSERT_TAX_AFTER_TX] =
      132; //"CANNOT TAX INSERT TAX AFTER TX";
  _msgErrCodeMap[ErrorResponseException::MULTI_DISCOUNTS_EXIST] = 133; //"MULTI DISCOUNTS EXIST";
  _msgErrCodeMap[ErrorResponseException::NO_VALID_DISCOUNT_PUB_FARE] =
      134; //"NO VALID DISCOUNT PUB FARE";
  _msgErrCodeMap[ErrorResponseException::SCHEDULES_NOT_AVAILABLE_FOR_CARRIER_DATE] =
      136; //"SCHEDULES NOT AVAILABLE FOR CARRIER DATE";
  _msgErrCodeMap[ErrorResponseException::DATA_ERROR_DETECTED] = 137; //"DATA ERROR DETECTED";
  _msgErrCodeMap[ErrorResponseException::NO_PNR_CREATED] = 139; //"NO PNR CREATED";
  _msgErrCodeMap[ErrorResponseException::NO_FARE_FOR_CLASS] = 140; //"NO FARE FOR CLASS";
  _msgErrCodeMap[ErrorResponseException::ErrorResponseCode::CANNOT_PRICE_AS_REQUESTED] =
      142; //"CANNOT PRICE AS REQUESTED";
  _msgErrCodeMap[ErrorResponseException::NO_FARES_RBD_CARRIER] = 146; //"NO FARES RBD CARRIER";
  _msgErrCodeMap[ErrorResponseException::NEGOTIATED_FARES_APPLY] = 148; //"NEGOTIATED FARES APPLY";
  _msgErrCodeMap[ErrorResponseException::NO_FARES_FOUND_FOR_FARE_COMPONENT] =
      150; //"NO FARES FOUND FOR FARE COMPONENT";
  _msgErrCodeMap[ErrorResponseException::CODESHARE_PROCESSING_ERROR] =
      151; //"CODESHARE PROCESSING ERROR";
  _msgErrCodeMap[ErrorResponseException::NO_MATCH_FOR_FARE_COMPONENT] =
      153; //"NO MATCH FOR FARE COMPONENT";
  _msgErrCodeMap[ErrorResponseException::INVALID_FARE_BASIS_FOR_CARRIER] =
      154; //"INVALID FARE BASIS FOR CARRIER";
  _msgErrCodeMap[ErrorResponseException::REQ_CARRIER_HAS_NO_FARES] =
      155; //"REQ CARRIER HAS NO FARES";
  _msgErrCodeMap[ErrorResponseException::EXCEED_LENGTH_UNABLE_TO_CALCULATE_FARE] =
      0; //"EXCEED LENGTH UNABLE TO CALCULATE FARE";
  _msgErrCodeMap[ErrorResponseException::EXCEED_LENGTH_UNABLE_TO_CONVERT_CURRENCY] =
      0; //"EXCEED LENGTH UNABLE TO CONVERT CURRENCY";
  _msgErrCodeMap[ErrorResponseException::NO_APPLICABLE_YY_FARES] =
      152; //"THERE ARE NO APPLICABLE YY FARES";
  _msgErrCodeMap[ErrorResponseException::UNABLE_TO_CALCULATE_BSR_NOT_AVAILABLE] =
      4; //"UNABLE TO CALCULATE - BSR NOT AVAILABLE";
  _msgErrCodeMap[ErrorResponseException::BAD_FARE_PATH_TOTAL_NUC_AMOUNT] =
      0; //"BAD FARE PATH TOTAL NUC AMOUNT";
  _msgErrCodeMap[ErrorResponseException::PU_NOT_ALLOWED_WITH_CAT35_NET_FARES] =
      0; //"PU NOT ALLOWED WITH CAT35 NET FARES";
  _msgErrCodeMap[ErrorResponseException::ENTRY_RESTRICTED_TO_EPR_KEYWORD_MUL375] =
      0; //"ENTRY RESTRICTED TO EPR KEYWORD MUL375";
  _msgErrCodeMap[ErrorResponseException::MCP_IAP_RESTRICTED] =
      0; //"UNABLE TO PROCESS-ENTRY RESTRICTED IN PARTITION ";
  _msgErrCodeMap[ErrorResponseException::FEE_CURRENCY_CONVERSION_FAILED] =
      0; //"FEE CURRENCY CONVERSION FAILED";

  //
  // Negotiated Fare IT/BT ticketing codes start at 5500
  //
  _msgErrCodeMap[ErrorResponseException::ISSUE_SEPARATE_TICKET] = 0; //"ISSUE SEPARATE TICKET";
  _msgErrCodeMap[ErrorResponseException::UTAT_NET_SELLING_AMOUNTS_CONFLICT] =
      0; //"UNABLE TO AUTO TICKET - NET/SELLING AMOUNTS CONFLICT";
  _msgErrCodeMap[ErrorResponseException::UTAT_MULTIPLE_TOUR_CODES] =
      0; //"UNABLE TO AUTO TICKET - MULTIPLE TOUR CODES";
  _msgErrCodeMap[ErrorResponseException::UTAT_TOUR_CODE_NOT_FOUND] =
      0; //"UNABLE TO AUTO TICKET - TOUR CODE NOT FOUND";
  _msgErrCodeMap[ErrorResponseException::UTAT_INVALID_TEXT_BOX_COMBO] =
      0; //"UNABLE TO AUTO TICKET - INVALID TEXT BOX COMBINATION";
  _msgErrCodeMap[ErrorResponseException::UTAT_COMMISSIONS_NOT_COMBINABLE] =
      0; //"UNABLE TO AUTO TICKET - COMMISSIONS NOT COMBINABLE";
  _msgErrCodeMap[ErrorResponseException::NET_REMIT_FARE_PHASE_FOUR] =
      0; //"NET REMIT FARE PHASE FOUR AND USE NET/FOR TKT ISSUANCE";
  _msgErrCodeMap[ErrorResponseException::UNABLE_TO_PROCESS_NEG_FARE_DATA] =
      0; //"UNABLE TO PROCESS NEGOTIATED FARE DATA";
  _msgErrCodeMap[ErrorResponseException::NO_NET_FARE_AMOUNT] = 0; //"NO NET FARE AMOUNT";
  _msgErrCodeMap[ErrorResponseException::INVALID_COMBO_NO_NET_FARE_DISPLAY] =
      0; //"INVALID COMBINATION OF FARES NO NET FARE DISPLAY";
  _msgErrCodeMap[ErrorResponseException::UTAT_MULTIPLE_VALUE_CODES] =
      0; //"UNABLE TO AUTO TICKET - MULTIPLE VALUE CODES";
  _msgErrCodeMap[ErrorResponseException::UTAT_MULTIPLE_PRINT_OPTIONS] =
      0; //"UNABLE TO AUTO TICKET - MULTIPLE PRINT OPTIONS";
  _msgErrCodeMap[ErrorResponseException::FARE_REQUIRE_COMM_PERCENT] =
      0; //"FARE REQUIRES COMMISSION PERCENT";

  //
  // TripSearch / BargainFinder / Low Fare Search codes start at 6000
  //
  _msgErrCodeMap[ErrorResponseException::INVALID_JOURNEY_RECORD] =
      0; //"INVALID-JOURNEY RECORD PRESENT";
  _msgErrCodeMap[ErrorResponseException::INVALID_INPUT] = 0; //"INVALID INPUT FORMAT";
  _msgErrCodeMap[ErrorResponseException::INVALID_DUTY_CODE] = 0; //"INVALID DUTY CODE";
  _msgErrCodeMap[ErrorResponseException::NO_ITIN_SEGS_FOUND] = 0; //"NO ITINERARY SEGMENTS FOUND";
  _msgErrCodeMap[ErrorResponseException::PNR_DATABASE_ERROR] = 0; //"PNR DATA BASE ERROR";
  _msgErrCodeMap[ErrorResponseException::NEED_PREFERRED_CARRIER] = 0; //"NEED PREFERRED CARRIER";
  _msgErrCodeMap[ErrorResponseException::MAX_PREF_CARRIERS_EXCEEDED] =
      0; //"SEE CTP-MORE THAN 3 PREF CARRIERS";
  _msgErrCodeMap[ErrorResponseException::MAX_PASSENGERS_EXCEEDED] =
      0; //"MAXIMUM PASSENGERS ALLOWED IS 8";
  _msgErrCodeMap[ErrorResponseException::INVALID_SAME_PREF_AND_NON_PREF_CARRIER] =
      0; //"INVALID-SAME PREF/NON-PREF CARRIER";
  _msgErrCodeMap[ErrorResponseException::ENTER_ACTIVE_SEGS] = 0; //"ENTER ACTIVE SEGMENTS";
  _msgErrCodeMap[ErrorResponseException::CONFLICTING_OPTIONS] = 0; //"CONFLICTING OPTIONS SEE *";
  _msgErrCodeMap[ErrorResponseException::INVALID_SEG_TYPE_IN_PNR_RETIEVAL] =
      0; //"INVALID SEG TYPE IN PNR RETIEVAL";
  _msgErrCodeMap[ErrorResponseException::PNR_SEGS_NOT_FOUND] = 0; //"PNR SEGS NOT FOUND";
  _msgErrCodeMap[ErrorResponseException::CHECK_SEG_CONTINUITY] = 0; //"CHECK SEG CONTINUITY";
  _msgErrCodeMap[ErrorResponseException::INVALID_TERMINAL_TYPE] = 0; //"INVALID TERMINAL TYPE";
  _msgErrCodeMap[ErrorResponseException::MULTI_CONNECT] =
      0; //"MULTI CONNECT - SPECIFY BREAK POINT";
  _msgErrCodeMap[ErrorResponseException::DUPLICATE_X_AND_O_QUALIFIER] =
      0; //"DUPLICATE X AND O QUALIFIER-REENTER";
  _msgErrCodeMap[ErrorResponseException::CANNOT_USE_SEG_NUMBER] =
      0; //"UNABLE TO USE SEGMENT NUMBER-MODIFY";
  _msgErrCodeMap[ErrorResponseException::BOOK_SEPARATE_PNR] = 0; //"BOOK SEPARATE PNR FOR **";
  _msgErrCodeMap[ErrorResponseException::MAX_FOUR_SEATS] = 0; //"MAXIMUM FOUR SEATS FOR **";
  _msgErrCodeMap[ErrorResponseException::TRY_DIAGNOSTIC_ENTRY_10_20] =
      0; //"TRY DIAGNOSTIC ENTRY # 10 - 20";
  _msgErrCodeMap[ErrorResponseException::TRY_DIAGNOSTIC_ENTRY_1_6] =
      0; //"TRY DIAGNOSTIC ENTRY # 1 - 6";
  _msgErrCodeMap[ErrorResponseException::SPECIFY_PUBLIC_OR_PRIVATE] =
      0; //"SPECIFY PUBLIC OR PRIVATE";
  _msgErrCodeMap[ErrorResponseException::NO_DIAGNOSTIC_TO_DISPLAY] =
      0; //"NO DIAGNOSTIC TO DISPLAY";
  _msgErrCodeMap[ErrorResponseException::NO_CONVERTED_FARE_BASIS_CODE] =
      0; //"NO CONVERTED FARE BASIS CODE";
  _msgErrCodeMap[ErrorResponseException::NO_PQ_ITEMS_FOR_GIVEN_ORDINAL] =
      0; //"NO PQ ITEMS FOR GIVEN ORDINAL";
  _msgErrCodeMap[ErrorResponseException::FARENET_PROCESSING_NOT_YET_AVAIL] =
      0; //"FARENET PROCESSING NOT YET AVAILABLE";
  _msgErrCodeMap[ErrorResponseException::CALL_DIRECT] = 0; //"CALL DIRECT";
  _msgErrCodeMap[ErrorResponseException::NO_FLIGHTS_FOUND] = 0; //"NO FLIGHTS FOUND FOR 0";
  _msgErrCodeMap[ErrorResponseException::INVALID_CLASS] = 0; //"INVALID CLASS USED";
  _msgErrCodeMap[ErrorResponseException::NO_COMBOS] = 0; //"NO VALID COMBINATIONS FOUND";
  _msgErrCodeMap[ErrorResponseException::INVALID_FORMAT] = 0; //"INVALID FORMAT";
  _msgErrCodeMap[ErrorResponseException::NO_TRIP] = 0; //"TRIP NOT FOUND";
  _msgErrCodeMap[ErrorResponseException::CANNOT_SELL_SEG] = 0; //"UNABLE TO SELL SEGMENTS";
  _msgErrCodeMap[ErrorResponseException::MUST_BE_PRICED_FIRST] =
      0; //"INVALID-MUST BE PRICED FIRST";
  _msgErrCodeMap[ErrorResponseException::D1_NO_FLIGHT_ITEM_BLOCKS] = 0; //"D#1-NO FLIGHT ITEM BLKS";
  _msgErrCodeMap[ErrorResponseException::D1_FIND_ERR] = 0; //"D#1-FIND ERR A-06-JFI";
  _msgErrCodeMap[ErrorResponseException::D2_NO_FLIGHT_ITEM_SORT_LIST_BLOCKS] =
      0; //"D#2-NO FLIGHT ITEM SORT LIST BLKS";
  _msgErrCodeMap[ErrorResponseException::D3_NO_FLIGHT_COMB_BLOCKS] = 0; //"D#3-NO FLIGHT COMB BLKS";
  _msgErrCodeMap[ErrorResponseException::D3_INVALID_JRFCT_PARMS] = 0; //"D#3-INVLD JRFCT PARMS";
  _msgErrCodeMap[ErrorResponseException::QJRA_TRAN_VECTOR_ERR] = 0; //"QJRA-TRAN/VECTOR ERR";
  _msgErrCodeMap[ErrorResponseException::QJRB_TRAN_VECTOR_ERR] = 0; //"QJRB-TRAN/VECTOR ERR";
  _msgErrCodeMap[ErrorResponseException::NO_DIRECTS_NONSTOPS] = 0; //"NO DIRECTS/NONSTOPS FOR 0";
  _msgErrCodeMap[ErrorResponseException::TOO_MANY_COMBOS] =
      0; //"TOO MANY COMBINATIONS-USE SEG SEL";
  _msgErrCodeMap[ErrorResponseException::CORPORATE_PRICING_ACTIVE] =
      0; //"INVALID-CORPORATE PRICING IS ACTIVE";
  _msgErrCodeMap[ErrorResponseException::ALT_CITIES_INVALID_FOR_ARNK] =
      0; //"ALT CITIES NOT VALID FOR ARNK";
  _msgErrCodeMap[ErrorResponseException::ALT_DATES_INVALID_FOR_ARNK] =
      0; //"ALT DATES NOT VALID FOR ARNK";
  _msgErrCodeMap[ErrorResponseException::TRIP_DURATION_INVALID_FOR_ARNK] =
      0; //"TRIP DURATION NOT VALID FOR ARNK";
  _msgErrCodeMap[ErrorResponseException::MAX_CONNECTION_TIME_INVALID_FOR_ARNK] =
      0; //"MAXIMUM CONNECTION TIME NOT VALID FOR ARNK";
  _msgErrCodeMap[ErrorResponseException::MAX_TRAVEL_TIME_INVALID_FOR_ARNK] =
      0; //"MAXIMUM TRAVEL TIME NOT VALID FOR ARNK";
  _msgErrCodeMap[ErrorResponseException::ALT_CITIES_NOT_ALLOWED] =
      0; //"ALT CITIES CURRENTLY NOT ALLOWED";
  _msgErrCodeMap[ErrorResponseException::INVALID_SEG_NUMBER] =
      0; //"INVALID SEGMENT NUMBER SPECIFIED/SEGMENTS OUT OF SEQUENCE";
  _msgErrCodeMap[ErrorResponseException::INVALID_CITY_AIRPORT_CODE] =
      0; //"INVALID CITY/AIRPORT CODE";
  _msgErrCodeMap[ErrorResponseException::NEED_SEPARATOR] = 0; //"NEED SEPARATOR, *";
  _msgErrCodeMap[ErrorResponseException::SEG_DOES_NOT_MATCH_CITY_AIRPORT] =
      0; //"SEGMENT SPECIFIED DOES NOT MATCH CITY/AIRPORT SPECIFIED";
  _msgErrCodeMap[ErrorResponseException::NO_CITY_AIRPORT_FOR_ALTS] =
      0; //"CITY/AIRPORT FOR ALTS CANNOT BE FOUND";
  _msgErrCodeMap[ErrorResponseException::ALT_CITIES_AIRPORTS_INVALID_FOR_CONNECT] =
      0; //"ALT CITIES/AIRPORTS NOT VALID FOR CONNECTING CITIES";
  _msgErrCodeMap[ErrorResponseException::ALT_CITY_AIRPORT_AND_MILEAGE_RANGE] =
      0; //"ALT CITY/AIRPORT CODE AND MILEAGE RANGE CANNOT BE COMBINED";
  _msgErrCodeMap[ErrorResponseException::MILEAGE_RANGE_INVALID] =
      0; //"MILEAGE RANGE INVALID ENTER - 1-150 MILES";
  _msgErrCodeMap[ErrorResponseException::INVALID_ALT_CITY_AIRPORT_CODE] =
      0; //"INVALID ALT CITY/AIRPORT CODE";
  _msgErrCodeMap[ErrorResponseException::INTERNATIONAL_ALT_CITY_AS_PSEUDO] =
      0; //"INTERNATIONAL ALT CITY NOT PERMITTED FOR PSEUDO";
  _msgErrCodeMap[ErrorResponseException::MAX_ALT_AIRPORT_CITIES_EXCEEDED] =
      0; //"MAXIMUM NUMBER OF ALT AIRPORTS/CITIES PERMITTED IS X";
  _msgErrCodeMap[ErrorResponseException::DUPLICATE_BOARD_OFF_POINT_IN_SEG] =
      0; //"SPECIFY SAME SEGMENT BOARD/OFF POINT ONLY ONCE";
  _msgErrCodeMap[ErrorResponseException::SPECIFY_SEGS_FOR_NONCONSECUTIVE_ALT_DATES] =
      0; //"MUST SPECIFY SEGMENTS FOR NONCONSECUTIVE ALT DATES";
  _msgErrCodeMap[ErrorResponseException::INCLUDE_CONNECTING_SEGS] =
      0; //"INVALID ENTRY - MUST INCLUDE CONNECTING SEGMENTS";
  _msgErrCodeMap[ErrorResponseException::INVALID_NUMBER_FOR_ALT_DATES] =
      0; //"NUMBER ENTERED FOR ALT DATES IS INVALID - ENTER 1-3";
  _msgErrCodeMap[ErrorResponseException::COMBINED_CONSEC_NONCONSEC_ALT_DATES] =
      0; //"CONSECUTIVE/NONCONSECUTIVE ALT DATES CANNOT BE COMBINED";
  _msgErrCodeMap[ErrorResponseException::INVALID_DATE_ENTRY_FOR_ALT_TRAVEL_DATES] =
      0; //"INVALID DATE ENTRY FOR ALT TRAVEL DATES - ENTER DDMMM";
  _msgErrCodeMap[ErrorResponseException::MAX_NUMBER_ALT_TRAVEL_DATES_EXCEEDED] =
      0; //"MAXIMUM NUMBER PERMITTED FOR ALT TRAVEL DATES IS 6";
  _msgErrCodeMap[ErrorResponseException::DURATION_ONLY_VALID_FOR_ALT_DATES] =
      0; //"TRIP DURATION ONLY VALID WITH ALT TRAVEL DATES";
  _msgErrCodeMap[ErrorResponseException::DURATION_ONLY_VALID_FOR_CONSEC_ALT_DATES] =
      0; //"TRIP DURATION VALID WITH CONSECUTIVE ALT TVL DATES ONLY";
  _msgErrCodeMap[ErrorResponseException::DURATION_IS_INVALID] =
      0; //"TRIP DURATION IS INVALID ENTER 0-3";
  _msgErrCodeMap[ErrorResponseException::DURATION_ONLY_VALID_FOR_CONSEC_STOPOVERS] =
      0; //"TRIP DURATION VALID FOR CONSECUTIVE STOPOVERS ONLY";
  _msgErrCodeMap[ErrorResponseException::MAX_CONNECTION_TIME_EXCEEDED] =
      0; //"MAXIMUM CONNECTION TIME IS INVALID ENTER 1-24";
  _msgErrCodeMap[ErrorResponseException::MAX_TRAVEL_TIME_EXCEEDED] =
      0; //"MAXIMUM TRAVEL TIME IS INVALID ENTER 1-48";
  _msgErrCodeMap[ErrorResponseException::PRIOR_DATE_DEPART_NOT_CHECKED] =
      0; //"ARVL RQST - PRIOR DATE DEPARTURE NOT CHECKED";
  _msgErrCodeMap[ErrorResponseException::NO_PRIVATE_FARES_VALID_FOR_PASSENGER] =
      0; //"NO PRIVATE FARES VALID FOR PASSENGER TYPE/CLASS OF SERVICE";
  _msgErrCodeMap[ErrorResponseException::NO_PUBLIC_FARES_VALID_FOR_PASSENGER] =
      0; //"NO PUBLIC FARES VALID FOR PASSENGER TYPE/CLASS OF SERVICE";
  //
  // New errors start at 7000
  //
  _msgErrCodeMap[ErrorResponseException::NETWORK_EXCEPTION] = 0; //"NETWORK EXCEPTION";
  _msgErrCodeMap[ErrorResponseException::MEMORY_EXCEPTION] = 0; //"MEMORY EXCEPTION";
  _msgErrCodeMap[ErrorResponseException::DCA_CORBA_EXCEPTION] = 0; //"DCA CORBA EXCEPTION";
  _msgErrCodeMap[ErrorResponseException::DB_CURSOR_OPEN_ERROR] = 0; //"DB CURSOR OPEN ERROR";
  _msgErrCodeMap[ErrorResponseException::DB_CURSOR_FETCH_ERROR] = 0; //"DB CURSOR FETCH ERROR";
  _msgErrCodeMap[ErrorResponseException::DB_CURSOR_CLOSE_ERROR] = 0; //"DB CURSOR CLOSE ERROR";
  _msgErrCodeMap[ErrorResponseException::DB_INSERT_ERROR] = 0; //"DB INSERT ERROR";
  _msgErrCodeMap[ErrorResponseException::DB_UPDATE_ERROR] = 0; //"DB UPDATE ERROR";
  _msgErrCodeMap[ErrorResponseException::DB_SELECT_ERROR] = 0; //"DB SELECT ERROR";
  _msgErrCodeMap[ErrorResponseException::DB_BEGIN_WORK_ERROR] = 0; //"DB BEGIN WORK ERROR";
  _msgErrCodeMap[ErrorResponseException::DB_COMMIT_WORK_ERROR] = 0; //"DB COMMIT WORK ERROR";
  //
  // Salesguide errors start at 8000
  //
  _msgErrCodeMap[ErrorResponseException::SSG_FILE_OPEN_ERROR] = 0; //"SSG FILE OPEN ERROR";
  _msgErrCodeMap[ErrorResponseException::SSG_DATABASE_ERROR] = 0; //"SSG DATABASE ERROR";
  _msgErrCodeMap[ErrorResponseException::SSG_RECORD_ID_ERROR] = 0; //"SSG RECORD ID ERROR";
  _msgErrCodeMap[ErrorResponseException::SSG_PROCESSING_STARTED] = 0; //"SSG PROCESSING STARTED";
  _msgErrCodeMap[ErrorResponseException::SSG_PROCESSING_COMPLETED] =
      0; //"SSG PROCESSING COMPLETED";
  _msgErrCodeMap[ErrorResponseException::CNP_PROCESSING_COMPLETED_EOF] =
      0; //"CNP PROCESSING COMPLETED EOF";
  _msgErrCodeMap[ErrorResponseException::EQP_PROCESSING_FILE_NUMBER] =
      0; //"EQP PROCESSING FILE NUMBER";
  _msgErrCodeMap[ErrorResponseException::CNP_DELETE_OLD_FAILED] = 0; //"CNP DELETE OLD FAILED";
  _msgErrCodeMap[ErrorResponseException::SSG_DELETE_FAILED] = 0; //"SSG DELETE FAILED";
  _msgErrCodeMap[ErrorResponseException::SSG_ITEM_DELETED] = 0; //"SSG ITEM DELETED";
  _msgErrCodeMap[ErrorResponseException::SSG_UPDATE_FAILED] = 0; //"SSG UPDATE FAILED";
  _msgErrCodeMap[ErrorResponseException::SSG_ITEM_UPDATED] = 0; //"SSG ITEM UPDATED";
  _msgErrCodeMap[ErrorResponseException::SSG_ADD_FAILED] = 0; //"SSG ADD FAILED";
  _msgErrCodeMap[ErrorResponseException::SSG_ITEM_ADDED] = 0; //"SSG ITEM ADDED";
  _msgErrCodeMap[ErrorResponseException::SSG_DUPLICATE_FOUND] = 0; //"SSG DUPLICATE FOUND";
  _msgErrCodeMap[ErrorResponseException::SSG_SYSTEM_SUSPENDED] = 0; //"SSG SYSTEM SUSPENDED";
  _msgErrCodeMap[ErrorResponseException::SSG_PROCESSING_RESTARTED] =
      0; //"SSG PROCESSING RESTARTED";
  _msgErrCodeMap[ErrorResponseException::SSG_INPUT_FILE_MISSING] = 0; //"SSG INPUT FILE MISSING";
  _msgErrCodeMap[ErrorResponseException::CNP_PROCESSING_COMPLETED_NEF] =
      0; //"CNP PROCESSING COMPLETED NEF";
  _msgErrCodeMap[ErrorResponseException::SSG_CHECKPOINT_FAILED] = 0; //"SSG CHECKPOINT FAILED";
  _msgErrCodeMap[ErrorResponseException::SSG_MCT_MCTREGIONS_SQL_ERROR] =
      0; //"SSG MCT MCTREGIONS SQL ERROR";
  _msgErrCodeMap[ErrorResponseException::SSG_MCT_SYSTEMPARAMETERS_SQL_ERROR] =
      0; //"SSG MCT SYSTEMPARAMETERS SQL ERROR";
  _msgErrCodeMap[ErrorResponseException::SSG_MCT_MINIMUMCONNECTTIME_SQL_ERROR] =
      0; //"SSG MCT MINIMUMCONNECTTIME SQL ERROR";
  _msgErrCodeMap[ErrorResponseException::SSG_MCT_INVALID_ENTRY_CODE] =
      0; //"SSG MCT INVALID ENTRY CODE";
  _msgErrCodeMap[ErrorResponseException::SSG_MCT_INVALID_ACTION_CODE] =
      0; //"SSG MCT INVALID ACTION CODE";
  _msgErrCodeMap[ErrorResponseException::SSG_MCT_FILES_SWITCHED] = 0; //"SSG MCT FILES SWITCHED";
  _msgErrCodeMap[ErrorResponseException::SSG_MCT_FILES_NOT_SWITCHED] =
      0; //"SSG MCT FILES NOT SWITCHED";
  _msgErrCodeMap[ErrorResponseException::SSG_MCT_SWITCH_ERROR] = 0; //"SSG MCT SWITCH ERROR";
  _msgErrCodeMap[ErrorResponseException::SSG_MCT_CACHE_NOTIFY_ERROR] =
      0; //"SSG MCT CACHE NOTIFY ERROR";
  _msgErrCodeMap[ErrorResponseException::SSG_MCT_MVS_LOAD_STARTED] =
      0; //"SSG MCT MVS LOAD STARTED";
  _msgErrCodeMap[ErrorResponseException::SSG_MCT_MVS_LOAD_COMPLETE] =
      0; //"SSG MCT MVS LOAD COMPLETE";
  _msgErrCodeMap[ErrorResponseException::SSG_MCT_MVS_LOAD_ERROR] = 0; //"SSG MCT MVS LOAD ERROR";
  _msgErrCodeMap[ErrorResponseException::SSG_MCT_MVSFILE_LOADER_SUSPENDED] =
      0; //"SSG MCT MVSFILE LOADER SUSPENDED";
  _msgErrCodeMap[ErrorResponseException::SSG_SCHEDULE_BATCH_LOAD_STARTED] =
      0; //"SSG SCHEDULE BATCH LOAD STARTED";
  _msgErrCodeMap[ErrorResponseException::SSG_SCHEDULE_BATCH_LOAD_START_FAILED] =
      0; //"SSG SCHEDULE BATCH LOAD START FAILED";
  _msgErrCodeMap[ErrorResponseException::SSG_SCHEDULE_BATCH_LOAD_FAILED] =
      0; //"SSG SCHEDULE BATCH LOAD FAILED";
  _msgErrCodeMap[ErrorResponseException::SSG_SCHEDULE_BATCH_LOAD_COMPLETED] =
      0; //"SSG SCHEDULE BATCH LOAD COMPLETED";
  _msgErrCodeMap[ErrorResponseException::SSG_SCHEDULE_BATCH_FILE_FORMAT_ERROR] =
      0; //"SSG SCHEDULE BATCH FILE FORMAT ERROR";
  _msgErrCodeMap[ErrorResponseException::SSG_SCHEDULE_BATCH_PROCESSING_ERROR] =
      0; //"SSG SCHEDULE BATCH PROCESSING ERROR";
  _msgErrCodeMap[ErrorResponseException::SSG_SCHEDULE_INVALID_ARGS_ERROR] =
      0; //"SSG SCHEDULE INVALID ARGS ERROR";
  _msgErrCodeMap[ErrorResponseException::SSG_SCHEDULE_APPLICATION_INIT_ERROR] =
      0; //"SSG SCHEDULE APPLICATION INIT ERROR";
  _msgErrCodeMap[ErrorResponseException::SSG_SCHEDULE_APPLICATION_LOGIC_ERROR] =
      0; //"SSG SCHEDULE APPLICATION LOGIC ERROR";
  _msgErrCodeMap[ErrorResponseException::SSG_SCHEDULE_PROCESS_RECORD0_STARTED] =
      0; //"SSG SCHEDULE PROCESS RECORD0 STARTED";
  _msgErrCodeMap[ErrorResponseException::SSG_SCHEDULE_PROCESS_RECORD0_COMPLETED] =
      0; //"SSG SCHEDULE PROCESS RECORD0 COMPLETED";
  _msgErrCodeMap[ErrorResponseException::SSG_SCHEDULE_PROCESS_RECORD0_FAILED] =
      0; //"SSG SCHEDULE PROCESS RECORD0 FAILED";
  _msgErrCodeMap[ErrorResponseException::SSG_SCHEDULE_TAPE_NOT_LOADED_ERROR] =
      0; //"SSG SCHEDULE TAPE NOT LOADED ERROR";
  _msgErrCodeMap[ErrorResponseException::SSG_SCHEDULE_DYNAMIC_UPDATE_ERROR] =
      0; //"SSG SCHEDULE DYNAMIC UPDATE ERROR";
  _msgErrCodeMap[ErrorResponseException::SSG_QUERY_CHECKPOINT_FAILED] =
      0; //"SSG QUERY CHECKPOINT FAILED";
  _msgErrCodeMap[ErrorResponseException::SSG_UPDATE_CHECKPOINT_FAILED] =
      0; //"SSG UPDATE CHECKPOINT FAILED";

  // Bankers Selling Rate (BSR) errors start at 8501
  _msgErrCodeMap[ErrorResponseException::BSR_INVALID_DATA] = 0; //"BSR INVALID DATA";
  _msgErrCodeMap[ErrorResponseException::BSR_PROCESSING_START] = 0; //"BSR PROCESSING START";
  _msgErrCodeMap[ErrorResponseException::BSR_ACTION_CODE] = 0; //"BSR ACTION CODE";
  _msgErrCodeMap[ErrorResponseException::BSR_PROCESSING_END] = 0; //"BSR PROCESSING END";
  _msgErrCodeMap[ErrorResponseException::BSR_EMU_NATION_IGNORED] = 0; //"BSR EMU NATION IGNORED";
  _msgErrCodeMap[ErrorResponseException::BSR_DB_ERROR] = 0; //"BSR DB ERROR";

  // Cat31 Errors
  _msgErrCodeMap[ErrorResponseException::UNABLE_TO_MATCH_FARE] = 167;
  _msgErrCodeMap[ErrorResponseException::UNABLE_TO_MATCH_REISSUE_RULES] = 169;
  _msgErrCodeMap[ErrorResponseException::REISSUE_RULES_FAIL] = 164;
  _msgErrCodeMap[ErrorResponseException::NUMBER_OF_REISSUES_RESTRICTED] = 165;

  _msgErrCodeMap[ErrorResponseException::FARE_CURRENCY_OVERRIDE_NOT_VALID] =
      175; //"CURRENCY OVERRIDE NOT VALID";
  _msgErrCodeMap[ErrorResponseException::ATAE_RETURNED_NO_BOOKING_CODES] =
      0; //"ATAE SERVICE RETURNED NO BOOKING CODES";
  _msgErrCodeMap[ErrorResponseException::RESTRICTED_CURRENCY] = 0; //"RESTRICTED CURRENCY";
  _msgErrCodeMap[ErrorResponseException::AGENT_CURRENCY_CODE_MISSING] =
      0; //"AGENT CURRENCY CODE MISSING";
  _msgErrCodeMap[ErrorResponseException::AGENT_PCC_NON_EXISTENT] = 0; //"AGENT PCC NON EXISTENT";
  _msgErrCodeMap[ErrorResponseException::INVALID_GLOBAL_DIRECTION_REQUESTED] =
      0; //"INVALID GLOBAL DIRECTION REQUESTED ";
  _msgErrCodeMap[ErrorResponseException::FAILED_TO_CONNECT_RULE_TEXT_SERVER] =
      0; //"FAILED TO CONNECT RULE TEXT SERVER ";
  _msgErrCodeMap[ErrorResponseException::INVALID_CORP_ID] = 0; //"INVALID CORPORATE ID";
  _msgErrCodeMap[ErrorResponseException::INVALID_CURRENCY_CODE_REQUESTED] =
      174; //"INVALID CURRENCY CODE REQUESTED";
  _msgErrCodeMap[ErrorResponseException::SOLD_OUT] = 0; //"SOLD OUT";

  // SAX Parser failures
  _msgErrCodeMap[ErrorResponseException::SAX_PARSER_FAILURE] =
      0; //"PARSE FAILURE - INVALID REQUEST";

  // Limitiation errors start at 9501
  _msgErrCodeMap[ErrorResponseException::LMT_ISSUE_SEP_TKTS_EXCEED_NUM_DEPT_ARR] =
      0; //"ISSUE SEPARATE TKTS - MORE THAN N ARR/DEP IN";
  _msgErrCodeMap[ErrorResponseException::LMT_ISSUE_SEP_TKTS_INTL_SURFACE_RESTR] =
      145; //"ISSUE SEPARATE TICKETS-INTL SURFACE RESTRICTED";
  _msgErrCodeMap[ErrorResponseException::LMT_ISSUE_SEP_TKTS_EXCEED_NUM_DEPT_ARR_JAL] =
      141; //"ISSUE SEPARATE TKTS - MORE THAN 3 ARR/DEP IN";

  // All system related exceptions like std::bad_aloc etc
  // can be passed as unknow exception
  _msgErrCodeMap[ErrorResponseException::UNKNOWN_EXCEPTION] = 0; //"UNKNOWN EXCEPTION";
  _msgErrCodeMap[ErrorResponseException::SYSTEM_EXCEPTION] = 0; //"SYSTEM EXCEPTION";

  _msgErrCodeMap[ErrorResponseException::UNABLE_TO_MATCH_REFUND_RULES] = 170;
  _msgErrCodeMap[ErrorResponseException::REFUND_RULES_FAIL] = 171;

}
}
