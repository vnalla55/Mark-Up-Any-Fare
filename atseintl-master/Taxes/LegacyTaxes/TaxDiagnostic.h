// ---------------------------------------------------------------------------
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ---------------------------------------------------------------------------

#ifndef TAX_DIAGNOSTIC_H
#define TAX_DIAGNOSTIC_H

#include "Diagnostic/DCFactory.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class DiagManager;
class PricingTrx;
class TaxResponse;
class TaxCodeReg;

namespace YQYR
{
class ServiceFeeRec1Validator;
}

//
// Class TaxDiagnostic will handle all Tax Service calls to the appropriate Tax Service
// Diagnostic formatters.
//

class TaxDiagnostic final
{

public:
  static const std::string UTC_INQUIRY;

  //
  // TaxFailCodes will be used thoughout the Tax Service to identify Tax specific failures
  //

  enum FailCodes
  {
    NONE, // 0
    SALES_DATE, // 1
    TRAVEL_DATE, // 2
    POINT_OF_SALE, // 3
    POINT_OF_ISSUE, // 4
    PAX_TYPE, // 5
    FREE_TKT_EXEMPT, // 6
    NO_SPECIAL_TAX, // 7
    SELL_CURRENCY, // 8
    VALID_CXR, // 9
    ITINERARY, // 10
    TRAVEL_TYPE, // 11
    DISCOUNT_PERCENTAGE, // 12
    CXR_EXCL, // 13
    EQUIP_EXCL, // 14
    TRANSIT_RESTRICTION, // 15
    ORIGIN_LOCATION, // 16
    LOCATION_RESTRICTION, // 17
    TRIP_TYPES_FROM_TO, // 18
    TRIP_TYPES_BETWEEN, // 19
    TRIP_TYPES_WITHIN, // 20
    TRIP_TYPES_WHOLLY, // 21
    TRANSIT, // 22
    TRANSIT_TIME, // 23
    TRANSIT_RESTRICTION_IND, // 24
    FORM_OF_PAYMENT, // 25
    TRANSIT_INTL_DOM, // 26
    TRANSIT_SURF_INTL, // 27
    TRANSIT_INTL_SURF, // 28
    TRANSIT_OFF_LINE_CXR, // 29
    TRANSIT_ARRIVE_TIME, // 30
    TRANSIT_DEPART_TIME, // 31
    FARE_TYPE, // 32
    FARE_CLASS, // 33
    APPLY_TOO_MANY, // 34
    SALE_COUNTRY, // 35
    NO_TAX_VECTOR, // 36
    OLD_TAX_ITEM, // 37
    OVERRIDE, // 38
    EXEMPT_ALL_TAXES, // 39
    NO_TAX_NATION_FOUND, // 40
    NO_TAX_CODE, // 41
    NO_TAX_ADDED, // 42
    CURRENCY_CONVERTER_BSR, // 43
    CURRENCY_CONVERTER_NUC, // 44
    APPLY_ONCE_PER_ITIN, // 45
    ROUNDING_ERROR, // 46
    ENPLANEMENT_LOCATION, // 47
    DEPLANEMENT_LOCATION, // 48
    DESTINATION_LOCATION, // 49
    TERMINATION_LOCATION, // 50
    BAD_DATA_HANDLER_POINTER, // 51
    RANGE_FARE, // 52
    RANGE_MILES, // 53
    NUM_COUPON_BOOKLET, // 54
    TAX_ONCE_PER_BOARD_OFF, // 55
    EQUIPMENT_TYPE, // 56
    CARRIER_EXEMPTION, // 57
    NO_US_STOPOVER, // 58
    PARTIAL_TAX, // 59
    NO_ZP_PAX_CMP, // 60
    TAX_ONCE_PER_SEGMENT, // 61
    CLASS_OF_SERVICE, // 62
    TICKET_DESIGNATOR, // 63
    JOURNEY,
    PORTION,
    SECTOR,
    LOC1_TRANSFER_TYPE,
    JOURNEY_TYPE,
    PARTITION_ID,
    ORIGINAL_TICKET_ONLY,
    OTHER
  };

  TaxDiagnostic() = default;

  //
  // doTaxDiagnostics can build multiple information displays for Tax Service.
  // Display: WPQ/*804 - Customer Display of Valid Taxes(old WPQ/*24)
  // Display: WPQ/*801 - All Valid Tax Display
  // Display: WPQ/*802 - All Valid Tax Record Display
  // Display: WPQ/*803 - All Valid Passenger Facility Charge Display
  // Display: WPQ/*804 - Customer Display of Valid Taxes(old WPQ/*24)
  //

  bool checkDiagnosticTypeToCollect(PricingTrx& trx) const;
  void writeTaxResponseToDiagnostic(PricingTrx& trx, TaxResponse* taxResponse, DiagCollector& diag, const std::string& modifiedTaxes);

  void collectDiagnostics(PricingTrx& trx);

  //
  // collectTaxErrorInfo will assist in the debugging of the Tax Service
  // The Diagnostic Provided will have an indepth display of the specific Tax and a fail
  // code to find the precise line of code that resulted in an invalid Tax.
  //

  static void collectErrors(PricingTrx& trx,
                            TaxResponse& taxResponse,
                            FailCodes failCode,
                            DiagnosticTypes taxDiagNumber,
                            const std::string& extendedInfo = "");

  //
  // collectTaxErrorInfo will assist in the debugging of the Tax Service
  // The Diagnostic Provided will have an indepth display of the specific Tax and a fail
  // code to find the precise line of code that resulted in an invalid Tax.
  //

  static void collectErrors(PricingTrx& trx,
                            const TaxCodeReg& taxCodeReg,
                            TaxResponse& taxResponse,
                            FailCodes failCode,
                            DiagnosticTypes taxDiagNumber,
                            const std::string& extendedInfo = "");

  static void collectErrors(PricingTrx& trx,
                            YQYR::ServiceFeeRec1Validator& serviceFeeRec1Validator,
                            TaxResponse& taxResponse,
                            FailCodes failCode,
                            DiagnosticTypes taxDiagNumber,
                            const std::string& extendedInfo = "",
                            const std::string& extendedInfo1 = "");

  std::map<FailCodes, std::string>& msgMap() { return _msgMap; }
  const std::map<FailCodes, std::string>& msgMap() const { return _msgMap; }

  void
  displayTaxRetrievalDate(PricingTrx& trx, const TaxResponse& taxResponse, DiagCollector& diag);

  static bool isTaxDiagnostic(PricingTrx& trx);
  static bool isValidNation(PricingTrx& trx, const NationCode& nation);
  static bool isValidTaxCode(PricingTrx& trx, const TaxCode& code);
  static bool isValidSeqNo(PricingTrx& trx, int seq);
  static bool isSpnOff(PricingTrx& trx, int spn);

  static bool printSeqDef(PricingTrx& trx, TaxCodeReg& taxCodeReg);
  static void printUtcInfo(PricingTrx& trx, DiagManager& diag, TaxCodeReg& taxCodeReg);
  static void printDatesInfo(DiagManager& diag, TaxCodeReg& taxCodeReg);
  static void printTaxOnTaxInfo(DiagManager& diag, TaxCodeReg& taxCodeReg, PricingTrx& trx);
  static void printExemptionCxr(DiagManager& diag, TaxCodeReg& taxCodeReg);
  static void printRestrictionFareClass(DiagManager& diag, TaxCodeReg& taxCodeReg);
  static void printTransit(DiagManager& diag, TaxCodeReg& taxCodeReg);
  static void printTicketDesignator(DiagManager& diag, TaxCodeReg& taxCodeReg);

  static std::string toString(const RoundingRule& rule);
  static std::string occurenceToString(const Indicator& ind);
  static std::string tripTypeToString(const Indicator& ind);
  static std::string itinTypeToString(const Indicator& ind);
  static std::string travelTypeToString(const Indicator& ind);
  static std::string locTypeToString(const LocType& locType);

  static void printHelpInfo(PricingTrx& trx);

private:
  void fillMessageMap();
  void displayValidatingCxrDiags(PricingTrx& trx, const Itin& itin,
                                const std::string& modifiedTaxes, DiagCollector& diag);

  std::map<FailCodes, std::string> _msgMap;

  TaxDiagnostic(const TaxDiagnostic& rhs) = delete;
  TaxDiagnostic& operator=(const TaxDiagnostic& rhs) = delete;

  static log4cxx::LoggerPtr _logger;
};
}

#endif // TAX_DIAGNOSTIC_H
