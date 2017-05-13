//-------------------------------------------------------------------
//
//  Copyright Sabre 2015
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
//
// subclass to add simple smarts to DB object
//
#pragma once
#include "Common/CurrencyConversionCache.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/LocUtil.h"
#include "Common/Money.h"
#include "Rules/RuleConst.h"

#include <memory>

namespace tse
{
class CurrencyConversionFacade;
class FareRetailerCalcDetailInfo;
class PaxTypeFare;
class PricingTrx;

class CalcMoney : public Money
{
  friend class AdjustedFareCalcTest;

  public:
    PricingTrx& _trx;
    Money _itinMoney;
    std::unique_ptr<CurrencyConversionFacade> _ccf;
    bool _isIntl;
    bool _isRT = false;
    CurrencyConversionCache _cache;
    bool _excCurrOverrideNotNuc;
    bool _applyNonIATARounding = false;

  public:

    static const MoneyAmount NO_AMT;

    enum WhichCurrency
    {
      CURRENCY_FAILED = 0,
      CURRENCY_1 = 1,
      CURRENCY_2 = 2
    };

    CalcMoney(PricingTrx& trx, Itin& itin);
    CalcMoney& operator=(const CalcMoney& rhs);

    void setCurrency(const CurrencyCode& newCur);
    void setRT(bool newRT);
    MoneyAmount fareAmount();
    void setFareAmount(MoneyAmount fareAmt);
    void getFromPTF(PaxTypeFare& paxTypeFare, bool doNotChkNonIATARounding = false);

    void doPercent(const MoneyAmount percent);
    int doAdd(const MoneyAmount amt1,
              const CurrencyCode cur1,
              const MoneyAmount amt2,
              const CurrencyCode cur2);
    int doMinus(const MoneyAmount amt1,
                const CurrencyCode cur1,
                const MoneyAmount amt2,
                const CurrencyCode cur2);
    int getFromSpec(const MoneyAmount amt1,
                    const CurrencyCode cur1,
                    const MoneyAmount amt2,
                    const CurrencyCode cur2);

    bool isIntl() { return _isIntl; }
    MoneyAmount& nucValue() { return _itinMoney.value(); }

  private:
    void calcFareAmount();
    int pickAmt(Money& native,
                Money& nuc,
                const MoneyAmount amt1,
                const CurrencyCode cur1,
                const MoneyAmount amt2,
                const CurrencyCode cur2);
};

class AdjPrice
{
  friend class AdjustedFareCalcTest;

public:
  AdjPrice(Indicator ind = RuleConst::BLANK,
           Percent per = 0,
           Percent per2 = 0,
           MoneyAmount amt1 = 0,
           CurrencyCode cur1 = EMPTY_STRING(),
           MoneyAmount amt2 = 0,
           CurrencyCode cur2 = EMPTY_STRING(),
           int noDecPer1 = 0,
           int noDecPer2 = 0,
           int noDecAmt1 = 0,
           int noDecAmt2 = 0)
    : _ind(ind),
      _percent(per),
      _percent2(per2),
      _price1(amt1, cur1),
      _price2(amt2, cur2),
      _amount1(amt1),
      _amount2(amt2),
      _noDecPercent1(noDecPer1),
      _noDecPercent2(noDecPer2),
      _noDecAmt1(noDecAmt1),
      _noDecAmt2(noDecAmt2),
      _usedSide2(false) {};

  MoneyAmount calcPercent(const MoneyAmount& amt);

  bool doPrice(CalcMoney& calcMoney, CurrencyCode* curSide = nullptr);

  bool isPercentPrice() const;
  bool isPriceInd() const;
  bool isRangeInd() const;

  Indicator _ind; // how to calc price: S,C,A,M or R,P,N,T
  Percent _percent;
  Percent _percent2;
  Money _price1;
  Money _price2;
  MoneyAmount _amount1;
  MoneyAmount _amount2;
  int _noDecPercent1;
  int _noDecPercent2;
  int _noDecAmt1;
  int _noDecAmt2;
  bool _usedSide2;
};


class AdjustedFareCalc
{
//  friend class AdjustedFareCalc;

public:
  AdjustedFareCalc(PricingTrx& trx, Itin& itin) : _calcMoney(trx, itin) { };

  void load(const FareRetailerCalcDetailInfo&);

  void getPrice(const PricingTrx& trx,
                PaxTypeFare& basePTF,
                MoneyAmount& newFareAmt,
                MoneyAmount& newNucAmt);
  CurrencyCode& getCurrencyCode(){ return _curSide; };

  void getSellingInfo(Percent& percent, MoneyAmount& amount, int& noDecAmt, int& noDecPercent);

protected:
  AdjPrice _selling;
  CurrencyCode _curSide;
  CalcMoney _calcMoney;
};
}
