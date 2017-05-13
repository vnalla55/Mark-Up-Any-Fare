//-------------------------------------------------------------------
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
//-------------------------------------------------------------------
//
// subclass to add simple smarts to DB object
//
#pragma once
#include "Common/LocUtil.h"
#include "Common/Money.h"
#include "DBAccess/MarkupControl.h"
#include "DBAccess/NegFareCalcInfo.h"
#include "Rules/RuleConst.h"

namespace tse
{
class FareController;
class FareRetailerCalcDetailInfo;
class FareRetailerRuleInfo;
class PaxTypeFare;
class PricingTrx;

class NegPrice
{
  friend class NegFareCalcTest;

public:
  NegPrice(Indicator ind = RuleConst::BLANK,
           Percent per = 0,
           MoneyAmount amt1 = 0,
           CurrencyCode cur1 = EMPTY_STRING(),
           MoneyAmount amt2 = 0,
           CurrencyCode cur2 = EMPTY_STRING(),
           int decPer = 0,
           int decAmt1 = 0,
           int decAmt2 = 0)
    : _ind(ind),
      _percent(per),
      _price1(amt1, cur1),
      _price2(amt2, cur2),
      _noDecPercent(decPer),
      _noDecAmt1(decAmt1),
      _noDecAmt2(decAmt2)
  {
  }

  MoneyAmount calcPercent(const MoneyAmount& amt) { return amt * (_percent / 100.0); }

  MoneyAmount
  getPrice(const Money& base, FareController& ctrl, bool isIntl, CurrencyCode* curSide = nullptr);
  bool doPrice(FareController& ctrl, CurrencyCode* curSide = nullptr);

  bool isPercentPrice() const;
  bool isPriceInd() const;
  bool isRangeInd() const;

  Indicator _ind; // how to calc price: S,C,A,M or R,P,N,T
  Percent _percent;
  Money _price1;
  Money _price2;
  int _noDecPercent;
  int _noDecAmt1;
  int _noDecAmt2;
  bool _usedSide2 = false;
};

// base is min, member is max
class NegRange : public NegPrice
{
  friend class NegFareCalcTest;

public:
  NegRange() : NegPrice(), _max() {};

  NegRange(Indicator ind,
           Percent minPer,
           Percent maxPer,
           MoneyAmount minAmt1,
           MoneyAmount maxAmt1,
           CurrencyCode cur1,
           MoneyAmount minAmt2,
           MoneyAmount maxAmt2,
           CurrencyCode cur2)
    : NegPrice(ind, minPer, minAmt1, cur1, minAmt2, cur2),
      _max(ind, maxPer, maxAmt1, cur1, maxAmt2, cur2) {};

  // see if price is in the range determined by base
  bool isInRange(NegPrice& selling,
                 const Money& base,
                 const MoneyAmount price,
                 FareController& ctrl,
                 bool isIntl,
                 bool isRT = false);

  NegPrice _max;
};

class NegLoc
{
public:
  NegLoc(VendorCode vendor = EMPTY_STRING(),
         Indicator directionality = RuleConst::BLANK,
         LocKey loc1 = LocKey(),
         LocKey loc2 = LocKey(),
         Zone userDefZone1 = EMPTY_STRING(),
         Zone userDefZone2 = EMPTY_STRING(),
         Indicator netSellingInd = RuleConst::BLANK)
    : _vendor(vendor),
      _directionality(directionality),
      _loc1(loc1),
      _loc2(loc2),
      _userDefZone1(userDefZone1),
      _userDefZone2(userDefZone2),
      _netSellingInd(netSellingInd) {};

  bool isMatchFareLoc(PricingTrx& trx, const PaxTypeFare& ptFare);

protected:
  VendorCode _vendor;
  Indicator _directionality;
  LocKey _loc1;
  LocKey _loc2;
  Zone _userDefZone1;
  Zone _userDefZone2;
  Indicator _netSellingInd;
};

class NegFareCalc : public NegLoc
{
  friend class NegFareCalcTest;

public:
  NegFareCalc() : NegLoc(), _viewNetInd(RuleConst::BLANK), _curSide(NUC) {};

  void load(NegFareCalcInfo* x);
  void load(VendorCode vendor, MarkupCalculate* x, Indicator viewNetInd, PseudoCityCode creatorPCC);
  void loadFR(const FareRetailerCalcDetailInfo&,
              const FareRetailerRuleInfo&,
              Indicator,
              Indicator,
              PseudoCityCode);
  void loadFRWholeSale(const FareRetailerCalcDetailInfo&);
  void clearWholeSale();

  bool isValidCat(Indicator catType, bool canUpdate);
  bool isValidCat(Indicator catType);

  void getPrice(const PricingTrx& trx,
                PaxTypeFare& basePTF,
                FareController& ctrl,
                MoneyAmount& newFareAmt,
                MoneyAmount& newNucAmt);

  void
  getPriceFRdiscounted(const PricingTrx& trx,
                       PaxTypeFare& basePTF,
                       FareController& ctrl,
                       MoneyAmount& newFareAmt,
                       MoneyAmount& newNucAmt,
                       bool isRT,
                       const DiscountInfo& discountInfo);

  void
  getSellingInfo(Indicator& ind, Money& amt, int& noDecAmt, Percent& percent, int& noDecPercent);

  bool getWholesalerFareAmt(const Money& base,
                            const Money& nucBase,
                            MoneyAmount& newFareAmt,
                            MoneyAmount& newFareNucAmt,
                            FareController& ctrl,
                            const bool isIntl,
                            const bool isRT);

  void makeDummyCalcInfo(const Money& base);

  const Indicator& viewNetInd() const { return _viewNetInd; }
  Indicator& viewNetInd() { return _viewNetInd; }
  const CurrencyCode& curSide() const { return _curSide; }
  CurrencyCode& curSide() { return _curSide; }
  const PseudoCityCode& creatorPCC() const { return _creatorPCC; }
  PseudoCityCode& creatorPCC() { return _creatorPCC; }

  bool isMatchMarkupCalc(MarkupCalculate* x, const PaxTypeFare& ptFare);
  bool isT979FareIndInRange();

protected:
  Indicator _viewNetInd;
  CurrencyCode _curSide;
  PseudoCityCode _creatorPCC; // who made MU/REDIST data

  NegPrice _selling;
  NegRange _range;
  NegRange _markup;
  NegPrice _wholesale;
};
}
