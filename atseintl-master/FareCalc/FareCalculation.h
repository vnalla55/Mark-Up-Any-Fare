//----------------------------------------------------------------------------
//  File:        FareCalculation.h
//  Authors:
//  Created:
//
//  Description: Diagnostic 854 formatter
//
//  Updates:
//          date - initials - description.
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

#include "Common/CurrencyConversionRequest.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareUsage.h"
#include "DBAccess/FareCalcConfigText.h"
#include "FareCalc/FcMessage.h"
#include "FareCalc/FcStream.h"


#include <vector>

namespace tse
{
class AirSeg;
class CalcTotals;
class FareCalcCollector;
class FareCalcConfig;
class FarePath;
class FcConfig;
class FcMessageCollection;
class Itin;
class PricingTrx;
class PricingUnit;
class TicketEndorseLine;
class TravelSeg;

class FareCalculation
{
  friend class FareCalculationTest;
  friend class display_endorsements;

public:
  FareCalculation() : _fareCalcDisp(nullptr, FareCalcConsts::FCL_MAX_LINE_LENGTH) {}

  virtual ~FareCalculation() = default;

  virtual void
  initialize(PricingTrx* trx, const FareCalcConfig* fcConfig, FareCalcCollector* fcCollector);

  virtual void process();

  static bool getMsgAppl(FareCalcConfigText::TextAppl, std::string& msg, PricingTrx& pricingTrx);

  static bool getMsgAppl(FareCalcConfigText::TextAppl,
                         std::string& msg,
                         PricingTrx& pricingTrx,
                         const FareCalcConfig& fcConfig);

  static FareUsage::TktNetRemitPscResultVec::const_iterator
  findNetRemitPscResults(const FareUsage& fareUsage, const TravelSeg* tvlSeg);

  void displayCorpIdTrailerMessage();

  static const unsigned int WINDOW_WIDTH = 63;

protected:
  //////////////////////////////////////////////////////////////////////////////
  // Fare Line Format
  //
  void displayPsgFareLineFormat(CalcTotals& calcTotals);
  virtual void displayFareLineInfo(CalcTotals& calcTotals);
  std::string getFareBasis(CalcTotals& calcTotals);
  std::string getBookingCode(CalcTotals& calcTotals);
  void displayFareLineHdr();
  void displayFareLineHdrMultiVersion(bool activeNewVersion);
  virtual void displayTotalFareAmount(CalcTotals& calcTotals);
  void displayCurrencyCode(CalcTotals& calcTotals);
  void displayRO(CalcTotals& calcTotals);
  virtual void displayTaxAmount(CalcTotals& calcTotals, unsigned int taxFieldWidth = 9);
  void displayBaseFareAmount(CalcTotals& calcTotals);

  //
  //////////////////////////////////////////////////////////////////////////////

  void fillLastTicketDay(CalcTotals& calcTotals);

  void displayDtlFareCalc(CalcTotals& calcTotals,
                          CalcTotals& cat35_calcTotals,
                          bool& firstPaxType,
                          bool& ttlProcessed);

  void displayNoMatchMessage(CalcTotals& calcTotals);

  void displayItineraryLine(PricingTrx& trx,
                            FareCalcCollector& fcCollector,
                            CalcTotals& calcTotals,
                            bool firstPaxType);

  void displayLastTicketDate(PricingTrx& trx, CalcTotals& calcTotals);
  void displayTimeStamp(PricingTrx& trx, const DateTime& dt);

  void displayCurrencyTextMessage(const Itin& itin);

  void displayRuler();

  virtual void displayPsgrInfo(const CalcTotals& calcTotals, bool detailFormat = true);

  void displayPsgrType(const CalcTotals& calcTotals);

  void displayCommandPricingAndVariousMessages(CalcTotals* calcTotals = nullptr);

  void displayPtcMessages(PricingTrx& trx, CalcTotals& calcTotals);

  void displayFareTax(PricingTrx& trx, FareCalcCollector& fcCollector, CalcTotals& calcTotals);

  void displayPsgFareCalc(PricingTrx& trx, FareCalcCollector& fcCollector, CalcTotals& calcTotals);

  void displayLineOfFlight(PricingTrx& trx, CalcTotals& calcTotals, char fcConnectionInd);

  virtual void displayCxrBkgCodeFareBasis(PricingTrx& trx,
                                          CalcTotals& calcTotals,
                                          AirSeg* airSeg,
                                          std::string& fareBasis);

  void
  displayNVAandNVBDate(PricingTrx& trx, CalcTotals& calcTotals, AirSeg* airSeg, TravelSeg* tvlS);

  void displayBaggageAllowance(const CalcTotals& calcTotals, const AirSeg* airSeg);

  void displayCxrFareBagHeaderLine();

  void displayPsgrAndFareCalcLines(PricingTrx& trx,
                                   FareCalcCollector& fcCollector,
                                   CalcTotals& calcTotals);

  void horizontal(PricingTrx& trx, FareCalcCollector& fcCollector, CalcTotals& calcTotals);

  void horizontalProcessBaseAmount(CalcTotals& calcTotals, const DateTime& ticketingDate);

  void horizontalProcessEquivAmount(CalcTotals& calcTotals);

  void horizontalProcessTaxAmount(PricingTrx& trx, CalcTotals& calcTotals, int16_t& nbrTaxBoxes);

  void horizontalProcessTotalAmount(CalcTotals& calcTotals);

  void vertical(PricingTrx& trx, CalcTotals& calcTotals);

  void mix(PricingTrx& trx, CalcTotals& calcTotals);

  void horizontalProcessTaxBreakDown(PricingTrx& trx, CalcTotals& calcTotals);

  void buildTaxExempt(PricingTrx& trx);

  bool processGrandTotalLine(PricingTrx& trx, FareCalcCollector& fcCollector);

  void getTotalXT(PricingTrx& trx, bool& xtInd, int16_t& nbrTaxBoxes, CalcTotals& calcTotals);

  void getTotalFare(CalcTotals& calcTotals);

  static std::string
  getNetRemitFbc(PricingTrx& trx, const FareUsage* fareUsage, const TravelSeg* travelSeg);

  MoneyAmount& getEquivalentAmount(
      PricingTrx& trx,
      const MoneyAmount& inAmount,
      MoneyAmount& outAmount,
      const std::string& fromCurrency,
      const std::string& toCurrency,
      CurrencyConversionRequest::ApplicationType applType = CurrencyConversionRequest::OTHER);

  void getCurrencyNoDec(PricingTrx& trx, const std::string inCurrencyCode);

  bool getTaxOverride(PricingTrx& trx, int16_t& nbrTaxBoxes);

  void convertAmount2String(const MoneyAmount& inAmount,
                            const int16_t& inNbrDec,
                            std::string& outString,
                            const int16_t& outNbrDigit = 2,
                            const bool checkAmount = true);

  void displayPsgrFareBasisLine(PricingTrx& trx, CalcTotals& calcTotals);

  void displayBaseTaxHeader(PricingTrx& trx, const CalcTotals& calcTotals);

  void displayBaseTaxHeaderMultiVersion(PricingTrx& trx,
                                        const CalcTotals& calcTotals,
                                        bool activeNewVersion);

  void resetCounters();

  void verticalProcessBaseAmount(CalcTotals& calcTotals, const DateTime& ticketingDate);

  void verticalProcessEquivAmount(PricingTrx& trx);

  void verticalProcessTaxAmount(PricingTrx& trx, CalcTotals& calcTotals);

  void verticalProcessTotalAmount(PricingTrx& trx);

  void verticalProcessXTLine(PricingTrx& trx, CalcTotals& calcTotals);

  void verticalProcessRateLine(PricingTrx& trx, const CalcTotals& calcTotals);

  void displayEndorsements(PricingTrx& pricingTrx, const FarePath* farePath);

  virtual void displayWarnings(const CalcTotals& calcTotals);

  void displayWpaTrailerMessage();

  int16_t checkFareAmountLength(const int16_t& inAmountLen, const int16_t& maxAmountLen);

  bool checkExistFarePath(PricingTrx& trx, const CalcTotals& calcTotals);

  void diagTruePaxType(const CalcTotals& calcTotals);
  void displayItinInfo(const CalcTotals& calcTotals);
  void diagBrands(const CalcTotals& calcTotals);
  void diagJourney(const CalcTotals& calcTotals);

  bool verifyBookingClass(CalcTotals& ct);
  bool verifyBookingClass(std::vector<CalcTotals*>& cts);

  virtual void displayVerifyBookingClassMsg();

  void displayAxessCat27Lines(const FarePath* farePath);
  void displayAxessCat35Lines(PricingTrx& trx, const CalcTotals& calcTotals);
  void wpNettPrefix(FareCalcCollector& fcCollector);
  void displaySegmentFeeMessage();
  void displayNonIATARoundingTextMessage(CalcTotals& calcTotals);
  void displaySpanishLargeFamilyDiscountMessage(PricingTrx& trx, const Itin& itin);
  void displayCat05BookingDTOverrideMessage(const FarePath* fp);
  void displayBrandWarning();
  void displayBrandingLegsInfo();
  void displayNoBrandsOffered(PricingTrx& trx, CalcTotals& calcTotals);

  CalcTotals*
  selectCalcTotals(PricingTrx& trx,
                   CalcTotals* calcTotals,
                   bool netRemit = false);

  class CheckBookingClass
  {
  public:
    CheckBookingClass(bool& status) : _status(status) {}
    bool operator()(const FareUsage* fu);

  private:
    bool& _status;
  };

  class CheckRO
  {
  public:
    CheckRO(bool& status) : roStatus(status) {}
    CheckRO(const CheckRO& rhs) : roStatus(rhs.roStatus) {}
    void operator()(const FareUsage* fu);

  private:
    bool& roStatus;
  };

protected:
  std::string getFareBasisCode(const CalcTotals& ct, const FareUsage* fu) const;
  std::string getFareBasisCode(const CalcTotals& ct, const TravelSeg* ts) const;
  std::string getDifferential(const FareUsage* fu);

  void displayIndustryAndGoverningWarning(FcMessageCollection& collection);
  void displayBookingCodeWarning(FcMessageCollection& collection);
  void addToCalcTotals(CalcTotals* calcTotals, FcMessageCollection& collection);

  PricingTrx* _trx = nullptr;
  const FcConfig* _fcConfig = nullptr;
  FareCalcCollector* _fcCollector = nullptr;

  FareCalc::FcStream _fareCalcDisp;

  bool _needXTLine = false;
  bool _dispSegmentFeeMsg = false;

  //@TODO get rid of all unnecessary member variables
  std::string _fareCurrencyCode;

  std::vector<std::string> _warningFopMsgs; // unique FOP warning MSG's

  MoneyAmount _totalBaseAmount = 0;
  MoneyAmount _xtAmount = 0;
  MoneyAmount _totalEquivAmount = 0;
  MoneyAmount _totalFareAmount = 0;
  MoneyAmount _tempWorkAmount = 0;

  int16_t _nbrDec = 0; // no of decimal for the current currency
  int16_t _equivNoDec = 0; // for equivalent, tax and total amount
  int16_t _fareNoDec = 0; // base fare number of decimal
  int16_t _psgrCount = 0; // control multiple passenger types processing
  int16_t _fareAmountLen = 0; // length of fare, tax fields

  std::string _warning; // FCC Warning Message

  std::vector<int> _roOptions; // WPA options with RO

  bool _needNetRemitCalc = false;

  bool _allRequireRebook = true; // CWT WPA
  std::vector<int> _optionsToRebook; // WPA mix - option numbers for rebook

  bool _warningEtktCat15 = false; // E_tkt warning
};

class display_endorsements
{
public:
  display_endorsements(const PricingTrx& trx, FareCalculation& fareCalculation, bool forTicketing)
    : _trx(trx), _fareCalculation(fareCalculation), _forTicketing(forTicketing)
  {
  }

  void operator()(const TicketEndorseLine& endorseLine);

private:
  const PricingTrx& _trx;
  FareCalculation& _fareCalculation;
  bool _forTicketing;
};

class DisplayWarningsForFC
{
public:
  DisplayWarningsForFC(PricingTrx& trx,
                       FareCalc::FcStream& fareCalcDisp,
                       const FcConfig* fcConfig,
                       std::vector<std::string>& warningFopMsgs,
                       bool& warningEtktCat15,
                       const CalcTotals& calcTotals,
                       FareCalcCollector* fcCollector = nullptr);

  virtual ~DisplayWarningsForFC() = default;

  virtual void display();
  void display(const FareUsage* fareUsage);
  void display(const PricingUnit* pricingUnit);
  virtual void display(const FcMessage& message);

  void displayWarning(const std::string& message);
  virtual void displayFcMessages();

protected:
  PricingTrx& _trx;
  FareCalc::FcStream& _fareCalcDisp;
  const FcConfig* _fcConfig;
  std::vector<std::string>& _warningFopMsgs;
  bool& _warningEtktCat15;
  const CalcTotals& _calcTotals;
  FareCalcCollector* _fcCollector;
};
} // namespace tse
