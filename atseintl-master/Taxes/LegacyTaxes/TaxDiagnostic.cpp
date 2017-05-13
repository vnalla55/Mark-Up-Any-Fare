// ----------------------------------------------------------------------------
//  File:         TaxDiagnostic.cpp
//  Author:       Dean Van Decker
//  Created:      02/20/2004
//  Description:
//
//
//
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
// ----------------------------------------------------------------------------


#include "Common/AirlineShoppingUtils.h"
#include "Common/FallbackUtil.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/ServiceFeeUtil.h"
#include "DataModel/BaseExchangeTrx.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxSpecConfigReg.h"
#include "Diagnostic/Diag877Collector.h"
#include "Diagnostic/DiagManager.h"
#include "Diagnostic/DiagVisitor.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/ServiceFeesGroup.h"
#include "Taxes/Common/ReissueExchangeDateSetter.h"
#include "Taxes/LegacyFacades/ItinSelector.h"
#include "Taxes/LegacyTaxes/ServiceFeeRec1ValidatorYQ.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/TaxItinerary.h"
#include "Taxes/LegacyTaxes/TripTypesValidator.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"

#include <log4cxx/logger.h>
#include <map>
#include <string>

using namespace tse;
using namespace std;

namespace tse
{
FALLBACK_DECL(fixDiag817)
}


log4cxx::LoggerPtr
TaxDiagnostic::_logger(log4cxx::Logger::getLogger("atseintl.Taxes.TaxDiagnostic"));

const std::string TaxDiagnostic::UTC_INQUIRY = "FOR UTC INQUIRY USE PARAMETER : DDUTC";

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void TaxDiagnostic::collectDiagnostics
//
// Description:  This method will display diagnostic information to allow for a
//         quick debug of all tax processes. Diagnostic number must be set in
//         the Transaction Orchestrator to apply the following methods:
//
// @param  PricingTrx - Transaction object
//
//
// </PRE>
// ----------------------------------------------------------------------------

bool
TaxDiagnostic::checkDiagnosticTypeToCollect(PricingTrx& trx) const
{
  return ((trx.diagnostic().diagnosticType() == AllPassTaxDiagnostic281) || // 801
      (trx.diagnostic().diagnosticType() == TaxRecSummaryDiagnostic) || // 802
      (trx.diagnostic().diagnosticType() == PFCRecSummaryDiagnostic) || // 803
      (trx.diagnostic().diagnosticType() == LegacyTaxDiagnostic24) || // 804
      (trx.diagnostic().diagnosticType() == Diagnostic24) ||
      (trx.diagnostic().diagnosticType() == Diagnostic818) ||
      (trx.diagnostic().diagnosticType() == Diagnostic827) ||
      (trx.diagnostic().diagnosticType() == Diagnostic877) ||
      (trx.diagnostic().diagnosticType() == Diagnostic817));
}

void TaxDiagnostic::writeTaxResponseToDiagnostic(PricingTrx& trx,
    TaxResponse* taxResponse, DiagCollector& diag,
    const std::string& modifiedTaxes)
{
  // will display tax retrieval date if it's appropriate
  //
  // Do Diagnostic 801 *** TaxItem Tax Display ****
  //
  if (trx.diagnostic().diagnosticType() == AllPassTaxDiagnostic281)
  {
    diag.enable(AllPassTaxDiagnostic281); // 801
    displayTaxRetrievalDate(trx, *taxResponse, diag);
    diag << *taxResponse;
  }
  //
  // Do Diagnostic 802 **** Tax Record Display ****
  //
  if (trx.diagnostic().diagnosticType() == TaxRecSummaryDiagnostic)
  {
    diag.enable(TaxRecSummaryDiagnostic); // 802
    diag << *taxResponse;
  }
  //
  // Do Diagnostic 803 **** PFC Display ****
  //
  if (trx.diagnostic().diagnosticType() == PFCRecSummaryDiagnostic)
  {
    diag.enable(PFCRecSummaryDiagnostic); // 803
    diag << *taxResponse;
  }
  //
  // Do Diagnostic 804  **** Customer Tax Display WPQ/*24 ****
  //
  if ((trx.diagnostic().diagnosticType() == LegacyTaxDiagnostic24) ||
      (trx.diagnostic().diagnosticType() == Diagnostic24))
  {
    if ("Y" != modifiedTaxes)
    {
      diag.enable(LegacyTaxDiagnostic24, Diagnostic24); // 804
      diag << *taxResponse;
    }
  }
  //
  // Do Diagnostic 817
  //
  if (trx.diagnostic().diagnosticType() == Diagnostic817)
  {
    diag.enable(Diagnostic817); // 817
    diag << *taxResponse;
    diag.displayInfo();
  }
  //
  // Do Diagnostic 818
  //
  if (trx.diagnostic().diagnosticType() == Diagnostic818)
  {
    diag.enable(Diagnostic818); // 818
    diag << *taxResponse;
  }

  if (trx.diagnostic().diagnosticType() == Diagnostic827)
  {
    diag.enable(Diagnostic827); // 827
    diag << *taxResponse;
  }

  if (trx.diagnostic().diagnosticType() == Diagnostic877 &&
      trx.diagnostic().diagParamMapItem("TA") == "X")
  {
    diag.enable(Diagnostic877); // 877
    diag << *taxResponse;
  }
}

void
TaxDiagnostic::collectDiagnostics(PricingTrx& trx)
{
  if (!checkDiagnosticTypeToCollect(trx))
    return;

  DCFactory* factory = DCFactory::instance();
  DiagCollector& diag = *(factory->create(trx));
  diag.trx() = &trx;

  const std::string& modifiedTaxes = diag.rootDiag()->diagParamMapItem("MODIFIED_TAXES");

  ItinSelector itinSelector(trx);

  if (fallback::fixDiag817(&trx))
  {
    for(Itin* itin : itinSelector.get())
    {
      if (itin->getTaxResponses().empty())
      {
        if ("Y" != modifiedTaxes)
        {
          diag.enable(LegacyTaxDiagnostic24, Diagnostic24); // 804
          diag << "\n\n T A X E S   N O T   A P P L I C A B L E \n\n";
        }
      }
      else if( trx.isValidatingCxrGsaApplicable() && !itin->getAllValCxrTaxResponses().empty())
      {
        displayValidatingCxrDiags(trx, *itin, modifiedTaxes, diag);
      }
      else
      {
        for (TaxResponse* taxResponse : itin->getTaxResponses())
          writeTaxResponseToDiagnostic(trx, taxResponse, diag, modifiedTaxes);
      }
      diag.flushMsg();
    }
  }
  else
  {
    for(Itin* itin : itinSelector.getItin())
    {
      if (itin->getTaxResponses().empty())
      {
        if ("Y" != modifiedTaxes)
        {
          diag.enable(LegacyTaxDiagnostic24, Diagnostic24); // 804
          diag << "\n\n T A X E S   N O T   A P P L I C A B L E \n\n";
        }
      }
      else if( trx.isValidatingCxrGsaApplicable() && !itin->getAllValCxrTaxResponses().empty())
      {
        displayValidatingCxrDiags(trx, *itin, modifiedTaxes, diag);
      }
      else
      {
        for (TaxResponse* taxResponse : itin->getTaxResponses())
          writeTaxResponseToDiagnostic(trx, taxResponse, diag, modifiedTaxes);
      }
      diag.flushMsg();
    }

    PrintTaxesOnChangeFee printTaxesOnChangeFee(itinSelector);
    diag.accept(printTaxesOnChangeFee);
  }
}

void
TaxDiagnostic::displayValidatingCxrDiags(PricingTrx& trx, const Itin& itin,
                                         const std::string& modifiedTaxes, DiagCollector& diag)
{
  const std::string& diagCarrier = trx.diagnostic().diagParamMapItem(Diagnostic::DIAG_CARRIER);
  if (trx.diagnostic().diagnosticType() == AllPassTaxDiagnostic281)
  {
     diag.enable(AllPassTaxDiagnostic281); // 801
  }
  if (trx.diagnostic().diagnosticType() == TaxRecSummaryDiagnostic)
  {
    diag.enable(TaxRecSummaryDiagnostic); // 802
  }
  if (trx.diagnostic().diagnosticType() == PFCRecSummaryDiagnostic)
  {
    diag.enable(PFCRecSummaryDiagnostic); // 803
  }
  if (("Y" != modifiedTaxes) && ((trx.diagnostic().diagnosticType() == LegacyTaxDiagnostic24) ||
                                 (trx.diagnostic().diagnosticType() == Diagnostic24)))
  {
    diag.enable(LegacyTaxDiagnostic24, Diagnostic24); // 804
  }
  if (trx.diagnostic().diagnosticType() == Diagnostic817)
  {
    diag.enable(Diagnostic817); // 817
  }
  if (trx.diagnostic().diagnosticType() == Diagnostic827)
  {
    diag.enable(Diagnostic827); // 827
  }

  for (TaxResponse* taxResponse : itin.getAllValCxrTaxResponses())
  {
    if(taxResponse && !taxResponse->isDisplayed())
    {
      if(diagCarrier.empty() || diagCarrier == taxResponse->validatingCarrier())
      {
        diag << *taxResponse;
        taxResponse->displayed() = true;
      }
    }
  }
  if (trx.diagnostic().diagnosticType() == Diagnostic817 &&
      !diag.rootDiag()->diagParamMapItemPresent("DC")       )
  {
    diag.displayInfo();
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void TaxDiagnostic::collectTaxErrors
//
// Description:  This function will save information to allow for a
//         quick debug of any failed tax items.
//
// @param  PricingTrx - Transaction object
//
// @return TaxCodeDataStruct - The specific Tax item information.
//
// </PRE>
// ----------------------------------------------------------------------------

void
TaxDiagnostic::collectErrors(PricingTrx& trx,
                             YQYR::ServiceFeeRec1Validator& serviceFeeRec1Validator,
                             TaxResponse& taxResponse,
                             FailCodes failCode,
                             DiagnosticTypes taxDiagNumber,
                             const std::string& extendedInfo,
                             const std::string& extendedInfo1)
{
  if (LIKELY(!isTaxDiagnostic(trx)))
    return;

  if (trx.diagnostic().diagnosticType() == AllPassTaxDiagnostic281 ||
      trx.diagnostic().diagnosticType() == TaxRecSummaryDiagnostic ||
      trx.diagnostic().diagnosticType() == PFCRecSummaryDiagnostic ||
      trx.diagnostic().diagnosticType() == LegacyTaxDiagnostic24 ||
      trx.diagnostic().diagnosticType() == Diagnostic817 ||
      trx.diagnostic().diagnosticType() == Diagnostic818)
    return;

  std::map<std::string, std::string>::const_iterator i = trx.diagnostic().diagParamMap().find("TX");

  if (i != trx.diagnostic().diagParamMap().end())
  {
    LOG4CXX_INFO(_logger, "Diagnostic Info: " << i->second);

    if (i->second != serviceFeeRec1Validator.taxCode())
      return;
  }

  TaxDiagnostic taxDiagnostic;
  taxDiagnostic.fillMessageMap();

  std::string failReason = "INVALID REASON NUMBER";

  std::map<FailCodes, std::string>::iterator msgMapIter = taxDiagnostic.msgMap().find(failCode);

  if (msgMapIter != taxDiagnostic.msgMap().end())
    failReason = msgMapIter->second;

  if (trx.diagnostic().diagnosticType() == FailTaxCodeDiagnostic)
  {
    LOG4CXX_DEBUG(_logger,
                  "Tax Code: " << serviceFeeRec1Validator.taxCode() << "\n"
                               << "                                       "
                               << "Sequence Number: " << serviceFeeRec1Validator.seqNo() << "\n"
                               << "                                       "
                               << "Tax Amount: " << serviceFeeRec1Validator.taxAmt() << "\n"
                               << "                                       "
                               << "Passenger Type: " << taxResponse.paxTypeCode() << "\n"
                               << "                                       "
                               << "Not Applicaple For " << failReason
                               << " Reason Number: " << failCode);
  }

  taxResponse.diagCollector()->enable(FailTaxCodeDiagnostic, taxDiagNumber);
  const std::string& diagCarrier = trx.diagnostic().diagParamMapItem(Diagnostic::DIAG_CARRIER);
  if( trx.isValidatingCxrGsaApplicable())
  {
     if(!diagCarrier.empty() && diagCarrier != taxResponse.validatingCarrier())
       return;

     if(!taxResponse.validatingCarrier().empty() &&
       taxResponse.validatingCarrier() != trx.validatingCxr())
     {
        trx.setValCxrCode(taxResponse.validatingCarrier());
        (*taxResponse.diagCollector()) << "***********************************************************\n"
                                       << "VALIDATING CARRIER: " << taxResponse.validatingCarrier() << "\n";
     }
  }
  (*taxResponse.diagCollector()) << "***********************************************************\n"
                                 << "TAX CODE " << serviceFeeRec1Validator.taxCode()
                                 << "                       PASSENGER TYPE: "
                                 << taxResponse.paxTypeCode() << "\n"
                                 << "FAILED TAX FOR " << failReason << "\n"
                                 << "REASON NUMBER: " << failCode;

  if (!extendedInfo.empty())
  {
    std::map<std::string, std::string>::const_iterator i =
        trx.diagnostic().diagParamMap().find("DD");

    (*taxResponse.diagCollector()) << "\nEXT INFO: " << extendedInfo;
  }

  (*taxResponse.diagCollector()) << "\n";

  if (trx.getRequest()->owPricingRTTaxProcess())
  {
    std::string fltInfo = "";
    fltInfo = AirlineShoppingUtils::collectSegmentInfo(trx, taxResponse.farePath());
    fltInfo += "\n";
    (*taxResponse.diagCollector()) << fltInfo;
  }

  // TODO check it
  //(*taxResponse.diagCollector()) << serviceFeeRec1Validator ;

  if (trx.diagnostic().diagnosticType() == FailTaxCodeDiagnostic)
  {
    (*taxResponse.diagCollector())
        << "FOR TAX DATABASE INTERROGATION USE DIAGNOSTIC: " << taxDiagNumber << "\n";
  }

  taxResponse.diagCollector()->disable(FailTaxCodeDiagnostic, taxDiagNumber);
  taxResponse.diagCollector()->flushMsg();
}

void
TaxDiagnostic::collectErrors(PricingTrx& trx,
                             const TaxCodeReg& taxCodeReg,
                             TaxResponse& taxResponse,
                             FailCodes failCode,
                             DiagnosticTypes taxDiagNumber,
                             const std::string& extendedInfo)
{
  if (LIKELY(!isTaxDiagnostic(trx)))
    return;

  if (trx.diagnostic().diagnosticType() == AllPassTaxDiagnostic281 ||
      trx.diagnostic().diagnosticType() == TaxRecSummaryDiagnostic ||
      trx.diagnostic().diagnosticType() == PFCRecSummaryDiagnostic ||
      trx.diagnostic().diagnosticType() == LegacyTaxDiagnostic24 ||
      trx.diagnostic().diagnosticType() == Diagnostic817 ||
      trx.diagnostic().diagnosticType() == Diagnostic818 ||
      trx.diagnostic().diagnosticType() == Diagnostic824)
    return;

  std::map<std::string, std::string>::const_iterator i = trx.diagnostic().diagParamMap().find("TX");

  if (i != trx.diagnostic().diagParamMap().end())
  {
    LOG4CXX_INFO(_logger, "Diagnostic Info: " << i->second);

    if (i->second != taxCodeReg.taxCode())
      return;
  }

  TaxDiagnostic taxDiagnostic;
  taxDiagnostic.fillMessageMap();

  std::string failReason = "INVALID REASON NUMBER";

  std::map<FailCodes, std::string>::iterator msgMapIter = taxDiagnostic.msgMap().find(failCode);

  if (msgMapIter != taxDiagnostic.msgMap().end())
    failReason = msgMapIter->second;

  if (trx.diagnostic().diagnosticType() == FailTaxCodeDiagnostic)
  {
    LOG4CXX_DEBUG(_logger,
                  "Tax Code: " << taxCodeReg.taxCode() << "\n"
                               << "                                       "
                               << "Sequence Number: " << taxCodeReg.seqNo() << "\n"
                               << "                                       "
                               << "Tax Amount: " << taxCodeReg.taxAmt() << "\n"
                               << "                                       "
                               << "Passenger Type: " << taxResponse.paxTypeCode() << "\n"
                               << "                                       "
                               << "Not Applicaple For " << failReason
                               << " Reason Number: " << failCode);
  }

  taxResponse.diagCollector()->enable(FailTaxCodeDiagnostic, taxDiagNumber);

  const std::string& diagCarrier = trx.diagnostic().diagParamMapItem(Diagnostic::DIAG_CARRIER);
  if( trx.isValidatingCxrGsaApplicable())
  {
     if(!diagCarrier.empty() && diagCarrier != taxResponse.validatingCarrier())
       return;

     if(!taxResponse.validatingCarrier().empty() &&
       taxResponse.validatingCarrier() != trx.validatingCxr())
     {
        trx.setValCxrCode(taxResponse.validatingCarrier());
        (*taxResponse.diagCollector()) << "***********************************************************\n"
                                       << "VALIDATING CARRIER: " << taxResponse.validatingCarrier() << "\n";
     }
  }
  (*taxResponse.diagCollector()) << "***********************************************************\n"
                                 << "TAX CODE " << taxCodeReg.taxCode()
                                 << "                       PASSENGER TYPE: "
                                 << taxResponse.paxTypeCode() << "\n"
                                 << "FAILED TAX FOR " << failReason << "\n"
                                 << "REASON NUMBER: " << failCode;

  (*taxResponse.diagCollector()) << "\n";

  if (trx.getRequest()->owPricingRTTaxProcess())
  {
    std::string fltInfo = "";
    fltInfo = AirlineShoppingUtils::collectSegmentInfo(trx, taxResponse.farePath());
    fltInfo += "\n";
    (*taxResponse.diagCollector()) << fltInfo;
  }

  (*taxResponse.diagCollector()) << taxCodeReg;

  if (taxResponse.diagCollector()->isActive() && !extendedInfo.empty())
    (*taxResponse.diagCollector()) << "\nEXT INFO: " << extendedInfo;

  if (trx.diagnostic().diagnosticType() == FailTaxCodeDiagnostic)
  {
    (*taxResponse.diagCollector())
        << "FOR TAX DATABASE INTERROGATION USE DIAGNOSTIC: " << Diagnostic824 << "\n";
  }

  taxResponse.diagCollector()->disable(FailTaxCodeDiagnostic, taxDiagNumber);
  taxResponse.diagCollector()->flushMsg();
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void TaxDiagnostic::collectTaxErrors
//
// Description:  This function will save information to allow for a
//         quick debug of any failed tax items.
//
// @param  PricingTrx - Transaction object
//
// @return TaxCodeDataStruct - The specific Tax item information.
//
// </PRE>
// ----------------------------------------------------------------------------

void
TaxDiagnostic::collectErrors(PricingTrx& trx,
                             TaxResponse& taxResponse,
                             FailCodes failCode,
                             DiagnosticTypes taxDiagNumber,
                             const std::string& extendedInfo)
{
  if (LIKELY(!isTaxDiagnostic(trx)))
    return;

  taxResponse.diagCollector()->enable(FailTaxCodeDiagnostic, taxDiagNumber);

  const std::string& diagCarrier = trx.diagnostic().diagParamMapItem(Diagnostic::DIAG_CARRIER);
  if(! (trx.isValidatingCxrGsaApplicable() && !diagCarrier.empty() &&
                     diagCarrier != taxResponse.validatingCarrier())  )
  {
     (*taxResponse.diagCollector()) << "\n TAX DEBUG INFORMATION REASON NUMBER: " << failCode <<  "\n";

     TaxDiagnostic taxDiagnostic;
     taxDiagnostic.fillMessageMap();

     std::string failReason = "INVALID REASON NUMBER";

     std::map<FailCodes, std::string>::iterator msgMapIter = taxDiagnostic.msgMap().find(failCode);
     if (msgMapIter != taxDiagnostic.msgMap().end())
       failReason = msgMapIter->second;

     (*taxResponse.diagCollector()) << " FAILED TAX FOR " << failReason << "\n";

     if (taxResponse.diagCollector()->isActive() && !extendedInfo.empty())
       (*taxResponse.diagCollector()) << " EXT INFO: " << extendedInfo << "\n";

  }
  taxResponse.diagCollector()->disable(FailTaxCodeDiagnostic, taxDiagNumber);

  taxResponse.diagCollector()->flushMsg();
} // TaxDiagnostic::TaxDiagnostic

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void TaxDiagnostic::fillMessageMap
//
// Description:  This function will save information to allow for a
//         quick debug of any failed tax items.
//
//
// </PRE>
// ----------------------------------------------------------------------------

void
TaxDiagnostic::fillMessageMap()
{
  if (_msgMap.empty())
  {
    _msgMap[NONE] = "NEVER SHOULD HAVE FAILED"; // 0
    _msgMap[SALES_DATE] = "SALES DATE NOT VALID"; // 1
    _msgMap[TRAVEL_DATE] = "TRAVEL DATE NOT VALID"; // 2
    _msgMap[POINT_OF_SALE] = "POINT OF SALE NOT VALID"; // 3
    _msgMap[POINT_OF_ISSUE] = "POINT OF ISSUE NOT VALID"; // 4
    _msgMap[PAX_TYPE] = "PASSENGER TYPE RESTRICTION"; // 5
    _msgMap[FREE_TKT_EXEMPT] = "FREE TICKET EXEMPTION"; // 6
    _msgMap[NO_SPECIAL_TAX] = "NO SPECIAL TAX LOCATED IN TAX MAP"; // 7
    _msgMap[SELL_CURRENCY] = "SELL CURRENCY RESTRICTION"; // 8
    _msgMap[VALID_CXR] = "VALIDATING CARRIER RESTRICTION"; // 9
    _msgMap[ITINERARY] = "ONE WAY OR ROUND TRIP EXEMPTION"; // 10
    _msgMap[TRAVEL_TYPE] = "INTERNATIONAL OR DOMESTIC JOURNEY TYPE"; // 11
    _msgMap[DISCOUNT_PERCENTAGE] = "DISCOUNT PERCENTAGE RESTRICTION"; // 12
    _msgMap[CXR_EXCL] = "CARRIER EXCLUSION"; // 13
    _msgMap[EQUIP_EXCL] = "EQUIPMENT EXCLUSION"; // 14
    _msgMap[TRANSIT_RESTRICTION] = "TRANSIT RESTRICTION"; // 15
    _msgMap[ORIGIN_LOCATION] = "ORIGIN LOCATION RESTRICTION"; // 16
    _msgMap[LOCATION_RESTRICTION] = "LOCATION RESTRICTION"; // 17
    _msgMap[TRIP_TYPES_FROM_TO] = "FROM TO JOURNEY RESTRICTION"; // 18
    _msgMap[TRIP_TYPES_BETWEEN] = "BETWEEN JOURNEY RESTRICTION"; // 19
    _msgMap[TRIP_TYPES_WITHIN] = "WITHIN JOURNEY RESTRICTION"; // 20
    _msgMap[TRIP_TYPES_WHOLLY] = "WHOLLY WITHIN JOURNEY RESTRICTION"; // 21
    _msgMap[TRANSIT] = "TRANSIT RESTRICTION"; // 22
    _msgMap[TRANSIT_TIME] = "TRANSIT TIME"; // 23
    _msgMap[TRANSIT_RESTRICTION_IND] = "TRANSIT INDICATOR RESTRICTION"; // 24
    _msgMap[FORM_OF_PAYMENT] = "FORM OF PAYMENT RESTRICTION"; // 25
    _msgMap[TRANSIT_INTL_DOM] = "INTERNATIONAL TO DOMESTIC TRANSIT RESTRICTION"; // 26
    _msgMap[TRANSIT_SURF_INTL] = "DOMESTIC TO SURFACE TRANSIT RESTRICTION"; // 27
    _msgMap[TRANSIT_INTL_SURF] = "INTERNATIONAL TO SURFACE TRANSIT RESTRICTION"; // 28
    _msgMap[TRANSIT_OFF_LINE_CXR] = "OFFLINE TRANSIT RESTRICTION"; // 29
    _msgMap[TRANSIT_ARRIVE_TIME] = "ARRIVAL TIME TRANSIT RESTRICTION"; // 30
    _msgMap[TRANSIT_DEPART_TIME] = "DEPARTURE TIME TRANSIT RESTRICTION"; // 31
    _msgMap[FARE_TYPE] = "FARE TYPE EXEMPTION"; // 32
    _msgMap[FARE_CLASS] = "FARE CLASS EXEMPTION"; // 33
    _msgMap[APPLY_TOO_MANY] = "MAXIMUM TAX APPLIED"; // 34
    _msgMap[SALE_COUNTRY] = "SALE COUNTRY RESTRICTION"; // 35
    _msgMap[NO_TAX_VECTOR] = "NO TAX VECTORS BUILT"; // 36
    _msgMap[OLD_TAX_ITEM] = "TAX ITEM DATE INVALID"; // 37
    _msgMap[OVERRIDE] = "TAX OVERRIDE ENCOUNTERED"; // 38
    _msgMap[EXEMPT_ALL_TAXES] = "ALL TAXES ARE EXEMPTED PER ENTRY"; // 39
    _msgMap[NO_TAX_NATION_FOUND] = "TAX NATION ORDER TABLE NOT LOCATED"; // 40
    _msgMap[NO_TAX_CODE] = "TAX CODE MISSING FROM THE TAX TABLE"; // 41
    _msgMap[NO_TAX_ADDED] = "REJECT SPECIAL PROCESSING TAX"; // 42
    _msgMap[CURRENCY_CONVERTER_BSR] = "CURRENCY CONVERSION FAILURE"; // 43
    _msgMap[CURRENCY_CONVERTER_NUC] = "NUC CURRENCY CONVERSION FAILURE"; // 44
    _msgMap[APPLY_ONCE_PER_ITIN] = "TAX RESTRICTED TO BE APPLIED ONCE PER ITINERARY"; // 45
    _msgMap[ROUNDING_ERROR] = "ROUNDING ERROR ENCOUNTERED"; // 46
    _msgMap[ENPLANEMENT_LOCATION] = "ENPLANEMENT JOURNEY RESTRICTION"; // 47
    _msgMap[DEPLANEMENT_LOCATION] = "DEPLANEMENT JOURNEY RESTRICTION"; // 48
    _msgMap[DESTINATION_LOCATION] = "DESTINATION JOURNEY RESTRICTION"; // 49
    _msgMap[TERMINATION_LOCATION] = "TERMINATION JOURNEY RESTRICTION"; // 50
    _msgMap[BAD_DATA_HANDLER_POINTER] = "BAD DATAHANDLER POINTER"; // 51
    _msgMap[RANGE_FARE] = "FARE RANGE RESTRICTION"; // 52
    _msgMap[RANGE_MILES] = "MILEAGE RANGE RESTRICTION"; // 53
    _msgMap[NUM_COUPON_BOOKLET] = "COUPON BOOKLET REATRICTION"; // 54
    _msgMap[TAX_ONCE_PER_BOARD_OFF] = "APPLIED TAX ONCE PER CITY PAIR"; // 55
    _msgMap[EQUIPMENT_TYPE] = "EQUIPMENT RESTRICTION"; // 56
    _msgMap[CARRIER_EXEMPTION] = "CARRIER RESTRICTION"; // 57
    _msgMap[NO_US_STOPOVER] = "NO US STOPOVER LOCATED"; // 58
    _msgMap[PARTIAL_TAX] = "PARTIAL TAX QUALIFIER NOT MET"; // 59
    _msgMap[NO_ZP_PAX_CMP] = "COMPANION PASSENGER RESTRICTION"; // 60
    _msgMap[TAX_ONCE_PER_SEGMENT] = "APPLY ONCE PER SEGMENT"; // 61
    _msgMap[CLASS_OF_SERVICE] = "CLASS OF SERVICE RESTRICTION"; // 62
    _msgMap[TICKET_DESIGNATOR] = "TICKET DESIGNATOR EXEMPTION"; // 63
    _msgMap[JOURNEY] = "YQYR JOURNEY";
    _msgMap[PORTION] = "YQYR PORTION";
    _msgMap[SECTOR] = "YQYR SECTOR";
    _msgMap[LOC1_TRANSFER_TYPE] = "LOC1 TRANSFER TYPE";
    _msgMap[JOURNEY_TYPE] = "JOURNEY TYPE";
    _msgMap[PARTITION_ID] = "INVALID PARTITION";
    _msgMap[ORIGINAL_TICKET_ONLY] = "ORIGINAL TICKET ONLY";
  }
}

void
TaxDiagnostic::displayTaxRetrievalDate(PricingTrx& trx,
                                       const TaxResponse& taxResponse,
                                       DiagCollector& diag)
{
  if (trx.excTrxType() == PricingTrx::PORT_EXC_TRX)
  {
    BaseExchangeTrx& excTrx = static_cast<BaseExchangeTrx&>(trx);
    if (excTrx.applyReissueExchange())
    {
      diag << " \n TAX RETRIEVAL DATE : " << trx.getRequest()->ticketingDT().toIsoExtendedString()
           << "\n";
    }
  }
  else if (RexPricingTrx::isRexTrxAndNewItin(trx) &&
           static_cast<RexBaseTrx&>(trx).applyReissueExchange() && taxResponse.farePath() != nullptr)
  {
    ReissueExchangeDateSetter dateSetter(trx, *(taxResponse.farePath()));
    diag << "\nTAX RETRIEVAL DATE : "
         << static_cast<RexBaseTrx&>(trx).ticketingDate().toIsoExtendedString() << "\n";
  }
}

bool
TaxDiagnostic::isTaxDiagnostic(PricingTrx& trx)
{
  if (LIKELY(trx.diagnostic().diagnosticType() < TAXES_DIAG_RANGE_BEGIN ||
      trx.diagnostic().diagnosticType() > TAXES_DIAG_RANGE_END))
    return false;
  else
    return true;
}

bool
TaxDiagnostic::isValidNation(PricingTrx& trx, const NationCode& nation)
{
  if (LIKELY(!isTaxDiagnostic(trx)))
    return true;

  const std::string& strVal = trx.diagnostic().diagParamMapItem("PN");
  if (!strVal.empty() && strVal!=nation)
    return false;

  return true;
}

bool
TaxDiagnostic::isValidTaxCode(PricingTrx& trx, const TaxCode& code)
{
  if (LIKELY(!isTaxDiagnostic(trx)))
    return true;

  const std::string& strVal = trx.diagnostic().diagParamMapItem("PT");
  if (!strVal.empty() && strVal!=code)
    return false;

  return true;
}

bool
TaxDiagnostic::isValidSeqNo(PricingTrx& trx, int seq)
{
  if (LIKELY(!isTaxDiagnostic(trx)))
    return true;

  const std::string& strVal = trx.diagnostic().diagParamMapItem("PS");
  if (!strVal.empty())
  {
    try
    {
      if (seq != std::stoi(strVal))
        return false;
    }
    catch(std::invalid_argument&){}
    catch(std::out_of_range&){}
  }

  return true;
}

bool
TaxDiagnostic::isSpnOff(PricingTrx& trx, int spn)
{
  if (LIKELY(!isTaxDiagnostic(trx)))
    return false;

  const std::string& strVal = trx.diagnostic().diagParamMapItem("SS");
  if (!strVal.empty())
  {
    try
    {
      if (spn == std::stoi(strVal))
        return true;
    }
    catch(std::invalid_argument&){}
    catch(std::out_of_range&){}
  }

  return false;
}

bool
TaxDiagnostic::printSeqDef(PricingTrx& trx, TaxCodeReg& taxCodeReg)
{
  if (LIKELY(trx.diagnostic().diagnosticType() != Diagnostic824))
    return false;

  DiagManager diag(trx, Diagnostic824);

  diag << "***\n" << taxCodeReg.taxCode() << taxCodeReg.seqNo() <<
    "\n  SPECIAL TAX NUMBER: " << taxCodeReg.specialProcessNo() <<
    "\n  DECIMAL PLACE: " << taxCodeReg.taxNodec() <<
    "\n  ROUND DECIMAL PLACE: " << taxCodeReg.taxcdRoundUnitNodec() <<
    "\n  DISCOUNT PERCENTAGE: " << taxCodeReg.discPercent() <<
    "\n  DISCOUNT PERCENTAGE DECIMAL PLACE: " << taxCodeReg.discPercentNodec() <<
    "\n  TAX AMOUNT: " << taxCodeReg.taxAmt() <<
    "\n  MINIMUM TAX: " << taxCodeReg.minTax() <<
    "\n  MAXIMUM TAX: " << taxCodeReg.maxTax() <<
    "\n  PLUS UP AMOUNT: " << taxCodeReg.plusupAmt() <<
    "\n  LOW RANGE: " << taxCodeReg.lowRange() <<
    "\n  HIGH RANGE: " << taxCodeReg.highRange() <<
    "\n  RANGE INCREMENT: " << taxCodeReg.rangeincrement() <<
    "\n  FARE RANGE DECIMAL PLACE: " << taxCodeReg.fareRangeNodec() <<
    "\n  LOCATION 1 EXCLUDED: " << taxCodeReg.loc1ExclInd() <<
    "\n  LOCATION 1: " << taxCodeReg.loc1() <<
    "\n  LOCATION 1 TYPE: " << taxCodeReg.loc1Type() <<
    "\n  LOCATION 2 EXCLUDED: " << taxCodeReg.loc2ExclInd() <<
    "\n  LOCATION 2: " << taxCodeReg.loc2() <<
    "\n  LOCATION 2 TYPE: " << taxCodeReg.loc2Type() <<
    "\n  TYPE: " << taxCodeReg.taxType() <<
    "\n  TAX NATION: " << taxCodeReg.nation() <<
    "\n  ROUND RULE: " << toString(taxCodeReg.taxcdRoundRule()) <<
    "\n  FULL FARE: " << taxCodeReg.taxfullFareInd() <<
    "\n  EQUIVALENT AMOUNT: " << taxCodeReg.taxequivAmtInd() <<
    "\n  EXCESS BAGGAGE: " << taxCodeReg.taxexcessbagInd() <<
    "\n  TRAVEL DATE AS ORIGIN: " << taxCodeReg.tvlDateasoriginInd() <<
    "\n  DISPLAY ONLY: " << taxCodeReg.displayonlyInd() <<
    "\n  FEE: " << taxCodeReg.feeInd() <<
    "\n  INTERLINABLE: " << taxCodeReg.interlinableTaxInd() <<
    "\n  SEPARATE: " << taxCodeReg.showseparateInd() <<
    "\n  POINT OF SALE EXCLUDED: " << taxCodeReg.posExclInd() <<
    "\n  POINT OF SALE LOCATION TYPE: " << taxCodeReg.posLocType() <<
    "\n  POINT OF SALE LOCATION: " << taxCodeReg.posLoc() <<
    "\n  POINT OF ISSUANCE EXCLUDE: " << taxCodeReg.poiExclInd() <<
    "\n  POINT OF ISSUANCE LOCATION TYPE: " << taxCodeReg.poiLocType() <<
    "\n  POINT OF ISSUANCE LOCATION: " << taxCodeReg.poiLoc() <<
    "\n  SELL CURRENCY EXCLUDE: " << taxCodeReg.sellCurExclInd() <<
    "\n  SELL CURRENCY: " << taxCodeReg.sellCur() <<
    "\n  OCCURENCE: " << occurenceToString(taxCodeReg.occurrence()) <<
    "\n  FREE TICKET EXEMPT: " << taxCodeReg.freeTktexempt() <<
    "\n  ID TRAVEL EXEMPT: " << taxCodeReg.idTvlexempt() <<
    "\n  RANGE TYPE: " << taxCodeReg.rangeType() <<
    "\n  RANGE INDICATOR: " << taxCodeReg.rangeInd() <<
    "\n  NEXT STOP OVER RESTRICTION: " << taxCodeReg.nextstopoverrestr() <<
    "\n  SPECIAL ROUNDING: " << taxCodeReg.spclTaxRounding() <<
    "\n  CURRENCY: " << taxCodeReg.taxCur() <<
    "\n  CURRENCY DECIMAL PLACES: " << taxCodeReg.taxCurNodec() <<
    "\n  CARRIER EXCLUDE: " << taxCodeReg.valcxrExclInd() <<
    "\n  EXEMPT EQUIPMENT EXCLUDE: " << taxCodeReg.exempequipExclInd() <<
    "\n  PASSENGER EXCLUDE: " << taxCodeReg.psgrExclInd() <<
    "\n  FARE TYPE EXCLUDE: " << taxCodeReg.fareTypeExclInd() <<
    "\n  MULTI OCCURRENCE: " << taxCodeReg.multioccconvrndInd() <<
    "\n  ORIGIN LOCATION TYPE: " << taxCodeReg.originLocType() <<
    "\n  ORIGIN LOCATION: " << taxCodeReg.originLoc() <<
    "\n  ORIGIN LOCATION EXCLUDE: " << taxCodeReg.originLocExclInd() <<
    "\n  LOCATION 1 APPLY: " << taxCodeReg.loc1Appl() <<
    "\n  LOCATION 2 APPLY: " << taxCodeReg.loc2Appl() <<
    "\n  TRIP TYPE: " << tripTypeToString(taxCodeReg.tripType()) <<
    "\n  TRAVEL TYPE: " << travelTypeToString(taxCodeReg.travelType()) <<
    "\n  ITINERARY TYPE: " << itinTypeToString(taxCodeReg.itineraryType());

  printTaxOnTaxInfo(diag, taxCodeReg, trx);
  printUtcInfo(trx, diag, taxCodeReg);
  printDatesInfo(diag, taxCodeReg);

  printExemptionCxr(diag, taxCodeReg);
  printRestrictionFareClass(diag, taxCodeReg);
  printTransit(diag, taxCodeReg);
  printTicketDesignator(diag, taxCodeReg);

  diag << "\n";

  return true;
}

void
TaxDiagnostic::printExemptionCxr(DiagManager& diag, TaxCodeReg& taxCodeReg)
{
  if (!taxCodeReg.exemptionCxr().empty())
  {
    diag << "\n  EXEMPT CXR EXCLUDE: " << taxCodeReg.exempcxrExclInd();
    diag << "\n  EXEMPT CXR (COUNT-" << taxCodeReg.exemptionCxr().size() << "): ";
    for(const TaxExemptionCarrier& data: taxCodeReg.exemptionCxr())
    {
      diag << "\n    - CARIER-" << data.carrier() << " FLT1-" << data.flight1() << " FLT2-"
        << data.flight2() << " DIR-" << data.direction() << "AIRPORT1-" << data.airport1()
        << " AIRPORT2-" << data.airport2() << "\n";
    }
  }
}

void
TaxDiagnostic::printRestrictionFareClass(DiagManager& diag, TaxCodeReg& taxCodeReg)
{
  if (!taxCodeReg.restrictionFareClass().empty())
  {
    diag << "\n  FARE CLASS EXCLUDE: " << taxCodeReg.fareclassExclInd();
    diag << "\n  FARE CLASS (COUNT-" << taxCodeReg.exemptionCxr().size() << ")\n";
    std::string strLine;
    for(const FareClassCode& data: taxCodeReg.restrictionFareClass())
    {
      strLine += !strLine.empty() ? ", " : "";
      strLine += static_cast<std::string>(data);
      if (strLine.length() > 50)
      {
        diag << "      - " << strLine << "\n";
        strLine.erase();
      }
    }

    if (!strLine.empty())
      diag << "      - " << strLine << "\n";
  }
}

void
TaxDiagnostic::printTransit(DiagManager& diag, TaxCodeReg& taxCodeReg)
{
  if (!taxCodeReg.restrictionTransit().empty())
  {
    diag << "\n  TRANSIT (COUNT-" << taxCodeReg.restrictionTransit().size() << "): ";
    for(const TaxRestrictionTransit& data: taxCodeReg.restrictionTransit())
    {
      diag << "\n    - H-";
      if (data.transitHours() >=0)
        diag << data.transitHours();
      diag << " M-";
      if (data.transitMinutes() >= 0)
        diag << data.transitMinutes();

      diag << " SAMEDAY-" << data.sameDayInd() << " NEXTDAY-" << data.nextDayInd()
        << " SAMEFLT-" << data.sameFlight() << " TAXONLY-" << data.transitTaxonly()
           << "\n      DOM2DOM-" << data.transitDomDom() << " DOM2INTL-" << data.transitDomIntl()
        << " INTL2DOM-" << data.transitIntlDom() << " INTL2INTL-" << data.transitIntlIntl()
        << " SURF2DOM-" << data.transitSurfDom()
           << "\n      SURF2INTL-" << data.transitSurfIntl()
        << " OFFLINE-" << data.transitOfflineCxr()
        << " FLTARR-" << data.flightArrivalHours() << ":" << data.flightArrivalMinutes()
        << " FLTDEP-" << data.flightDepartHours() << ":" << data.flightDepartMinutes()
           << "\n      VIALOC-" << locTypeToString(data.viaLocType()) << ":" << data.viaLoc()
        << "\n";
    }
  }
}

void
TaxDiagnostic::printTicketDesignator(DiagManager& diag, TaxCodeReg& taxCodeReg)
{
if (!taxCodeReg.restrictionFareClass().empty())
  {
    diag << "\n  TICKET DESIGNATOR EXCLUDE: " << taxCodeReg.tktdsgExclInd();
    diag << "\n  TICKET DESIGNATOR (COUNT-" << taxCodeReg.taxRestrTktDsgs().size() << ")\n";
    std::string strLine;
    for(const TaxRestrictionTktDesignator* data: taxCodeReg.taxRestrTktDsgs())
    {
      if (!data)
        continue;

      strLine += !strLine.empty() ? ", " : "";
      strLine += data->tktDesignator() + "/" + data->carrier();
      if (strLine.length() > 50)
      {
        diag << "      - " << strLine << "\n";
        strLine.erase();
      }
    }

    if (!strLine.empty())
      diag << "      - " << strLine << "\n";
  }
}

void
TaxDiagnostic::printDatesInfo(DiagManager& diag, TaxCodeReg& taxCodeReg)
{
  diag  <<
    "\n  CREATE DATE: " << taxCodeReg.createDate() <<
    "\n  EXPIRE DATE: " << taxCodeReg.expireDate() <<
    "\n  LOCK DATE: " << taxCodeReg.lockDate() <<
    "\n  EFFECTIVE DATE: " << taxCodeReg.effDate() <<
    "\n  DISCONTINUE DATE: " << taxCodeReg.discDate() <<
    "\n  FIRST TRAVEL DATE: " << taxCodeReg.firstTvlDate() <<
    "\n  LAST TRAVEL DATE: " << taxCodeReg.lastTvlDate() <<
    "\n  MOD DATE: " << taxCodeReg.modDate();
}

void
TaxDiagnostic::printTaxOnTaxInfo(DiagManager& diag, TaxCodeReg& taxCodeReg, PricingTrx& trx)
{
  if (!taxCodeReg.taxOnTaxCode().empty())
  {
    diag << "\n  TAX ON TAX EXCLUDE FARE AMOUNT: " << taxCodeReg.taxOnTaxExcl();
    diag << "\n  TAX ON TAX CODE (COUNT-" << taxCodeReg.taxOnTaxCode().size() << "): ";

    bool bComa = false;
    for(const std::string& code: taxCodeReg.taxOnTaxCode())
    {
      diag << (bComa ? ", " : "") << code;
      bComa = true;
    }
  }
}

void
TaxDiagnostic::printUtcInfo(PricingTrx& trx, DiagManager& diag, TaxCodeReg& taxCodeReg)
{
  diag << "\n  SPECIAL CONFIGURATION: " << taxCodeReg.specConfigName();
  if (!taxCodeReg.specConfigName().empty())
  {
    std::vector<TaxSpecConfigReg*> tscv =
      trx.dataHandle().getTaxSpecConfig(taxCodeReg.specConfigName());
    for (const TaxSpecConfigReg* cit : tscv)
    {
      diag <<
        "\n    EFFECTIVE DATE: " << cit->effDate().dateToIsoExtendedString() <<
        "\n    DISCONTINUE DATE: " << cit->discDate().dateToIsoExtendedString();
      for (const TaxSpecConfigReg::TaxSpecConfigRegSeq* sit : cit->seqs())
        diag << setw(20) << "\n    " << sit->paramName() << " : " << sit->paramValue();
    }
  }
}

void
TaxDiagnostic::printHelpInfo(PricingTrx& trx)
{
  if (trx.diagnostic().diagnosticType() != Diagnostic824 &&
      trx.diagnostic().diagnosticType() != FailTaxCodeDiagnostic)
    return;

  DiagManager diag(trx, trx.diagnostic().diagnosticType());
  if (!trx.diagnostic().diagParamMapItemPresent("HE"))
  {
    diag << "HELP - DISPLAY DETAILED INFORMATION\n***\n";
    return;
  }

  diag << "HELP - \n" <<
     "  FOR TAX DATABASE INTERROGATION USE DIAGNOSTIC: " << Diagnostic824  << "\n" <<
     "  PARAMETERS USED BY ALL TAX DIAGNOSTICS\n" <<
     "  PNXX - PROCESS TAXES ONLY FOR NATION XX\n" <<
     "  PTXXX - PROCESS TAXES ONLY FOR TAX CODE XXX\n" <<
     "  PSXXX - PROCESS TAXES ONLY FOR TAX SEQUENCE NUMBER XXX\n" <<
     "  SSXXX - SKIP SPECIAL PROCES NUMBER XXX / SWITCH OFF SPN\n";

   diag << "\n***\n";
}

std::string
TaxDiagnostic::toString(const RoundingRule& rule)
{
  std::string strVal = std::to_string(rule);
  switch (rule)
  {
  case RoundingRule::DOWN:
    strVal += " - DOWN";
    break;
  case RoundingRule::UP:
    strVal += " - UP";
    break;
  case RoundingRule::NEAREST:
    strVal += " - NEAREST";
    break;
  case RoundingRule::NONE:
    strVal += " - NONE";
    break;
  case RoundingRule::EMPTY:
    strVal += " - EMPTY";
    break;
  }

  return strVal;
}

std::string
TaxDiagnostic::occurenceToString(const Indicator& ind)
{
  std::string strVal(1, ind);
  switch (ind)
  {
  case 'O':
    strVal += " - APPLY ONCE PER TAX SEQUENCE";
    break;
  case 'M':
    strVal += " - APPLY MULTIPLE TIMES";
    break;
  case 'B':
    strVal += " - APPLY PER TICKET BOOKLET";
    break;
  case 'C':
    strVal += " - APPLY ONCE PER TAX CODE";
    break;
  }

  return strVal;
}

std::string
TaxDiagnostic::tripTypeToString(const Indicator& ind)
{
  std::string strVal(1, ind);
  switch (ind)
  {
  case TripTypesValidator::TAX_FROM_TO:
    strVal += " - FROM TO";
    break;
  case TripTypesValidator::TAX_BETWEEN:
    strVal += " - BETWEEN";
    break;
  case TripTypesValidator::TAX_WITHIN_SPEC:
    strVal += " - WITHIN SPECIFIED";
    break;
  case TripTypesValidator::TAX_WITHIN_WHOLLY:
    strVal += " - WHOLY WITHIN";
    break;
  }

  return strVal;
}

std::string
TaxDiagnostic::travelTypeToString(const Indicator& ind)
{
  std::string strVal(1, ind);
  switch (ind)
  {
  case 'D':
    strVal += " - DOMESTIC";
    break;
  case 'I':
    strVal += " - INTERNATIONAL";
    break;
  }

  return strVal;
}

std::string
TaxDiagnostic::itinTypeToString(const Indicator& ind)
{
  std::string strVal(1, ind);
  switch (ind)
  {
  case 'O':
    strVal += " - ONE WAY";
    break;
  case 'R':
    strVal += " - ROUND TRIP";
    break;
  case 'J':
    strVal += " - OPEN JAW";
    break;
  }

  return strVal;
}

std::string
TaxDiagnostic::locTypeToString(const LocType& locType)
{
  std::string strVal(1, locType==UNKNOWN_LOC ? ' ' : locType);
  switch (locType)
  {
  case IATA_AREA:
    strVal += "-IATA AREA";
    break;
  case SUBAREA:
    strVal += "-SUBAREA";
    break;
  case MARKET:
    strVal += "-MARKET";
    break;
  case NATION:
    strVal += "-NATION";
    break;
  case STATE_PROVINCE:
    strVal += "-STATE PROVINCE";
    break;
  case ZONE:
    strVal += "-ZONE";
    break;

  case UNKNOWN_LOC: //to avoid gcc warning
    break;
  }

  return strVal;
}
