//-------------------------------------------------------------------
//
//  File:        FareDisplayResponseXMLTags.h
//  Created:     March 8, 2006
//  Authors:     Hitha Alex
//
//  Updates:
//
//  Copyright Sabre 2006
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"


namespace tse
{

class DateTime;
class FareDisplayTrx;
class FareDisplayInfo;
class PaxTypeFare;
class SeasonsInfo;
class Fare;
class FareDisplaySDSTaxInfo;
class TaxRecord;
class TaxItem;

class FareDisplayResponseXMLTags
{
public:
  //--------------------------------------------------------------------------
  // @function
  //
  // Description: Constructor
  //
  // @param none
  //--------------------------------------------------------------------------
  FareDisplayResponseXMLTags();

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags::~FareDisplayResponseXMLTags
  //
  // Description: Virtual destructor
  //
  // @param none
  //--------------------------------------------------------------------------
  virtual ~FareDisplayResponseXMLTags();

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags::vendorCode
  //
  // Description: Method to get vendorCode tag
  //
  // @param paxTypeFare - a valid PaxTypeFare
  // @param trx         - a valid FareDisplayTrx
  //--------------------------------------------------------------------------
  std::string vendorCode(PaxTypeFare& paxTypeFare, FareDisplayTrx& trx);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags::advancePurchase
  //
  // Description: Method to get advance purchase tag
  //
  // @param fareDisplayInfo - a valid FareDisplayInfo
  //--------------------------------------------------------------------------
  std::string advancePurchase(FareDisplayInfo*& fareDisplayInfo);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags::minMaxStay
  //
  // Description: Methods to get min stay and max stay tags
  //
  // @param fareDisplayInfo - a valid FareDisplayInfo
  //--------------------------------------------------------------------------
  std::string minStay(FareDisplayInfo* fareDisplayInfo);
  std::string maxStay(FareDisplayInfo* fareDisplayInfo);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags::journeyType
  //
  // Description: Method to get journey Type tag
  //
  // @param paxTypeFare - a valid PaxTypeFare
  // @param trx         - a valid FareDisplayTrx
  //--------------------------------------------------------------------------
  std::string journeyType(PaxTypeFare& paxTypeFare, FareDisplayTrx& trx);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags::netFareIndicator
  //
  // Description: Method to get net fare indicator tag
  //
  // @param paxTypeFare - a valid PaxTypeFare
  //--------------------------------------------------------------------------
  std::string netFareIndicator(PaxTypeFare& paxTypeFare);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags::sameDayChange
  //
  // Description: Method to get same day change tag
  //
  // @param fareDisplayInfo - a valid FareDisplayInfo
  //--------------------------------------------------------------------------
  std::string sameDayChange(FareDisplayInfo*& fareDisplayInfo);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags::routing
  //
  // Description: Method to get routing Sequence or routing number
  //
  // @param paxTypeFare     - a valid PaxTypeFare
  // @param fareDisplayInfo - a valid FareDisplayInfo
  //--------------------------------------------------------------------------
  std::string routing(PaxTypeFare& paxTypeFare, FareDisplayInfo*& fareDisplayInfo);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags::season
  //
  // Description: Method to get season application tag
  //
  // @param seasonsInfo - a valid SeasonInfo
  //--------------------------------------------------------------------------
  std::string season(SeasonsInfo*& seasonsInfo);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags::displayCurrency
  //
  // Description: Method to get the display currency code
  //
  // @param trx - a valid FareDisplayTrx
  //--------------------------------------------------------------------------
  std::string displayCurrency(FareDisplayTrx& trx);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags::constructedFareIndicator
  //
  // Description: Method to determine whether a fare is constructed or not
  //
  // @paxTypeFare  - a valid PaxTypeFare
  //--------------------------------------------------------------------------
  std::string constructedFareIndicator(PaxTypeFare& paxTypeFare);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags::industryFareIndicator
  //
  // Description: Method to determine whether a fare is industry or not
  //
  // @paxTypeFare  - a valid PaxTypeFare
  //--------------------------------------------------------------------------
  std::string industryFareIndicator(PaxTypeFare& paxTypeFare);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags::firstSalesDate
  //
  // Description: Method to get first sales date
  //
  // @param fareDisplayInfo - a valid FareDisplayInfo
  //--------------------------------------------------------------------------
  std::string firstSalesDate(FareDisplayInfo*& fareDisplayInfo);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags::effectiveDate
  //
  // Description: Method to get effective date
  //
  // @param fareDisplayInfo - a valid FareDisplayInfo
  //--------------------------------------------------------------------------
  std::string effectiveDate(FareDisplayInfo*& fareDisplayInfo);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags::expirationDate
  //
  // Description: Method to get expiration date
  //
  // @param fareDisplayInfo - a valid FareDisplayInfo
  //--------------------------------------------------------------------------
  std::string expirationDate(FareDisplayInfo*& fareDisplayInfo);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags::lastTicketDate
  //
  // Description: Method to get last ticket date
  //
  // @param fareDisplayInfo - a valid FareDisplayInfo
  //--------------------------------------------------------------------------
  std::string lastTicketDate(FareDisplayInfo*& fareDisplayInfo);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags::cabin
  //
  // Description: Method to determine the cabin
  //
  // @paxTypeFare  - a valid PaxTypeFare
  //--------------------------------------------------------------------------
  std::string cabin(PaxTypeFare& paxTypeFare);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags::cabin
  //
  // Description: Method to determine the cabin and Premium Cabin
  //
  // @paxTypeFare  - a valid PaxTypeFare
  //--------------------------------------------------------------------------
  std::string allCabin(PaxTypeFare& paxTypeFare, FareDisplayTrx& trx);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags::typeOfFare
  //
  // Description: Method to determine type of fare
  //
  // @paxTypeFare  - a valid PaxTypeFare
  // @trx	- a valid FareDisplayTrx
  //--------------------------------------------------------------------------
  std::string typeOfFare(PaxTypeFare& paxTypeFare, FareDisplayTrx& trx);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags::privateFareIndicator
  //
  // Description: Method to determine private fare indicator
  //
  // @param fareDisplayInfo - a valid FareDisplayInfo
  //--------------------------------------------------------------------------
  std::string privateFareIndicator(FareDisplayInfo*& fareDisplayInfo, FareDisplayTrx& trx);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags::corporateId
  //
  // Description: Method to get corporate id
  //
  // @param paxTypeFare  - a valid PaxTypeFare
  // @param trx          - a valid FareDisplayTrx
  //--------------------------------------------------------------------------
  std::string corporateId(PaxTypeFare& paxTypeFare, FareDisplayTrx& trx);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags::baseSellingAmount
  //
  // Description: Method to get base selling amount
  //
  // @paxTypeFare  - a valid PaxTypeFare
  //--------------------------------------------------------------------------
  std::string baseSellingAmount(PaxTypeFare& paxTypeFare, const DateTime& ticketingDate);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags::baseSellingCurrency
  //
  // Description: Method to get base selling currency
  //
  // @paxTypeFare  - a valid PaxTypeFare
  //--------------------------------------------------------------------------
  std::string baseSellingCurrency(PaxTypeFare& paxTypeFare);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags::cat22Indicator
  //
  // Description: Method to get the indicator that tells whether
  //                   a fare is Cat 22 or not
  //
  // @fareDisplayTrx  - a valid FareDisplayTrx
  // @fareDisplayInfo - a valid FareDisplayInfo
  //--------------------------------------------------------------------------
  std::string cat22Indicator(FareDisplayTrx& fareDisplayTrx, FareDisplayInfo*& fareDisplayInfo);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags:: FareAmount
  //
  // Description: Method to get cat 35 fare amount
  //
  // @paxTypeFare  - a valid PaxTypeFare
  //--------------------------------------------------------------------------
  std::string cat35FareAmount(FareDisplayTrx& trx, PaxTypeFare& paxTypeFare);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags:: baseFareAmount
  //
  // Description: Method to get base fare amount
  //
  // @baseFare  - a valid PaxTypeFare
  // @fare - a valid Fare
  //--------------------------------------------------------------------------
  std::string
  baseFareAmount(const PaxTypeFare*& baseFare, const Fare*& fare, const DateTime& ticketingDate);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags:: originalFareAmount
  //
  // Description: Method to get original fare amount
  //
  // @paxTypeFare  - a valid Fare
  //--------------------------------------------------------------------------
  virtual std::string originalFareAmount(const Fare*& fare, const DateTime& ticketingDate);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags:: totalAmount
  //
  // Description: Method to get total amount including tax
  //
  // @fareDisplayTrx	- a valid FareDisplayTrx
  // @fareDisplayInfo - a valid FareDisplay Info
  // @paxTypeFare  - a valid PaxTypeFare
  //--------------------------------------------------------------------------
  std::vector<FareDisplaySDSTaxInfo*> getSDSTaxInfo(FareDisplayTrx& fareDisplayTrx,
                                                    FareDisplayInfo*& fareDisplayInfo,
                                                    PaxTypeFare& paxTypeFare);

  std::vector<FareDisplaySDSTaxInfo*> buildSDSTaxInfo(FareDisplayTrx& fareDisplayTrx,
                                                      PaxTypeFare& paxTypeFare,
                                                      std::vector<TaxRecord*> taxRecordVec,
                                                      std::vector<TaxItem*> taxItemVec,
                                                      std::string owrt);

  std::string totalAmount(const std::vector<FareDisplaySDSTaxInfo*>& sdsTaxInfo,
                          FareDisplayTrx& fareDisplayTrx,
                          PaxTypeFare& paxTypeFare);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags::pricingTicketingRestriction
  //
  // Description: Method to determine cat 35 pricing/ticketing restriction
  //              for selling fares
  //
  // @paxTypeFare  - a valid PaxTypeFare
  //--------------------------------------------------------------------------
  std::string pricingTicketingRestriction(PaxTypeFare& paxTypeFare);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags::oneWayFare
  //
  // Description: Method to get the one way fare
  //
  // @trx          - a valid FareDisplayTrx
  // @paxTypeFare  - a valid PaxTypeFare
  //--------------------------------------------------------------------------
  std::string oneWayFare(FareDisplayTrx& trx, PaxTypeFare& paxTypeFare);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags::roundTripFare
  //
  // Description: Method to get the round Trip Fare
  //
  // @trx          - a valid FareDisplayTrx
  // @paxTypeFare  - a valid PaxTypeFare
  //--------------------------------------------------------------------------
  std::string roundTripFare(FareDisplayTrx& trx, PaxTypeFare& paxTypeFare);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags::yyFareMsg()
  //
  // Description: Method to retrieve YY fare msg
  //
  // @trx  - a FD transaction
  //--------------------------------------------------------------------------
  std::string yyFareMsg(FareDisplayTrx& trx);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags::hdrMsgs()
  //
  // Description: Method to retrieve misc. header messages
  //
  // @param trx a FD transaction
  // @param msgs a vector of strings to hold the header messages
  //--------------------------------------------------------------------------
  void hdrMsgs(FareDisplayTrx& trx, std::vector<std::string>& msgs);

  //--------------------------------------------------------------------------
  // @function FareDisplayResponseXMLTags::currencyConversionHdrMsgs()
  //
  // Description: Method to retrieve currency conversion header messages
  //              (Derived from CurrencyConversionSection and
  //               FQCurrencyConversionSection classes)
  //
  // @param trx a FD transaction
  // @param msgs a vector of strings to hold the header messages
  //--------------------------------------------------------------------------
  void currencyConversionHdrMsgs(FareDisplayTrx& trx, std::vector<std::string>& msgs);

  bool getCurrencyConversionInfo(FareDisplayTrx& trx,
                                 CurrencyCode& sourceCurrency,
                                 CurrencyCode& targetCurrency,
                                 CurrencyCode& intermediateCurrency,
                                 ExchRate& exchangeRate1,
                                 ExchRate& exchangeRate2);

  std::string formatMoneyAmount(const MoneyAmount& amount, CurrencyNoDec& noDec);
  std::string
  formatMoneyAmount(const MoneyAmount& amount, CurrencyCode& ccode, const DateTime& ticketingDate);
  MoneyAmount getTotalTax(const std::vector<FareDisplaySDSTaxInfo*>& sdsTaxInfo);

private:
  FareDisplayResponseXMLTags(const FareDisplayResponseXMLTags& rhs);
  FareDisplayResponseXMLTags& operator=(const FareDisplayResponseXMLTags& rhs);

  void formatSameDayChangeData(FareDisplayInfo& fareDisplayInfo, std::string& sameDayChangeTag);

  void altCurrencyHdrMsgs(FareDisplayTrx& trx, std::vector<std::string>& msgs);

};

} // namespace tse

