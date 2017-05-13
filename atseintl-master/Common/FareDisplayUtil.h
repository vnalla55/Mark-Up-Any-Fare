//----------------------------------------------------------------------------
//
//  File:        FareDisplayUtil.h
//  Created:     04/25/2005
//  Authors:     Mike Carroll
//
//  Description: Common functions required for ATSE fare display.
//
//  Copyright Sabre 2007
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

#include "Common/MultiTransportMarkets.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/Vendor.h"
#include "FareDisplay/FDConsts.h"

#include <set>

namespace tse
{
class DataHandle;
class PricingTrx;
class FareDisplayTrx;
class PaxTypeFare;
class FarePath;
class DateTime;
class FlightCount;
class MultiTransportMarkets;

class FareDisplayUtil
{
public:
  // -------------------------------------------------------------------
  //
  // @MethodName  FareDisplayUtil::getCRSParams()
  //
  // Evaluate and set argument pseudoCity and pseudoCity type based on the
  // agency record.
  //
  // @param trx - a valid FareDisplayTrx
  // @param userApplType - reference to a userApplType
  // @param userAppl - reference to a userAppl
  //
  // @return bool - true is values present, false otherwise
  // -------------------------------------------------------------------------
  static const bool
  getCRSParams(FareDisplayTrx& trx, Indicator& userApplType, UserApplCode& userAppl);

  /**
  * Utility method to dynamic cast a PricingTrx to FareDisplayTrx.
  * @param PricingTrx* A pointer to the base object.
  * @param FareDisplayTrx* A pointer to the derived object to which we want to dynamic cast.
  * @returns true if the cast is successfull, false othewise.
  */
  static bool getFareDisplayTrx(PricingTrx*, FareDisplayTrx*&);

  //---------------------------------------------------------------------------
  // To get a new temporary FareDisplayInfo and initialize it
  //---------------------------------------------------------------------------
  static bool initFareDisplayInfo(FareDisplayTrx* fdTrx, PaxTypeFare& ptFare);

  //---------------------------------------------------------------------------
  // Get a FareDisplayTrx and the FareDisplayInfo object, and set the Display
  // Only indicator to True
  //---------------------------------------------------------------------------
  static void
  setDisplayOnly(PricingTrx& pTrx, PaxTypeFare& ptFare, bool isDisplayOnly, bool isMinMaxFare);

  //---------------------------------------------------------------------------
  // Get the total fare for each paxTypeFare including taxes and surcharges.
  //---------------------------------------------------------------------------
  static void getTotalFares(FareDisplayTrx& pTrx);
  static void getTotalFare(FareDisplayTrx& trx, PaxTypeFare& paxTypeFare);

  // -------------------------------------------------------------------
  //
  // @MethodName  FareDisplayUtil::getUniqueSetOfCarriers()
  //
  // Build a set of unique carriers that have fares in the market.
  //
  // @param   trx        - the transaction to use
  // @param   includeAddon - flag to indicate whether addon fares should be
  //                         considered or not
  // @param   mktCarrierMap - map to be polulated with all markets, and for
  //                          each market the corresponding set of carriers
  //                          that publish fares
  //
  // @return  unique set of carriers
  //
  // -------------------------------------------------------------------------
  static const std::set<CarrierCode>& getUniqueSetOfCarriers(
      FareDisplayTrx& trx,
      bool includeAddon,
      std::map<MultiTransportMarkets::Market, std::set<CarrierCode>>* marketCarrierMap);

  // -------------------------------------------------------------------
  //
  // @MethodName  FareDisplayUtil::formatMonth()
  //
  // Formats month value base on template formatting type value
  //
  // @param int32_t&  month value
  // @param ostringstream& oss
  // @param Indicator& dateFormat
  //
  // @return  The formatted information
  //
  // -------------------------------------------------------------------------
  static void
  formatMonth(int32_t& monthValue, std::ostringstream& oss, const Indicator& dateFormat);

  // -------------------------------------------------------------------
  //
  // @MethodName  FareDisplayUtil::formatYear()
  //
  // Formats year value base on template formatting type value
  //
  // @param int32_t&  year value
  // @param ostringstream& oss
  // @param Indicator& dateFormat
  //
  // @return  The formatted information
  //
  // -------------------------------------------------------------------------
  static void formatYear(int32_t& yearValue, std::ostringstream& oss, const Indicator& dateFormat);

  static std::string getAddonZoneDescr(
      const VendorCode&, const CarrierCode&, TariffNumber, AddonZone, const FareDisplayTrx& fdTrx);

  static std::string getZoneDescr(const Zone& zone,
                                  const FareDisplayTrx& fdTrx,
                                  const VendorCode& vendor = Vendor::SABRE,
                                  Indicator zoneType = MANUAL);

  static std::string splitLines(const std::string& text,
                                size_t lineLength,
                                const std::string& terminators,
                                size_t firstLineLength,
                                const std::string& prefix = "");

  static MPType determineMPType(const FareDisplayTrx&);

  // TODO - Remove this method once full CAT 15 can be processed during
  //        prevalidation.
  // Method to determine if full CAT 15 validation should be done during FCO
  // prevalidation.
  static bool isCat15TuningEnabled();

  // Method to determine if brand grouping is enabled.
  static bool isBrandGroupingEnabled();

  // Method to determine if brand grouping is enabled.
  static bool isBrandServiceEnabled();

  // Method to Check if needs to be processed.
  static bool isBrandGrouping(FareDisplayTrx& trx);

  // Method to get the maximum nbr of fares allowed
  static uint32_t getMaxFares();

  // -------------------------------------------------------------------
  //
  // @MethodName  FareDisplayUtil::getAltCxrMsg()
  //
  // Retrieves the FQ message saying that other carriers publish fares for the
  // given market.
  //
  // @param orig Origin
  // @param dest Destination
  //
  // @return The string requested
  //
  // -------------------------------------------------------------------------
  static std::string getAltCxrMsg(const LocCode& orig, const LocCode& dest);

  // -------------------------------------------------------------------
  //
  // @MethodName  FareDisplayUtil::getConnectingFlightCount()
  //
  // Retrieves the connecting flight count field for the given FlightCount data
  // that is displayed in FQ shopper and SDS responses.
  //
  // @param fc FlightCount information
  //
  // @return Connecting flight count in string form
  //
  // -------------------------------------------------------------------------
  static std::string getConnectingFlightCount(const FareDisplayTrx& trx, const FlightCount& fc);

  // -------------------------------------------------------------------
  //
  // @MethodName  FareDisplayUtil::isWebUser()
  //
  // Determines whether or not the request is made by a web user.
  //
  // @param trx The FareDisplayTrx object reference
  //
  // @return A boolean indicating if the request was made by a web user
  //
  // -------------------------------------------------------------------------
  static bool isWebUser(const FareDisplayTrx& trx);

  // -------------------------------------------------------------------
  //
  // @MethodName  FareDisplayUtil::isAxessUser()
  //
  // Determines whether or not the request is made by an Axess user.
  //
  // @param trx The FareDisplayTrx object reference
  //
  // @return A boolean indicating if the request was made by an Axess user
  //
  // -------------------------------------------------------------------------
  static bool isAxessUser(const FareDisplayTrx& trx);

  // -------------------------------------------------------------------
  //
  // @MethodName  FareDisplayUtil::isQualifiedRequest()
  //
  // Determines whether or not the request is has qualifiers.
  //
  // @param trx The FareDisplayTrx object reference
  //
  // @return A boolean indicating if the request is qualified
  //
  // -------------------------------------------------------------------------
  static bool isQualifiedRequest(const FareDisplayTrx& trx);

  // -------------------------------------------------------------------
  //
  // @MethodName  FareDisplayUtil::isHostDBInfoDiagRequest()
  //
  // Determines whether or not the request is to display host and db info.
  //
  // @param trx The FareDisplayTrx object reference
  //
  // @return A boolean indicating if the request is to diosplay host/db info.
  //
  // -------------------------------------------------------------------------
  static bool isHostDBInfoDiagRequest(const FareDisplayTrx& trx);

  // -------------------------------------------------------------------
  //
  // @MethodName  FareDisplayUtil::validCarrierFaresExist()
  //
  // Determines whether or not the transaction contains valid carrier fares.
  //
  // @param trx The FareDisplayTrx object reference
  //
  // @return A boolean indicating if the request has carrier fares.
  //
  // -------------------------------------------------------------------------
  static bool validCarrierFaresExist(const FareDisplayTrx& trx);

  // -------------------------------------------------------------------
  //
  // @MethodName  FareDisplayUtil::validYYFaresExist()
  //
  // Determines whether or not the transaction contains valid YY fares.
  //
  // @param trx The FareDisplayTrx object reference
  //
  // @return A boolean indicating if the request has YY fares.
  //
  // -------------------------------------------------------------------------
  static bool validYYFaresExist(const FareDisplayTrx& trx);

  // -------------------------------------------------------------------
  //
  // @MethodName  FareDisplayUtil::getCarrierFareHeaderMsg()
  //
  // Retrieves the header msg that says that no carrier fares exist.
  //
  // @param trx The FareDisplayTrx object reference
  // @param s std::ostringstream object to write the message to
  //
  // @return A boolean indicating if the msg is applicable.
  //
  // -------------------------------------------------------------------------
  static bool
  getCarrierFareHeaderMsg(const FareDisplayTrx& trx, std::ostringstream& s, bool padding = true);

  static void displayNotPublish(const FareDisplayTrx& trx,
                                std::ostringstream& p0,
                                InclusionCode& incCode,
                                std::string& privateString,
                                bool padding,
                                bool firstLine = true);
  // -------------------------------------------------------------------
  //
  // @MethodName  FareDisplayUtil::shortRequestForPublished()
  //
  // Checks if it is a sort request for published fare.
  //
  // @param trx The PricingTrx object reference
  //
  //
  // @return True if it is a short request for published fare,
  //         False otherwise.
  //
  // -------------------------------------------------------------------------
  static bool shortRequestForPublished(PricingTrx& trx);

  // -------------------------------------------------------------------
  //
  // @MethodName  FareDisplayUtil::getZones()
  //
  // Checks if it is a sort request for published fare.
  //
  // @param VendorCode&  -const reference to Vendor Code
  //        CarrierCode& -const reference to Carrier Code
  //        TariffNumber - AddOn Tariff number
  //        AddOn Zone   - AddonZone,
  //        DateTime&    - const reference to Travel date
  //        string&      - reference to string;
  //
  // @return True if vector of Zones is not empty
  //
  // -------------------------------------------------------------------------
  static bool getZones(DataHandle& dh,
                       const VendorCode&,
                       const CarrierCode&,
                       TariffNumber,
                       AddonZone,
                       const DateTime&,
                       std::string&);

  static bool initializeFarePath(const FareDisplayTrx& trx, FarePath* farePath);

  static const LocCode& getFareOrigin(PaxTypeFare* paxTypeFare);
  static const LocCode& getFareDestination(PaxTypeFare* paxTypeFare);
  static bool updateCreateTimeFormat(std::string& crTime, bool format);

  // -------------------------------------------------------------------
  // @MethodName  FareDisplayUtil::isXMLDiagRequested
  // Check whether or not diagnostic 195/DD<diagParam> was requested
  // -------------------------------------------------------------------
  static bool isXMLDiagRequested(FareDisplayTrx& trx, const std::string& diagParam);

  // -------------------------------------------------------------------
  // @MethodName  FareDisplayUtil::displayXMLdiag
  // Display XML request or response for diagnostic purposes
  // -------------------------------------------------------------------
  static void displayXMLDiag(FareDisplayTrx& trx,
                             const std::string& xmlData,
                             const std::string& diagHeader,
                             StatusBrandingService status = StatusBrandingService::NO_BS_ERROR);

  // -------------------------------------------------------------------
  // @MethodName  FareDisplayUtil::isFrrCustomer
  // Check Fare Retailer customers to hide RR on the rule display
  // -------------------------------------------------------------------
  static bool isFrrCustomer(PricingTrx& trx);

  // Some common static consts used to build display and SDS responses.
  static const std::string ALT_CXR_YY_MSG;
  static const std::string INTERLINE_INDICATOR;
  static constexpr uint16_t MAX_ALLOWED_ONLINE_CONNECTION = 99;

  static const std::string DISPLAY_DETAIL;
  static const std::string HOST;
};
} // end tse namespace
