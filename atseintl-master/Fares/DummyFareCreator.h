#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

namespace tse
{
class FareMarket;
class Itin;
class PaxTypeFare;
class PricingTrx;
class PenaltyInfo;
class VoluntaryChangesInfoW;
class VoluntaryRefundsInfo;
class DataHandle;

namespace DummyFareCreator
{
struct FareSettings
{
  PaxTypeCode _pax;
  CurrencyCode _currency;
  Indicator _owrt;
  bool _convertToCalcCurrency = false;
  bool _privateTariff = false;
};

PaxTypeFare* createPtf(PricingTrx& trx,
                       const Itin& itin, FareMarket& fm,
                       const FareSettings& settings);

void createFaresForExchange(PricingTrx& trx, const Itin& itin, FareMarket& fm);
void createFaresForOriginBasedRT(PricingTrx& trx, const Itin& itin, FareMarket& fm);

const PenaltyInfo* createDummyPenaltyInfo(DataHandle& dh);
const VoluntaryChangesInfoW* createDummyVoluntaryChangesInfo(DataHandle& dh);
const VoluntaryRefundsInfo* createDummyVoluntaryRefundsInfo(DataHandle& dh);
}
}
