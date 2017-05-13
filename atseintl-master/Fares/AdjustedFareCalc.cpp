//-------------------------------------------------------------------
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
#include "Fares/AdjustedFareCalc.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyUtil.h"
#include "Common/Money.h"
#include "Common/TrxUtil.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Loc.h"
#include "Fares/NegotiatedFareController.h"
#include "Rules/RuleUtil.h"
#include "DBAccess/FareRetailerCalcDetailInfo.h"
#include "DBAccess/FareRetailerRuleInfo.h"

using namespace std;

namespace tse
{

const MoneyAmount CalcMoney::NO_AMT = -1.0;

CalcMoney::CalcMoney(PricingTrx& trx, Itin& itin)
  : Money(NUC),
    _trx(trx),
    _itinMoney(itin.calculationCurrency()),
    _ccf(new CurrencyConversionFacade()),
    _isIntl(itin.useInternationalRounding()),
    _cache(trx.dataHandle())
{
  if ((trx.excTrxType() == PricingTrx::PORT_EXC_TRX || trx.excTrxType() == PricingTrx::AR_EXC_TRX)
       && !itin.calcCurrencyOverride().empty()
       && itin.calcCurrencyOverride() != NUC
       && itin.calcCurrencyOverride() != USD
       && trx.billing()
       && trx.billing()->partitionID() == "WN"
     )
  {
    _excCurrOverrideNotNuc = true;
  }
  else
    _excCurrOverrideNotNuc = false;
}

CalcMoney& CalcMoney::operator=(const CalcMoney& rhs)
{
  if (&rhs == this)
    return *this;

  _amount = rhs.value();
  _currencyCode = rhs.code();
  _isNuc = rhs._isNuc;
  _itinMoney.value() = rhs._itinMoney.value();
  _isRT = rhs._isRT;

  return *this;
}

void
CalcMoney::setCurrency(const CurrencyCode& newCur)
{
  setCode(newCur);
  value() = NO_AMT;
}

void
CalcMoney::setRT(bool newRT)
{
  _isRT = newRT;
}

MoneyAmount
CalcMoney::fareAmount()
{
  calcFareAmount();
  return value();
}

void
CalcMoney::setFareAmount(MoneyAmount fareAmt)
{
  value() = fareAmt;
  _itinMoney.value() = NO_AMT;
}

void
CalcMoney::getFromPTF(PaxTypeFare& paxTypeFare, bool doNotChkNonIATARounding)
{
  if (!doNotChkNonIATARounding)
    _applyNonIATARounding = paxTypeFare.applyNonIATARounding(_trx);

  setRT(paxTypeFare.isRoundTrip());
  setCurrency(paxTypeFare.currency()); // Fare native currency

  if (_isRT)
  {
    _itinMoney.value() = paxTypeFare.nucOriginalFareAmount();
    value() = paxTypeFare.originalFareAmount();
  }
  else
  {
    _itinMoney.value() =
        paxTypeFare.nucFareAmount(); // FareAmount in converted currency, maynot NUC.
    value() = paxTypeFare.fareAmount(); // FareAmount in local currency
  }
}

void
CalcMoney::calcFareAmount()
{
  // don't overwite if calc could find native currency
  if (this->value() <= NO_AMT)
  {
    _ccf->convertCalc(*this,
                     _itinMoney,
                     _trx,
                     _isIntl,
                     CurrencyConversionRequest::OTHER,
                     false,
                     nullptr,
                     &_cache,
                     _trx.getTrxType() != PricingTrx::FAREDISPLAY_TRX);
  }
}

int
CalcMoney::pickAmt(Money& native,
                   Money& nuc,
                   const MoneyAmount amt1,
                   const CurrencyCode cur1,
                   const MoneyAmount amt2,
                   const CurrencyCode cur2)
{
  int retCurrency = CURRENCY_FAILED;

  if (cur1 == native.code())
  {
    native.value() = amt1;
    _ccf->convertCalc(
        nuc, native, _trx, _isIntl, CurrencyConversionRequest::OTHER, false, nullptr, &_cache);
    retCurrency = CURRENCY_1;
  }
  else if (cur2 == native.code())
  {
    native.value() = amt2;
    _ccf->convertCalc(
        nuc, native, _trx, _isIntl, CurrencyConversionRequest::OTHER, false, nullptr, &_cache);
    retCurrency = CURRENCY_2;
  }
  else if (amt1 == 0.0)
  {
    native.value() = 0.0;
    nuc.value() = 0.0;
    retCurrency = CURRENCY_1;
  }
  else
  {
    Money m1(amt1, cur1);
    _ccf->convertCalc(nuc, m1, _trx, _isIntl, CurrencyConversionRequest::OTHER, false, nullptr, &_cache);
    retCurrency = CURRENCY_1;
    if (!cur2.empty())
    {
      Money m2(amt2, cur2);
      Money nuc2(nuc.code());
      _ccf->convertCalc(
          nuc2, m2, _trx, _isIntl, CurrencyConversionRequest::OTHER, false, nullptr, &_cache);
      if (nuc2.value() < nuc.value())
      {
        nuc.value() = nuc2.value();
        retCurrency = CURRENCY_2;
      }
    }

    _ccf->convertCalc(
        native, nuc, _trx, _isIntl, CurrencyConversionRequest::OTHER, false, nullptr, &_cache);

  }

  return retCurrency;
}

void
CalcMoney::doPercent(const MoneyAmount percent)
{
  if (_applyNonIATARounding)
  {
    _itinMoney.setApplyNonIATARounding();
    this->setApplyNonIATARounding();
  }

  value() *= (percent / 100.0);
  _ccf->round(*this, _trx, _isIntl);
  CurrencyUtil::truncateNonNUCAmount(value(), this->noDec());

  if (_trx.excTrxType()==PricingTrx::AR_EXC_TRX && _itinMoney.code()==NUC && !_isIntl)
  {
    _ccf->convertCalc(
        _itinMoney, *this, _trx, _isIntl, CurrencyConversionRequest::OTHER, false, 0, &_cache);
  }
  else
  {
    _itinMoney.value() *= (percent / 100.0);
    if (_excCurrOverrideNotNuc)
      CurrencyUtil::truncateNonNUCAmount(_itinMoney.value(), this->noDec());
    else
      _ccf->round(_itinMoney, _trx, _isIntl);
  }
}

int
CalcMoney::doAdd(const MoneyAmount amt1,
                 const CurrencyCode cur1,
                 const MoneyAmount amt2,
                 const CurrencyCode cur2)
{
  Money temp(this->code());
  Money tempNuc(_itinMoney.code());

  int ret = pickAmt(temp, tempNuc, amt1, cur1, amt2, cur2);
  if (
      ( (ret==CURRENCY_1) && (cur1==temp.code()) && (cur1==_itinMoney.code()) ) ||
      ( (ret==CURRENCY_2) && (cur2==temp.code()) && (cur2==_itinMoney.code()) )
     )
  {
    _itinMoney.value() += temp.value();
    this->value() += temp.value(); // Add with amount in Native currency
  }
  else if (ret != CURRENCY_FAILED)
  {
    _itinMoney.value() += tempNuc.value();
    this->value() += temp.value(); // Add with amount in Native currency
  }

  return ret;
}

int
CalcMoney::doMinus(const MoneyAmount amt1,
                   const CurrencyCode cur1,
                   const MoneyAmount amt2,
                   const CurrencyCode cur2)
{
  Money temp(this->code());
  Money tempNuc(_itinMoney.code());

  int ret = pickAmt(temp, tempNuc, amt1, cur1, amt2, cur2);

  if (
       ( (ret==CURRENCY_1) && (cur1==temp.code()) && (cur1==_itinMoney.code()) ) ||
       ( (ret==CURRENCY_2) && (cur2==temp.code()) && (cur2==_itinMoney.code()) )
     )
  {
    _itinMoney.value() -= temp.value();
    this->value() -= temp.value(); // Minus with amount in Native currency
  }
  else if (ret != CURRENCY_FAILED)
  {
    _itinMoney.value() -= tempNuc.value();
    this->value() -= temp.value(); // Minus with amount in Native currency
  }

  return ret;
}

int
CalcMoney::getFromSpec(const MoneyAmount amt1,
                       const CurrencyCode cur1,
                       const MoneyAmount amt2,
                       const CurrencyCode cur2)
{
  return pickAmt(*this, _itinMoney, amt1, cur1, amt2, cur2);
}

bool
AdjPrice::doPrice(CalcMoney& calcMoney, CurrencyCode* curSide)
{
  char tempSide = 1;

  if (_ind == RuleConst::NF_CALC_PERCENT)     // 'C'
  {
    calcMoney.doPercent(_percent);
  }
  else if (_ind == RuleConst::NF_SPECIFIED)   // 'S'
  {
    tempSide = calcMoney.getFromSpec(
        _price1.value(), _price1.code(), _price2.value(), _price2.code());
  }
  else if (_ind == RuleConst::NF_ADD)         // 'A'
  {
    calcMoney.doPercent(_percent);
    tempSide =
        calcMoney.doAdd(_price1.value(), _price1.code(), _price2.value(), _price2.code());
  }
  else if (_ind == RuleConst::NF_MINUS)       // 'M'
  {
    calcMoney.doPercent(_percent);
    tempSide =
        calcMoney.doMinus(_price1.value(), _price1.code(), _price2.value(), _price2.code());
  }
  else
    return false;

  if (curSide)
    *curSide = (tempSide == 1) ? _price1.code() : _price2.code();

  _usedSide2 = (tempSide == 2);
  return (tempSide != 0);
}

bool
AdjPrice::isPriceInd() const
{
  return (_ind == RuleConst::NF_CALC_PERCENT || _ind == RuleConst::NF_SPECIFIED ||
          _ind == RuleConst::NF_ADD || _ind == RuleConst::NF_MINUS);
}

bool
AdjPrice::isPercentPrice() const
{
  return (_ind == RuleConst::NF_CALC_PERCENT || _ind == RuleConst::NF_ADD ||
          _ind == RuleConst::NF_MINUS);
}

bool
AdjPrice::isRangeInd() const
{
  return (_ind == RuleConst::NF_RANGE_PERCENT || _ind == RuleConst::NF_RANGE_SPECIFIED ||
          _ind == RuleConst::NF_ADD_RANGE || _ind == RuleConst::NF_MINUS_RANGE);
}

void
AdjustedFareCalc::getPrice(const PricingTrx& trx,
                           PaxTypeFare& basePTF,
                           MoneyAmount& newFareAmt,
                           MoneyAmount& newNucAmt)
{
  newFareAmt = NegotiatedFareController::INVALID_AMT;
  newNucAmt = NegotiatedFareController::INVALID_AMT;

  _curSide = basePTF.currency();
  if (!_selling.isPriceInd())
    return;

  _calcMoney.getFromPTF(basePTF);

  if (!_selling.doPrice(_calcMoney, &_curSide))
    return;


  if (basePTF.isRoundTrip() && !trx.getOptions()->isRtw())
  {
    newFareAmt = _calcMoney.fareAmount() / 2.0;
    CurrencyUtil::halveNUCAmount(_calcMoney.nucValue());
  }
  else
  {
    newFareAmt = _calcMoney.fareAmount();
  }

  newNucAmt = _calcMoney.nucValue();
}

void
AdjustedFareCalc::getSellingInfo(Percent& percent, MoneyAmount& amount, int& noDecAmt, int& noDecPercent)
{
  if (_selling._usedSide2)
  {
    noDecAmt = _selling._noDecAmt2;
    noDecPercent = _selling._noDecPercent2;
    percent = _selling._percent2;
    amount = _selling._amount2;
  }
  else
  {
    noDecAmt = _selling._noDecAmt1;
    noDecPercent = _selling._noDecPercent1;
    percent = _selling._percent;
    amount = _selling._amount1;
  }
}

void
AdjustedFareCalc::load(const FareRetailerCalcDetailInfo& frcdi)
{
  _selling = AdjPrice(frcdi.fareCalcInd(),
                      frcdi.percent1(),
                      frcdi.percent2(),
                      frcdi.amount1(),
                      frcdi.amountCurrency1(),
                      frcdi.amount2(),
                      frcdi.amountCurrency2(),
                      frcdi.percentNoDec1(),
                      frcdi.percentNoDec2(),
                      frcdi.amountNoDec1(),
                      frcdi.amountNoDec2());
}

}
