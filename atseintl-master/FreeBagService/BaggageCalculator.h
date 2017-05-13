//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------

#pragma once

#include "Common/OcTypes.h"
#include "DataModel/PrecalcBaggageData.h"

#include <vector>

namespace tse
{
class BaggageTravelFeeCalculator;
class FarePath;
class Itin;
class PaxType;
class PricingTrx;

class BaggageCalculator
{
public:
  BaggageCalculator(PricingTrx& trx, const Itin& itin, const PaxType& paxType);
  BaggageCalculator(const BaggageCalculator&) = delete;
  void operator=(const BaggageCalculator&) = delete;

  bool chargeFarePath(FarePath& fp);

private:
  std::vector<BaggageTravelFeeCalculator*> initBtCalculators() const;
  void diagCalculationDetails(const FarePath& fp, MoneyAmount lbound) const;

  Ts2ss _ts2ss;
  PricingTrx& _trx;
  const PrecalcBaggage::PaxData& _paxData;
  const std::vector<BaggageTravelFeeCalculator*> _btFeeCalculator;
};
}
