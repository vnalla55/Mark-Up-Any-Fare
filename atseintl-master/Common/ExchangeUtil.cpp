//-------------------------------------------------------------------
//
//  File:        ExchangeUtil.cpp
//  Created:     September 16, 2007
//  Authors:     Simon Li
//
//  Description: Util functions for exchange or Rex transactions
//
//  Updates:
//
//  Copyright Sabre 2007
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

#include "Common/ExchangeUtil.h"

#include "Common/CurrencyConversionFacade.h"
#include "Common/FallbackUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/FarePath.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"

#include <algorithm>

namespace tse
{
FALLBACK_DECL(exchangeRefactorRaiiDate); //remove RaiiProcessingDate::_trxOld with this fallback

namespace ExchangeUtil
{

FCChangeStatus
getChangeStatus(const std::vector<TravelSeg*>& tvlSegs,
                const int16_t& pointOfChgSegmentOrder)
{
  std::vector<TravelSeg*>::const_iterator tvlSegI = tvlSegs.begin();
  const std::vector<TravelSeg*>::const_iterator tvlSegIEnd = tvlSegs.end();

  bool totalFlown = true;

  for (; tvlSegI != tvlSegIEnd; tvlSegI++)
  {
    if ((*tvlSegI)->unflown())
    {
      totalFlown = false;
      break;
    }
  }

  if (totalFlown)
    return tse::FL;

  bool totalUnchanged = true;

  for (; tvlSegI != tvlSegIEnd; tvlSegI++)
  {
    // if ((*tvlSegI)->changeStatus().isSet(statusSeenAsChged))
    if ((*tvlSegI)->changeStatus() == TravelSeg::CHANGED ||
        (*tvlSegI)->changeStatus() == TravelSeg::INVENTORYCHANGED)
    {
      totalUnchanged = false;
      break;
    }
  }

  if (!totalUnchanged)
    return tse::UC;

  if (tvlSegs.front()->pnrSegment() > pointOfChgSegmentOrder && pointOfChgSegmentOrder > 0)
    return tse::UN;
  else
    return tse::UU;
}

struct IsUnflown
{
  bool operator()(TravelSeg* seg) const { return seg->unflown(); }
};

struct IsChanged
{
  bool operator()(TravelSeg* seg) const
  {
    return seg->changeStatus() == TravelSeg::CHANGED ||
           seg->changeStatus() == TravelSeg::INVENTORYCHANGED;
  }
};

FCChangeStatus
getChangeStatus(const std::vector<TravelSeg*>& tvlSegs,
                const int16_t& pointOfChgFirst,
                const int16_t& pointOfChgSecond)
{
  std::vector<TravelSeg*>::const_iterator tvlSegI = tvlSegs.begin();
  const std::vector<TravelSeg*>::const_iterator tvlSegIEnd = tvlSegs.end();

  if ((tvlSegI = find_if(tvlSegI, tvlSegIEnd, IsUnflown())) == tvlSegIEnd)
    return tse::FL;

  if (std::find_if(tvlSegI, tvlSegIEnd, IsChanged()) != tvlSegIEnd)
    return tse::UC;

  int16_t pointOfChg = pointOfChgFirst;
  TravelSeg* lastSeg = tvlSegs.back();

  if (pointOfChgFirst == -1 && !lastSeg->newTravelUsedToSetChangeStatus().empty())
  {
    pointOfChg = pointOfChgSecond;
    lastSeg = tvlSegs.back()->newTravelUsedToSetChangeStatus().back();
  }

  if (lastSeg->pnrSegment() > pointOfChg && pointOfChg > 0)
    return tse::UN;
  else
    return tse::UU;
}

void
avoidValidationOfCategoriesInMinFares(RexBaseTrx& trx,
                                      std::vector<PaxTypeFare*>& faresForMinFares)
{
  for (PaxTypeFare* fare : faresForMinFares)
  {
    fare->setCategoryProcessed(RuleConst::MINIMUM_STAY_RULE, true);
    fare->setCategoryProcessed(RuleConst::MAXIMUM_STAY_RULE, true);
    fare->setCategoryProcessed(RuleConst::COMBINABILITY_RULE, true);
    fare->setCategoryProcessed(RuleConst::ACCOMPANIED_PSG_RULE, true);
    fare->setCategoryProcessed(RuleConst::TICKET_ENDORSMENT_RULE, true);
    fare->setCategoryProcessed(RuleConst::TOURS_RULE, true);
  }
}

Money
convertCurrency(const PricingTrx& trx,
                const Money& source,
                const CurrencyCode& targetCurr,
                bool rounding)
{
  if (source.code() == targetCurr)
    return source;
  Money target(targetCurr);
  CurrencyConversionFacade facade;
  facade.convert(target, source, const_cast<PricingTrx&>(trx), rounding);
  return target;
}

void
setRetrievalInfo(PricingTrx& trx, FareMarket::RetrievalInfo* info)
{
  SetFaresRetrievalInfo sfri(info);
  for (FareMarket* fm : trx.fareMarket())
  {
    fm->retrievalInfo() = info;
    sfri(fm);
  }
}

bool
validateFixedLegsInCEXS(const PricingTrx& trx, const std::vector<ShoppingTrx::Leg>& legs)
{
  if (legs.size() < 3)
    return true;

  auto isFlownLeg = [](const ShoppingTrx::Leg& leg)
                    {
                      return !leg.sop().front().itin()->travelSeg().front()->unflown();
                    };
  auto isNotShopped = [](const ShoppingTrx::Leg& leg)
                      {
                         return !leg.sop().front().itin()->travelSeg().front()->isShopped();
                      };

  auto checkLegStatus = [&](const ShoppingTrx::Leg& leg, bool isFixed)
                        {
                          return isFixed || isFlownLeg(leg) || isNotShopped(leg);
                        };

  const std::vector<bool>& fixedLegs = trx.getFixedLegs();

  bool isAtLeastOneFixedLeg = std::any_of(fixedLegs.begin(), fixedLegs.end(),
                                          [](bool isFixed) { return isFixed; });
  if(isAtLeastOneFixedLeg)
  {
    bool statusHasChanged = false;
    for (unsigned int i = 1; i < fixedLegs.size(); ++i)
    {
      if (checkLegStatus(legs[i], fixedLegs[i]) !=
          checkLegStatus(legs[i-1], fixedLegs[i-1]))
      {
        if (statusHasChanged && !isNotShopped(legs[i]))
        {
          return false;
        }
        statusHasChanged = true;
      }
    }
  }
  return true;
}


RaiiProcessingDate::RaiiProcessingDate(RexBaseTrx& trx, const FarePath& fp, bool isFallback)
                                       : _savedDate(trx.fareApplicationDT()),
                                         _prevROEvalue(trx.useSecondROEConversionDate()),
                                         _isFallback(isFallback), _isRexBaseTrx(true),
                                         _trxOld(&trx), _trx(trx)
{
  if(isFallback || skipSettingDate(trx.excTrxType()))
    return;

  bool containsCurrent = false;
  bool containsHistorical = false;

  for (const PricingUnit* pricingUnit : fp.pricingUnit())
  {
    for (const FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      if (fareUsage->paxTypeFare()->retrievalFlag() & FareMarket::RetrievCurrent)
        containsCurrent = true;
      else
        containsHistorical = true;
    }
  }

  if (containsCurrent && containsHistorical)
  {
    if (fp.isReissue())
    {
      trx.setFareApplicationDT(trx.originalTktIssueDT());
    }
    else
    {
      trx.setFareApplicationDT(trx.currentTicketingDT());
    }
  }
  else if(containsCurrent)
  {
    trx.setFareApplicationDT(trx.currentTicketingDT());
  }
  else
  {
    trx.setFareApplicationDT(trx.originalTktIssueDT());
  }

  setSecondROEConversionDate(fp);
}

RaiiProcessingDate::RaiiProcessingDate(RexBaseTrx& trx, const DateTime& date, bool isFallback)
                          : _savedDate(trx.fareApplicationDT()),
                            _prevROEvalue(trx.useSecondROEConversionDate()),
                            _isFallback(isFallback), _isRexBaseTrx(true),
                            _trxOld(&trx), _trx(trx)
{
  if (fallback::exchangeRefactorRaiiDate(&_trx))
    trx.setFareApplicationDT(date);
  else if (!_isFallback)
    trx.setFareApplicationDT(date);
}

RaiiProcessingDate::RaiiProcessingDate(PricingTrx& trx, bool isFallback)
  : _savedDate (trx.dataHandle().ticketDate())
  , _prevROEvalue (false)
  , _isFallback(isFallback)
  , _isRexBaseTrx(false)
  , _trxOld(nullptr)
  , _trx(trx)
{ }

RaiiProcessingDate::~RaiiProcessingDate()
{
  //sorry about this, but the new logic has to work even if fallback is active
  if(fallback::exchangeRefactorRaiiDate(&_trx) && !_isRexBaseTrx)
  {
    if (!_isFallback && !skipSettingDate(_trx.excTrxType()))
      _trx.dataHandle().setTicketDate(_savedDate);
  }

  if (!fallback::exchangeRefactorRaiiDate(&_trx))
  {
    if (!_isFallback && !skipSettingDate(_trx.excTrxType()))
    {
      if (_isRexBaseTrx)
      {
        RexBaseTrx& rexbaseTrx = static_cast<RexBaseTrx&>(_trx);
        rexbaseTrx.setFareApplicationDT(_savedDate);
        rexbaseTrx.useSecondROEConversionDate() = _prevROEvalue;
      }
      else
      {
        _trx.dataHandle().setTicketDate(_savedDate);
      }
    }
  }
  else
  {
    if (!_isFallback && !skipSettingDate(_trxOld->excTrxType()) && _isRexBaseTrx)
    {
      _trxOld->setFareApplicationDT(_savedDate);
      _trxOld->useSecondROEConversionDate() = _prevROEvalue;
    }
  }
}

void
RaiiProcessingDate::useOriginalTktIssueDT()
{
  if (_isFallback)
    return;

  const BaseExchangeTrx* exchangeTrx = dynamic_cast<const BaseExchangeTrx*>(&_trx);
  if (exchangeTrx)
  {
    DateTime exchangedate = exchangeTrx->originalTktIssueDT();
    if (exchangedate.isValid() && exchangedate != _savedDate)
    {
      _trx.dataHandle().setTicketDate(exchangedate);
      _restoreOriginalDate = true;
    }
  }
}

void
RaiiProcessingDate::setSecondROEConversionDate(const FarePath& fp)
{
  if (!fallback::exchangeRefactorRaiiDate(&_trx))
  {
    if (RexPricingTrx::isRexTrxAndNewItin(_trx))
    {
      RexBaseTrx& rexbaseTrx = static_cast<RexBaseTrx&>(_trx);
      if(rexbaseTrx.applyReissueExchange())
      {
        if (fp.useSecondRoeDate() && !rexbaseTrx.newItinSecondROEConversionDate().isEmptyDate())
          rexbaseTrx.useSecondROEConversionDate() = true;
      }
    }
  }
  else
  {
    if (RexPricingTrx::isRexTrxAndNewItin(*_trxOld) &&
      static_cast<RexBaseTrx&>(*_trxOld).applyReissueExchange())
    {
      if (fp.useSecondRoeDate() && !_trxOld->newItinSecondROEConversionDate().isEmptyDate())
      {
        _trxOld->useSecondROEConversionDate() = true;
      }
    }
  }
}

bool
RaiiProcessingDate::skipSettingDate(const PricingTrx::ExcTrxType& excType)
{
  if (_restoreOriginalDate)
    return false;

  return excType == PricingTrx::NEW_WITHIN_ME ||
         excType == PricingTrx::EXC1_WITHIN_ME ||
         excType == PricingTrx::EXC2_WITHIN_ME ||
         excType == PricingTrx::AF_EXC_TRX;
}
} //ExchangeUtil

MoneyAmount
NonRefundableUtil::excTotalNucAmt() const
{
  if (!boost::indeterminate(_trx.excTktNonRefundable()))
    return excXmlNucAmt();

  return _trx.exchangeItin().front()->farePath().front()->getTotalNUCAmount();
}

MoneyAmount
NonRefundableUtil::excTotalBaseCurrAmt() const
{
  if (!boost::indeterminate(_trx.excTktNonRefundable())) // what with overriden curr override?
    return _trx.totalFareCalcAmount();

  const FarePath& excFarePath = *_trx.exchangeItin().front()->farePath().front();
  return excFarePath.convertToBaseCurrency(_trx, excFarePath.getTotalNUCAmount(), NUC);
}

MoneyAmount
NonRefundableUtil::excNonRefundableNucAmt() const
{
  if (!boost::indeterminate(_trx.excTktNonRefundable()))
    return _trx.excTktNonRefundable() ? excXmlNucAmt() : 0.0;

  return _trx.exchangeItin().front()->getNonRefAmountInNUC().value();
}

MoneyAmount
NonRefundableUtil::excNonRefundableBaseCurrAmt() const
{
  if (!boost::indeterminate(_trx.excTktNonRefundable()))
    return _trx.excTktNonRefundable() ? _trx.totalFareCalcAmount() : 0.0;

  const FarePath& excFarePath = *_trx.exchangeItin().front()->farePath().front();

  const Money& nonRefAmt = _trx.exchangeItin().front()->getNonRefAmount();
  return excFarePath.convertToBaseCurrency(_trx, nonRefAmt.value(), nonRefAmt.code());
}

void
NonRefundableUtil::calculateNonRefundableAmount(FarePath& newFarePath)
{
  MoneyAmount amt = 0.0;
  CurrencyCode srcCurr =
      _trx.exchangeItin().front()->farePath().front()->itin()->originationCurrency();

  if (boost::indeterminate(_trx.excTktNonRefundable()))
  {
    amt = _trx.exchangeItin().front()->getNonRefAmountInNUC().value();
    srcCurr = NUC;
  }
  else if (_trx.excTktNonRefundable())
    amt = _trx.totalFareCalcAmount();
  // else if( ! _trx.excTktNonRefundable()) already handled above

  if (amt > EPSILON && srcCurr != newFarePath.baseFareCurrency())
  {
    MoneyAmount baseAmount = 0.0;
    const Money sourceMoney(amt, srcCurr);
    Money targetMoney(baseAmount, newFarePath.baseFareCurrency());
    CurrencyConversionFacade ccf;
    ccf.convert(targetMoney, sourceMoney, _trx, true);
    amt = targetMoney.value();
  }

  newFarePath.setHigherNonrefundableAmountInBaseCurr(amt);
}

MoneyAmount
NonRefundableUtil::excXmlNucAmt() const
{
  const RexPricingOptions& ro = static_cast<const RexPricingOptions&>(*_trx.getOptions());
  if (ro.excBaseFareCurrency().empty())
    return excNucConvert(
        _trx.totalFareCalcAmount(),
        _trx.exchangeItin().front()->farePath().front()->itin()->originationCurrency());

  return excNucConvert(_trx.totalFareCalcAmount(), ro.excBaseFareCurrency());
}

MoneyAmount
NonRefundableUtil::excNucConvert(const MoneyAmount& srcAmt, const CurrencyCode& srcCurr) const
{
  if (srcCurr == NUC)
    return srcAmt;

  NUCCurrencyConverter converter;
  Money source(srcAmt, srcCurr);
  Money target(NUC);
  CurrencyConversionRequest request(
      target, source, _trx.originalTktIssueDT(), *_trx.getRequest(), _trx.dataHandle());

  request.useInternationalRounding() = true;
  return converter.convert(request, nullptr) ? target.value() : 0.0;
}

} // tse
