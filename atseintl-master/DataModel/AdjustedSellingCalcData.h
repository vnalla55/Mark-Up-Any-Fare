#pragma once

#include "Common/TsePrimitiveTypes.h"

namespace tse
{

class FareRetailerRuleInfo;

class AdjustedSellingCalcData
{
public:
  void setCalcInd(const Indicator calcInd) { _calcInd = calcInd; }
  const Indicator getCalcInd() const { return _calcInd; }

  void setPercent(const Percent& percent) { _percent = percent; }
  const Percent& getPercent() const { return _percent; }

  void setRuleAmount(const MoneyAmount ruleAmt) { _ruleAmt = ruleAmt; }
  const MoneyAmount& getRuleAmount() const { return _ruleAmt; }

  void setCalculatedAmt(const MoneyAmount calculatedAmt) { _calculatedAmt = calculatedAmt; }
  const MoneyAmount& getCalculatedAmt() const { return _calculatedAmt; }

  void setCalculatedNucAmt(const MoneyAmount calculatedNucAmt)
  { _calculatedNucAmt = calculatedNucAmt; }
  const MoneyAmount& getCalculatedNucAmt() const { return _calculatedNucAmt; }

  void setNoDecPercent(const int noDecPercent) { _noDecPercent = noDecPercent; }
  const int getNoDecPercent() const { return _noDecPercent; }

  void setNoDecAmt(const int noDecAmt) { _noDecAmt = noDecAmt; }
  const int getNoDecAmt() const { return _noDecAmt; }

  void setMarkupAdjAmt(const MoneyAmount& markupAdjAmt) { _markupAdjustedAmt = markupAdjAmt; }
  const MoneyAmount& getMarkupAdjAmt() const { return _markupAdjustedAmt; }

  void setFareRetailerRuleInfo(const FareRetailerRuleInfo* frriASL) { _frriASL = frriASL; }
  const FareRetailerRuleInfo* getFareRetailerRuleInfo() const { return _frriASL; }

  void setFareRetailerRuleId(const uint64_t& frrIdASL) { _frrIdASL = frrIdASL; }
  const uint64_t& getFareRetailerRuleId() const { return _frrIdASL; }

  void setFareRetailerRuleSeqNo(const uint64_t& frrSeqNoASL) { _frrSeqNoASL = frrSeqNoASL; }
  const uint64_t& getFareRetailerRuleSeqNo() const { return _frrSeqNoASL; }

  void setSourcePcc(const PseudoCityCode& pcc) { _frrSourcePccASL = pcc; }
  const PseudoCityCode& getSourcePcc() const { return _frrSourcePccASL; }

  void setCalculatedASLCurrency(const CurrencyCode& cur) { _calculatedASLCurrency = cur; }
  const CurrencyCode& getCalculatedASLCurrency() const { return _calculatedASLCurrency; }

private:
  Indicator _calcInd = ' ';    // 'S'pec, 'C'alc, 'A'dd, 'M'inus,.. etc..
  Percent _percent = 0.0;
  MoneyAmount _ruleAmt = 0.0;        // S/A/M amount
  MoneyAmount _calculatedAmt = 0.0;     // in fare currency
  MoneyAmount _calculatedNucAmt = 0.0;  // in NUC currency
  MoneyAmount _markupAdjustedAmt = 0.0; // Adjusted ASL Amount
  int _noDecPercent = 0;
  int _noDecAmt = 0;
  const FareRetailerRuleInfo* _frriASL = nullptr;
  uint64_t _frrIdASL = 0;
  uint64_t _frrSeqNoASL = 0;
  PseudoCityCode _frrSourcePccASL = "";
  CurrencyCode _calculatedASLCurrency = ""; // selected Currency for any method to calculate ASL
};

} // tse namespace
