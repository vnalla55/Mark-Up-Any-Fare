//-------------------------------------------------------------------
//  Description: Negotiated Fare Calculation
//      subclass of DB object for Table 979/Markup Control/
//      Markup Calculate to add simple validation/access
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
#include "Fares/NegFareCalc.h"

#include "Common/CurrencyUtil.h"
#include "Common/Money.h"
#include "Common/TrxUtil.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/Loc.h"
#include "Fares/FareController.h"
#include "Fares/DiscountedFareController.h"
#include "Fares/NegotiatedFareController.h"
#include "Rules/RuleUtil.h"
#include "DBAccess/FareRetailerCalcDetailInfo.h"
#include "DBAccess/FareRetailerRuleInfo.h"

using namespace std;

namespace tse
{
// not valid for ranges
bool
NegPrice::doPrice(FareController& ctrl, CurrencyCode* curSide)
{
  char tempSide = 1;

  if (_ind == RuleConst::NF_CALC_PERCENT)
  {
    ctrl.calcMoney().doPercent(_percent);
  }
  else if (_ind == RuleConst::NF_SPECIFIED)
  {
    tempSide = ctrl.calcMoney().getFromSpec(
        _price1.value(), _price1.code(), _price2.value(), _price2.code());
  }
  else if (_ind == RuleConst::NF_ADD)
  {
    ctrl.calcMoney().doPercent(_percent);
    tempSide =
        ctrl.calcMoney().doAdd(_price1.value(), _price1.code(), _price2.value(), _price2.code());
  }
  else if (_ind == RuleConst::NF_MINUS)
  {
    ctrl.calcMoney().doPercent(_percent);
    tempSide =
        ctrl.calcMoney().doMinus(_price1.value(), _price1.code(), _price2.value(), _price2.code());
  }
  else
    return false;

  if (curSide)
    *curSide = (tempSide == 1) ? _price1.code() : _price2.code();

  _usedSide2 = (tempSide == 2);
  return (tempSide != 0);
}

MoneyAmount
NegPrice::getPrice(const Money& base, FareController& ctrl, bool isIntl, CurrencyCode* curSide)
{
  char tempSide;
  if (_ind == RuleConst::NF_CALC_PERCENT || _ind == RuleConst::NF_RANGE_PERCENT)
    return calcPercent(base.value());

  Money temp(base.code());
  tempSide = ctrl.matchCurrency(temp, isIntl, _price1, _price2);

  if (curSide)
    *curSide = (tempSide == 1) ? _price1.code() : _price2.code();

  if (tempSide == 0)
    return 0;

  if (_ind == RuleConst::NF_SPECIFIED || _ind == RuleConst::NF_RANGE_SPECIFIED)
    return temp.value();
  else if (_ind == RuleConst::NF_ADD || _ind == RuleConst::NF_ADD_RANGE)
    return calcPercent(base.value()) + temp.value();
  else if (_ind == RuleConst::NF_MINUS || _ind == RuleConst::NF_MINUS_RANGE)
    return calcPercent(base.value()) - temp.value();

  return 0;
}

bool
NegPrice::isPriceInd() const
{
  return (_ind == RuleConst::NF_CALC_PERCENT || _ind == RuleConst::NF_SPECIFIED ||
          _ind == RuleConst::NF_ADD || _ind == RuleConst::NF_MINUS);
}
bool
NegPrice::isPercentPrice() const
{
  return (_ind == RuleConst::NF_CALC_PERCENT || _ind == RuleConst::NF_ADD ||
          _ind == RuleConst::NF_MINUS);
}

bool
NegPrice::isRangeInd() const
{
  return (_ind == RuleConst::NF_RANGE_PERCENT || _ind == RuleConst::NF_RANGE_SPECIFIED ||
          _ind == RuleConst::NF_ADD_RANGE || _ind == RuleConst::NF_MINUS_RANGE);
}

bool
NegRange::isInRange(NegPrice& selling,
                    const Money& base,
                    const MoneyAmount price,
                    FareController& ctrl,
                    bool isIntl,
                    bool isRT)
{
  if (!isRangeInd())
    return true; // no range check needed

  // only compare percenteges if that's all that was used in calc
  // avoids currency rounding from CalcMoney
  if (selling._ind == RuleConst::NF_CALC_PERCENT && _ind == RuleConst::NF_RANGE_PERCENT)
  {
    if (selling._percent < _percent)
      return false;
    if (_max._percent <= 0)
      return true;
    if (selling._percent > _max._percent)
      return false;
    return true;
  }
  if (selling._ind == RuleConst::NF_ADD && _ind == RuleConst::NF_ADD_RANGE &&
      selling._percent == _percent && selling._price1.code() == _price1.code() &&
      selling._price1.code() == _max._price1.code())
  {
    if (selling._price1.value() < _price1.value())
      return false;
    if (_max._percent > 0)
    {
      if (selling._percent == _max._percent && selling._price1.code() == _max._price1.code() &&
          selling._price1.value() > _max._price1.value())
        return false;
    }
    else
    {
      return true;
    }
  }

  MoneyAmount min = this->getPrice(base, ctrl, isIntl);
  MoneyAmount max = _max.getPrice(base, ctrl, isIntl);

  if (max == 0)
    max = price;

  if (price >= min && price <= max)
    return true;

  // don't reject small rounding errors
  MoneyAmount priceTemp = price;
  ctrl.roundCurrency(priceTemp, base.code(), isIntl);
  // too high?
  if (price > max)
  {
    ctrl.roundCurrency(max, base.code(), isIntl);
    return (priceTemp <= max);
  }
  // too low?
  if (price < min)
  {
    ctrl.roundCurrency(min, base.code(), isIntl);
    return (priceTemp >= min);
  }
  return false;
}

//-------------------------------------------------------------------
// <PRE>
//
// @function  NegotiatedFareController::isMatchFareLoc
//
// Description:  This method validates LOC1/LOC2 Geo specs and
//               zone tables in Table 979 and LOC1/LOC2 Geo specs in
//               Markup Calculate Table 980.
//
// @param  PaxTypeFare      Base fare
//
// @return true - if location fields matched
//
// </PRE>
//-------------------------------------------------------------------
bool
NegLoc::isMatchFareLoc(PricingTrx& trx, const PaxTypeFare& ptFare)
{
  bool matchedSwapped = false;
  if (RuleUtil::matchLocation(trx,
                              _loc1,
                              _userDefZone1,
                              _loc2,
                              _userDefZone2,
                              _vendor,
                              *ptFare.fareMarket(),
                              matchedSwapped,
                              ptFare.carrier()))
  {
    if (_directionality != X_TYPE)
    {
      return true;
    }
    else
    {
      if (((ptFare.origin() == ptFare.fareMarket()->origin()->loc()) ||
           (ptFare.origin() == ptFare.fareMarket()->boardMultiCity())) &&
          (!matchedSwapped))
      {
        return true;
      }
      else if (((ptFare.origin() == ptFare.fareMarket()->destination()->loc()) ||
                (ptFare.origin() == ptFare.fareMarket()->offMultiCity())) &&
               (matchedSwapped))
      {
        return true;
      }
    }
  }
  return false;
}

bool
NegFareCalc::isValidCat(Indicator catType, bool canUpdate)
{
  if (!isValidCat(catType))
    return false;

  // If data is in specified amount, the Update byte in Table983 must not be 'Y'
  if (_selling.isPriceInd() && canUpdate)
  {
    return false;
  }

  // If data is in range, the Update byte in Table983 must be 'Y'
  if (!_selling.isPriceInd() && !canUpdate)
  {
    return false;
  }

  return true;
}

bool
NegFareCalc::isValidCat(Indicator catType)
{
  // If Net/Selling Ind is 'S', DIS CAT TYPE must be 'T' or 'C'
  if (_netSellingInd == S_TYPE && catType == RuleConst::SELLING_FARE)
  {
    return false;
  }

  // If Net/Selling Ind is 'N', DIS CAT TYPE must be 'L'
  if (_netSellingInd == 'N' && catType != RuleConst::SELLING_FARE)
  {
    return false;
  }

  return true;
}

void
NegFareCalc::load(NegFareCalcInfo* x)
{
  _vendor = x->vendor();
  _directionality = x->directionality();
  _viewNetInd = RuleConst::BLANK;

  _loc1 = x->loc1();
  _loc2 = x->loc2();
  _userDefZone1 = x->userDefZone1();
  _userDefZone2 = x->userDefZone2();
  _netSellingInd = x->netSellingInd();
  _selling = NegPrice(x->fareInd(),
                      x->sellingPercent(),
                      x->sellingFareAmt1(),
                      x->sellingCur1(),
                      x->sellingFareAmt2(),
                      x->sellingCur2(),
                      x->sellingPercentNoDec(),
                      x->sellingNoDec1(),
                      x->sellingNoDec2());

  _range = NegRange(x->fareInd(),
                    x->calcPercentMin(),
                    x->calcPercentMax(),
                    x->calcMinFareAmt1(),
                    x->calcMaxFareAmt1(),
                    x->calcCur1(),
                    x->calcMinFareAmt2(),
                    x->calcMaxFareAmt2(),
                    x->calcCur2());
}

void
NegFareCalc::load(VendorCode vendor,
                  MarkupCalculate* x,
                  Indicator viewNetInd,
                  PseudoCityCode creatorPCC)
{
  _vendor = vendor;
  _directionality = x->directionality();
  _viewNetInd = viewNetInd;
  _creatorPCC = creatorPCC;

  _loc1.loc() = x->loc1().loc();
  _loc2.loc() = x->loc2().loc();
  _loc1.locType() = x->loc1().locType();
  _loc2.locType() = x->loc2().locType();

  _userDefZone1 = RuleConst::NOT_APPLICABLE_ZONE;
  _userDefZone2 = RuleConst::NOT_APPLICABLE_ZONE;
  _netSellingInd = x->netSellingInd();

  _selling = NegPrice(x->sellingFareInd(),
                      x->sellingPercent(),
                      x->sellingFareAmt1(),
                      x->sellingCur1(),
                      x->sellingFareAmt2(),
                      x->sellingCur2(),
                      x->sellingPercentNoDec(),
                      x->sellingNoDec1(),
                      x->sellingNoDec2());

  _markup = NegRange(x->markupFareInd(),
                     x->percentMin(),
                     x->percentMax(),
                     x->markupMinAmt1(),
                     x->markupMaxAmt1(),
                     x->markupCur1(),
                     x->markupMinAmt2(),
                     x->markupMaxAmt2(),
                     x->markupCur2());

  _wholesale = NegPrice(x->wholesalerFareInd(),
                        x->wholesalerPercent(),
                        x->wholesalerFareAmt1(),
                        x->wholesalerCur1(),
                        x->wholesalerFareAmt2(),
                        x->wholesalerCur2());
}

void
NegFareCalc::getPrice(const PricingTrx& trx,
                      PaxTypeFare& basePTF,
                      FareController& ctrl,
                      MoneyAmount& newFareAmt,
                      MoneyAmount& newNucAmt)
{
  newFareAmt = NegotiatedFareController::INVALID_AMT;
  newNucAmt = NegotiatedFareController::INVALID_AMT;

  _curSide = basePTF.currency();
  if (!_selling.isPriceInd() && !_selling.isRangeInd())
    return;

  ctrl.calcMoney().getFromPTF(basePTF);
  Money baseMoney(basePTF.originalFareAmount(), basePTF.currency());

  if (_wholesale.isPriceInd())
  {
    if (!_wholesale.doPrice(ctrl))
      return;
    if (!_selling.doPrice(ctrl, &_curSide))
      return;
  }
  else
  {
    if (!_selling.doPrice(ctrl, &_curSide))
      return;

    if (!_markup.isInRange(_selling,
                           Money(baseMoney),
                           ctrl.calcMoney().fareAmount(),
                           ctrl,
                           ctrl.calcMoney().isIntl(),
                           basePTF.isRoundTrip()))
    {
      return;
    }
  }

  if (!_range.isInRange(_selling,
                        Money(baseMoney),
                        ctrl.calcMoney().fareAmount(),
                        ctrl,
                        ctrl.calcMoney().isIntl(),
                        basePTF.isRoundTrip()))
  {
    return;
  }

  if (basePTF.isRoundTrip() && !trx.getOptions()->isRtw())
  {
    newFareAmt = ctrl.calcMoney().fareAmount() / 2.0;
    CurrencyUtil::halveNUCAmount(ctrl.calcMoney().nucValue());
  }
  else
  {
    newFareAmt = ctrl.calcMoney().fareAmount();
  }
  newNucAmt = ctrl.calcMoney().nucValue();
}

void
NegFareCalc::getSellingInfo(
    Indicator& ind, Money& amt, int& noDecAmt, Percent& percent, int& noDecPercent)
{
  ind = _selling._ind;
  percent = _selling._percent;
  noDecPercent = _selling._noDecPercent;
  if (_selling._usedSide2)
  {
    amt = _selling._price2;
    noDecAmt = _selling._noDecAmt2;
  }
  else
  {
    amt = _selling._price1;
    noDecAmt = _selling._noDecAmt1;
  }
}

bool
NegFareCalc::getWholesalerFareAmt(const Money& base,
                                  const Money& nucBase,
                                  MoneyAmount& newFareAmt,
                                  MoneyAmount& newFareNucAmt,
                                  FareController& ctrl,
                                  const bool isIntl,
                                  const bool isRT)
{
  _curSide = base.code();

  // check to see if wholesaleFareIndicator is X
  if (!_wholesale.isPriceInd())
    return false;
  else
  {
    newFareAmt = _wholesale.getPrice(base, ctrl, isIntl);
    newFareNucAmt = _wholesale.getPrice(nucBase, ctrl, isIntl);
  }

  if (_wholesale.isPercentPrice())
  {
    ctrl.roundCurrency(newFareAmt, base.code(), isIntl);
    CurrencyUtil::truncateNUCAmount(newFareNucAmt);
  }

  if (isRT)
  {
    newFareAmt = newFareAmt / 2;
    CurrencyUtil::halveNUCAmount(newFareNucAmt);
  }

  return true;
}

void
NegFareCalc::makeDummyCalcInfo(const Money& base)
{
  _selling._ind = RuleConst::NF_NO_CALC_DATA;
  _viewNetInd = RuleConst::BLANK;
  _curSide = base.code();
}

//-------------------------------------------------------------------
// <PRE>
//
// @function  NegotiatedFareController::isMatchMarkupCalc
//
// Description:  This method validates fare class, fare type,
//               season type, dow type and passenger type fields in
//               the Markup Calculate table.
//
// @param  MarkupCalculate  Markup Calculate 980 object
// @param  PaxTypeFare      Base fare
//
// @return true - if all fields matched
//
// </PRE>
//-------------------------------------------------------------------
bool
NegFareCalc::isMatchMarkupCalc(MarkupCalculate* x, const PaxTypeFare& ptFare)
{
  if (RuleUtil::matchFareClass(x->fareClass().c_str(), ptFare.fareClass().c_str()) &&
      RuleUtil::matchFareType(x->fareType().c_str(), ptFare.fcaFareType()) &&
      RuleUtil::matchSeasons(x->seasonType(), ptFare.fcaSeasonType()) &&
      RuleUtil::matchDayOfWeek(x->dowType(), ptFare.fcaDowType()) &&
      (x->psgType().empty() || x->psgType() == ADULT || x->psgType() == ptFare.fcasPaxType()))

  {
    return true;
  }
  return false;
}

bool
NegFareCalc::isT979FareIndInRange()
{
  return _range.isRangeInd();
}

void
NegFareCalc::loadFR(const FareRetailerCalcDetailInfo& frcdi,
                    const FareRetailerRuleInfo& frri,
                    Indicator viewNetInd,
                    Indicator netSellingInd,
                    PseudoCityCode creatorPCC)
{
  _vendor = frri.vendor();
  _directionality = frri.directionality();
  _creatorPCC = creatorPCC;

  _loc1.loc() = frri.loc1().loc();
  _loc2.loc() = frri.loc2().loc();
  _loc1.locType() = frri.loc1().locType();
  _loc2.locType() = frri.loc2().locType();

  _userDefZone1 = RuleConst::NOT_APPLICABLE_ZONE;
  _userDefZone2 = RuleConst::NOT_APPLICABLE_ZONE;
  _netSellingInd = netSellingInd;
  _viewNetInd = viewNetInd;

  _selling = NegPrice(frcdi.fareCalcInd(),
                      frcdi.percent1(),
                      frcdi.amount1(),
                      frcdi.amountCurrency1(),
                      frcdi.amount2(),
                      frcdi.amountCurrency2(),
                      frcdi.percentNoDec1(),
                      frcdi.amountNoDec1(),
                      frcdi.amountNoDec2());

  _markup = NegRange(frcdi.fareCalcInd(),
                     frcdi.percentMin1(),
                     frcdi.percentMax1(),
                     frcdi.amountMin1(),
                     frcdi.amountMax1(),
                     frcdi.amountCurrency1(),
                     frcdi.amountMin2(),
                     frcdi.amountMax2(),
                     frcdi.amountCurrency2());
}

void
NegFareCalc::loadFRWholeSale(const FareRetailerCalcDetailInfo& frcdi)
{
  _wholesale = NegPrice(frcdi.fareCalcInd(),
                        frcdi.percent1(),
                        frcdi.amount1(),
                        frcdi.amountCurrency1(),
                        frcdi.amount2(),
                        frcdi.amountCurrency2());
}

void
NegFareCalc::clearWholeSale()
{
  _wholesale._ind = RuleConst::BLANK;
  _wholesale._percent = 0;
  _wholesale._price1.setCode(EMPTY_STRING());
  _wholesale._price1.value() = 0;
  _wholesale._price2.setCode(EMPTY_STRING());
  _wholesale._price2.value() = 0;
  _wholesale._noDecPercent = 0;
  _wholesale._noDecAmt1 = 0;
  _wholesale._noDecAmt2 = 0;
  _wholesale._usedSide2 = false;
}

void
NegFareCalc::getPriceFRdiscounted(const PricingTrx& trx,
                                  PaxTypeFare& basePTF,
                                  FareController& ctrl,
                                  MoneyAmount& newFareAmt,
                                  MoneyAmount& newNucAmt,
                                  bool isRT,
                                  const DiscountInfo& discountInfo)
{
  ctrl.calcMoney().getFromPTF(basePTF);

  /*-----------------------------------------------------
   * Record1Resolver::CalcMoney.getFromPTF() sets
   * Record1Resolver::CalcMoney._itinMoney to basePTF's
   * nuc amount, and Record1Resolver::CalcMoney.value() to
   * basePTF's fareAmount. Here we override it with the
   * values from the parameters: newFareAmt and newNucAmt
   *-----------------------------------------------------*/
  ctrl.calcMoney().nucValue() = (isRT)? newNucAmt*2 : newNucAmt;
  ctrl.calcMoney().value() = (isRT)? newFareAmt*2 : newFareAmt;

  // Do the price now
  Indicator ind=discountInfo.farecalcInd();
  if (ind == DiscountedFareController::CALCULATED)
  {
    ctrl.calcMoney().doPercent(discountInfo.discPercent());
  }
  else if (ind == DiscountedFareController::SPECIFIED)
  {
    ctrl.calcMoney().getFromSpec(discountInfo.fareAmt1(),
                                 discountInfo.cur1(),
                                 discountInfo.fareAmt2(),
                                 discountInfo.cur2());
  }
  else if (ind == DiscountedFareController::ADD_CALCULATED_TO_SPECIFIED)
  {
    ctrl.calcMoney().doPercent(discountInfo.discPercent());
    ctrl.calcMoney().doAdd(discountInfo.fareAmt1(),
                           discountInfo.cur1(),
                           discountInfo.fareAmt2(),
                           discountInfo.cur2());
  }
  else if (ind == DiscountedFareController::SUBTRACT_SPECIFIED_FROM_CALCULATED)
  {
    ctrl.calcMoney().doPercent(discountInfo.discPercent());
    ctrl.calcMoney().doMinus(discountInfo.fareAmt1(),
                             discountInfo.cur1(),
                             discountInfo.fareAmt2(),
                             discountInfo.cur2());
  }

  newFareAmt = (isRT)?(ctrl.calcMoney().fareAmount()/2):ctrl.calcMoney().fareAmount();
  newNucAmt = (isRT)?(ctrl.calcMoney().nucValue()/2):ctrl.calcMoney().nucValue();
}

}
