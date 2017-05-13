//----------------------------------------------------------------------------
//  Copyright Sabre 2016
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

#include "DataModel/PrecalcBaggageData.h"

#include <vector>

namespace tse
{
class BaggageTravelLowerBoundCalculator;
class FareMarket;
class Itin;
class PaxType;
class PaxTypeFare;
class PricingTrx;

class BaggageLowerBoundCalculator
{
public:
  BaggageLowerBoundCalculator(PricingTrx& trx,
                              const Itin& itin,
                              const PaxType& paxType,
                              const FareMarket& fm);
  BaggageLowerBoundCalculator(const BaggageLowerBoundCalculator&) = delete;
  void operator=(const BaggageLowerBoundCalculator&) = delete;

  bool calcLowerBound(PaxTypeFare& ptf);

private:
  std::vector<BaggageTravelLowerBoundCalculator*> initBtCalculators(const FareMarket& fm);

  PricingTrx& _trx;
  const PrecalcBaggage::PaxData& _paxData;
  std::vector<BaggageTravelLowerBoundCalculator*> _btLbCalculator;
};
}
