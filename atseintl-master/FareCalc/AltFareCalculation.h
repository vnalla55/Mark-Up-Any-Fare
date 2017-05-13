#pragma once

#include "FareCalc/FareCalculation.h"

namespace tse
{

class PaxType;

class AltFareCalculation : public FareCalculation
{
public:
  void process() override;

  bool checkDetailFormat(); // check if detail format is required

  void displayPsgDetailFormat(CalcTotals& calcTotals, bool displayObFees);

  void displayPsgrInfo(const CalcTotals& calcTotals, bool detailFormat = true) override;

  void checkDiagNeeds(bool& diagAccTvlOut, bool& diagAccTvlIn, std::vector<uint16_t>& optIndexes);

  void displayWarnings(const CalcTotals& calcTotals) override;

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

private:
  void
  getCalcTotals(const PaxType* paxType, std::vector<CalcTotals*>& cts, bool sortList = true) const;

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

  void displayObFeesForSingleMatch(const CalcTotals& calcTotals);
private:
  bool _primaryResp = false;
  bool _secondaryResp = false;
  CalcTotals* _cat35CalcTotals = nullptr;
};
} // namespace tse
