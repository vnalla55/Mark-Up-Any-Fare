//----------------------------------------------------------------------------
//  File:        Diag865Collector.h
//  Authors:
//  Created:
//
//  Description: Diagnostic 865 formatter for the Cat35 commissions.
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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class PricingTrx;
class FarePath;
class Commissions;
class CollectedNegFareData;
class NegFareRest;
class NegPaxTypeFareRuleData;

class Diag865Collector : public DiagCollector
{
  friend class Diag865CollectorTest;

public:
  enum CommissionCalcMethod
  {
    NONE,
    AMOUNT,
    NET_TIMES_COMM_PCT,
    SELL_TIMES_COMM_PCT,
    MARKUP_PLUS_SELL_TIMES_COMM_PCT,
    MARKUP_PLUS_COMM_AMT,
    MARKUP
  };

  //@TODO will be removed, once the transition is done
  explicit Diag865Collector(Diagnostic& root)
    : DiagCollector(root), _totalNetAmtCalCurr(0), _totalSellAmtCalCurr(0)
  {
  }
  Diag865Collector() : _totalNetAmtCalCurr(0), _totalSellAmtCalCurr(0) {}

  MoneyAmount& totalNetAmtCalCurr() { return _totalNetAmtCalCurr; }
  MoneyAmount totalNetAmtCalCurr() const { return _totalNetAmtCalCurr; }
  MoneyAmount& totalSellAmtCalCurr() { return _totalSellAmtCalCurr; }
  MoneyAmount totalSellAmtCalCurr() const { return _totalSellAmtCalCurr; }

  void displayRequestData(PricingTrx& trx,
                          const FarePath& farePath,
                          const Commissions& comm,
                          const CollectedNegFareData& collectedNegFareData);

  void displayFareComponentHeader();
  void displayFarePathHeader(const FarePath& farePath);
  void displayCommissionData(const Commissions& comm,
                             const FareUsage* fareUsage,
                             const FarePath& farePath,
                             const NegFareRest* negFareRest,
                             const NegPaxTypeFareRuleData* negPaxTypeFare);
  void displayCommissionData(Commissions& comm, const FarePath& farePath);
  void displayCommissionApplication(const Commissions& comm,
                                    MoneyAmount amount = 0.0,
                                    CommissionCalcMethod method = NONE,
                                    bool netFareCommission = false);

  void displayFinalCommission(const Commissions& comm);
  void displayFailInfo();

private:
  void displayPaxTypeLine(const PaxTypeCode& paxTypeCode,
                          const std::string& fareBox,
                          const Indicator& displayType);
  void displayPaxTypeLine(const PaxTypeCode& paxTypeCode);
  void displayCommissionData(MoneyAmount sellingAmt,
                             MoneyAmount netAmt,
                             MoneyAmount milageSurcharge,
                             MoneyAmount milageNetSurcharge,
                             MoneyAmount cat8Charge,
                             MoneyAmount cat9Charge,
                             MoneyAmount cat12Charge,
                             MoneyAmount totalSellAmt,
                             MoneyAmount totalSellAmtInPayCurr,
                             MoneyAmount totalNetAmt,
                             MoneyAmount totalNetAmtInPayCurr,
                             const CurrencyCode& calcCurr,
                             const CurrencyCode& paymentCurr);

  std::string getFareBox(const NegFareRest* negFareRest) const;
  std::string formatAmount(MoneyAmount amount, const CurrencyCode& currencyCode = NUC) const;
  std::string formatPercent(Percent percent, bool zeroNA = false) const;
  std::string formatFlag(bool value) const;

  MoneyAmount _totalNetAmtCalCurr;
  MoneyAmount _totalSellAmtCalCurr;
};

} // namespace tse

