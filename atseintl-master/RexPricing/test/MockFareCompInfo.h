#ifndef MOCK_FARE_COMP_INFO_H
#define MOCK_FARE_COMP_INFO_H

#include "DataModel/FareCompInfo.h"

namespace tse
{

class MockFareCompInfo : public FareCompInfo
{
public:
  MockFareCompInfo(const MoneyAmount& amt = 0.0, int pctg = 0, bool discounted = false)
    : _rad(2008, 01, 20)
  {
    _tktFareCalcFareAmt = amt;
    _discounted = discounted;
    _mileageSurchargePctg = pctg;
    _fareMarket = &fm1;
    _secondaryFareMarket = &fm2;
    _hip = &hipItem;
  }
  void swapFareMarkets() { std::swap(_fareMarket, _secondaryFareMarket); }

protected:
  const DateTime& getRuleApplicationDate(const RexBaseTrx& trx, const CarrierCode& govCarrier) const
  {
    return _rad;
  }

  FareMarket fm1, fm2;
  MinFarePlusUpItem hipItem;
  DateTime _rad;
};
}

#endif
