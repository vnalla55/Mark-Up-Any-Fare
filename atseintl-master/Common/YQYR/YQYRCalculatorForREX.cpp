// ----------------------------------------------------------------
//
//   Copyright Sabre 2015
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#include "Common/YQYR/YQYRCalculatorForREX.h"

#include "Common/DateTime.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexBaseTrx.h"
#include "Taxes/Common/ReissueExchangeDateSetter.h"

namespace tse
{
YQYRCalculatorForREX::YQYRCalculatorForREX(PricingTrx& trx,
                                           Itin& it,
                                           const FareMarketPath* fmp,
                                           const PaxType* paxType)
  : YQYRCalculator(trx, it)
{
  const DateTime& historicalDate = rexBaseTrx().previousExchangeDT().isEmptyDate()
                                       ? rexBaseTrx().originalTktIssueDT()
                                       : rexBaseTrx().previousExchangeDT();

  bool hasSecondROE(!rexBaseTrx().newItinSecondROEConversionDate().isEmptyDate());

  _calculators[CalcKey(historicalDate, false)] =
      &_trx.dataHandle().safe_create<YQYRCalculator>(trx, it, fmp, paxType);

  _calculators[CalcKey(rexBaseTrx().currentTicketingDT(), false)] =
      &_trx.dataHandle().safe_create<YQYRCalculator>(trx, it, fmp, paxType);

  if (hasSecondROE)
  {
    _calculators[CalcKey(historicalDate, true)] =
        &_trx.dataHandle().safe_create<YQYRCalculator>(trx, it, fmp, paxType);

    _calculators[CalcKey(rexBaseTrx().currentTicketingDT(), true)] =
        &_trx.dataHandle().safe_create<YQYRCalculator>(trx, it, fmp, paxType);
  }
}

MoneyAmount
YQYRCalculatorForREX::chargeFarePath(const FarePath& fp, const CarrierCode valCxr) const
{
  ReissueExchangeDateSetter dateSetter(_trx, fp);
  return getDesignatedCalculator(fp).chargeFarePath(fp, valCxr);
}

void
YQYRCalculatorForREX::process()
{
  struct RestoreOnExit
  {
    RestoreOnExit(RexBaseTrx& trx)
      : _trx(trx),
        _savedROEInd(trx.useSecondROEConversionDate()),
        _savedDate(trx.ticketingDate()) {};
    ~RestoreOnExit()
    {
      _trx.setFareApplicationDT(_savedDate);
      _trx.useSecondROEConversionDate() = _savedROEInd;
    }
    RexBaseTrx& _trx;
    const bool _savedROEInd;
    const DateTime _savedDate;
  } r(rexBaseTrx());

  _lowerBound = std::numeric_limits<MoneyAmount>::max();
  for (const auto& date_ROE : _calculators)
  {
    rexBaseTrx().useSecondROEConversionDate() = date_ROE.first.second;
    rexBaseTrx().setFareApplicationDT(date_ROE.first.first);
    date_ROE.second->process();
    _lowerBound = std::min(_lowerBound, date_ROE.second->lowerBound());
  }
}

YQYRCalculator&
YQYRCalculatorForREX::getDesignatedCalculator(const FarePath& fp) const
{
  CalcMap::const_iterator it =
      _calculators.find(CalcKey(_trx.ticketingDate(), fp.useSecondRoeDate()));

  if (it == _calculators.end())
    throw std::logic_error("How come we don't have data under this key?");

  return *it->second;
}

void
YQYRCalculatorForREX::findMatchingPaths(const FarePath* fp,
                                        const CarrierCode valCxr,
                                        std::vector<YQYRApplication>& yqyrApplications)
{
  ReissueExchangeDateSetter dateSetter(_trx, *fp);
  getDesignatedCalculator(*fp).findMatchingPaths(fp, valCxr, yqyrApplications);
}

} // tse namespace
