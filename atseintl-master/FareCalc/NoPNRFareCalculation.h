#pragma once

#include "DataModel/NoPNRPricingTrx.h"
#include "DBAccess/FareTypeMatrix.h"
#include "DBAccess/NoPNRFareTypeGroup.h"
#include "FareCalc/FareCalculation.h"
#include "FareCalc/FcStream.h"
#include "FareCalc/NoPNRFareCalcCollector.h"

namespace tse
{

class NoPNRPricingTrx;
class FareCalcConfig;
class NoPNRFareCalcCollector;
class FareCalcCollector;
class PaxType;
class NoPNROptions;

class NoPNRFareCalculation : public FareCalculation
{
public:
  void initialize(PricingTrx* trx,
                  const FareCalcConfig* fcConfig,
                  FareCalcCollector* fcCollector) override;

  virtual void process() override;
  bool checkDetailFormat();
  void displayPsgDetailFormat(CalcTotals& calcTotals);
  virtual void displayPsgrInfo(const CalcTotals& calcTotals, bool detailFormat = true) override;

  class CalcTotalsCompare
  {
  public:
    CalcTotalsCompare(bool ascending) : lowToHigh(ascending) {}
    bool less(const CalcTotals* c1, const CalcTotals* c2) const;
    bool operator()(const CalcTotals* c1, const CalcTotals* c2) const
    {
      if (lowToHigh)
        return less(c1, c2);
      return !less(c1, c2);
    }

  private:
    bool lowToHigh;
  };

  static void displayIndicesVector(const std::string& prefix,
                                   const std::vector<int>& indicesToDisplay,
                                   const std::string& postfix,
                                   bool canBreakPostfix,
                                   std::ostream& output,
                                   bool appendFinalNewline = true,
                                   bool alwaysDisplayTwoDigits = true);

  void displayGIMessage();

protected:
  virtual void processDetailedFormat(const FareCalcCollector::CalcTotalsMap& cm);
  virtual void processPrimaryFormat();
  virtual void displayNoPNRFareLineHdr();
  virtual void displayNoPNRFareLineHdrMultiVersion(bool activeNewVersion);

  void displayFareLineInfo(CalcTotals& calcTotals) override;
  void displayTotalFareAmount(CalcTotals& calcTotals) override;
  void displayVerifyBookingClassMsg() override;
  virtual void displayWqTrailerMessage();
  virtual void displayTruePaxType(const CalcTotals& calcTotals);
  static bool isNomatch(const CalcTotals& calcTotals);
  virtual void displayNonCOCCurrencyIndicator(CalcTotals& calcTotals);

  static void displayRangeOrCommaSeparated(std::ostringstream& indicesOut,
                                           const std::vector<int>& indices,
                                           std::vector<int>::const_iterator start,
                                           std::vector<int>::const_iterator end,
                                           bool& first_token,
                                           std::ostream& output,
                                           bool alwaysDisplayTwoDigits);

  static void addToOutput(std::ostringstream& currentLine,
                          const std::string& txt,
                          bool canBreakBetweenLines,
                          std::ostream& output);

  virtual void processPrimaryFormatSinglePaxType(PaxType* pt, int& psgCount, bool firstPaxType);

  virtual void
  fillCalcTotalsList(const PaxType* paxType, std::vector<CalcTotals*>& calcTotalsList) const;
  virtual void checkForROmessages(CalcTotals& calcTotals);
  virtual void displayFinalTrailerMessage();
  bool noFaresForPsg(const std::vector<CalcTotals*>& cts);
  bool noFaresForNoPaxType();
  void displayWarnings(const CalcTotals& calcTotals) override;
  std::string getApplicableBookingCodesLine(CalcTotals& calcTotals);
  virtual void
  collectBookingCodes(std::vector<std::string>& appplicableBookingClasses, CalcTotals& calcTotals);
  std::string getPSSBookingCodesLine(CalcTotals& calcTotals);

  void displayCxrBkgCodeFareBasis(PricingTrx& trx,
                                  CalcTotals& calcTotals,
                                  AirSeg* airSeg,
                                  std::string& fareBasisCode) override;

  void applyTksDsgLen(char tktDesLength, std::string& fareBasis);
  void displayItinTravelSegmentDepartureDate(AirSeg* airSeg);

  virtual void displayTaxAmount(CalcTotals& calcTotals, unsigned int taxFieldWidth = 8) override;

private:
  std::vector<CalcTotals*>& getCalcTotals(const PaxType* paxType);
  void displayPaxTypeWarnings(const std::vector<CalcTotals*>& cts, const uint16_t& startIndex);

  void displayPaxTypeWarningsAccTvl(const std::vector<CalcTotals*>& cts);

  void createWpnResponse();
  void displayNoMatchResponse();
  void displayRoMessage(const std::vector<int>& roOptions);
  void displayRebookMessage(const std::vector<int>& optionsToRebook);
  void displayFareTypeMessage(const std::vector<CalcTotals*>& calcTotalsList);

  // All 4 methods below are designed to build the total grand line for the
  // WPxx entry (after WPA). They are for the 1S customers only at this time.
  void processGrandTotalLineWPnn(PricingTrx& trx, CalcTotals& calcTotals);

  MoneyAmount
  getBaseFareTotal(CalcTotals& totals, CurrencyCode& currencyCodeUsed, CurrencyNoDec& noDecUsed);

  MoneyAmount getEquivFareAmountTotal(CalcTotals& totals,
                                      CurrencyCode& currencyCodeUsed,
                                      CurrencyNoDec& noDecUsed);

  MoneyAmount
  getTaxTotal(CalcTotals& totals, CurrencyCode& currencyCodeUsed, CurrencyNoDec& noDecUsed);

  void determineOriginationCurrency(CalcTotals& ct, CurrencyCode& originationCurrency);

private:
  bool _primaryResp = false;
  bool _secondaryResp = false;
  CalcTotals* _cat35CalcTotals = nullptr;
  const NoPNROptions* _noPNRConfigOptions = nullptr;
  std::vector<CalcTotals*> _primaryCalcTotals;
  static const unsigned int WINDOW_WIDTH = 64;
  std::map<const PaxType*, std::vector<CalcTotals*>> calcTotalsObtained;
  std::vector<std::string> firstBookingCodes;
};

class DisplayWarningsForNoPnrFC : public DisplayWarningsForFC
{
public:
  DisplayWarningsForNoPnrFC(PricingTrx& trx,
                            FareCalc::FcStream& fareCalcDisp,
                            const FcConfig* fcConfig,
                            std::vector<std::string>& warningFopMsgs,
                            bool& warningEtktCat15,
                            const CalcTotals& calcTotals,
                            const NoPNROptions* options,
                            NoPNRFareCalculation& noPNRFareCalculation);

  virtual void display() override;
  virtual void display(const FcMessage& message) override;
  virtual void displayFcMessages() override;

protected:
  const NoPNROptions* _noPnrOptions;
  NoPNRFareCalculation& _noPNRFareCalculation;
};
} // namespace tse
