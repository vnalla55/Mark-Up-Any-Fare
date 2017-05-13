//-------------------------------------------------------------------
//  Copyright Sabre 2011
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
#pragma once

#include "Common/BaggageTripType.h"
#include "DataModel/BaggageTravel.h"

#include <vector>

namespace tse
{
class PricingTrx;
class DataStrategyBase;
class Diag852Collector;

class BaggageDataProcessor
{
  friend class BaggageDataProcessorTest;

public:
  BaggageDataProcessor(
      PricingTrx& trx,
      const std::map<uint32_t, std::vector<BaggageTravel*> >& farePath2BaggageTravels,
      const CheckedPoint& furthestCheckedPoint,
      BaggageTripType baggageTripType);

  void process(const DataStrategyBase& dataStrategy) const;

private:
  void processAsynchronously(const DataStrategyBase& dataStrategy,
                             const std::vector<BaggageTravel*>& baggageTravels,
                             uint32_t fareIndex) const;

  void processSynchronously(const DataStrategyBase& dataStrategy,
                           const std::vector<BaggageTravel*>& baggageTravels,
                           uint32_t fareIndex,
                           Diag852Collector* dc) const;

  PricingTrx& _trx;
  const std::map<uint32_t, std::vector<BaggageTravel*> >& _farePath2BaggageTravels;
  const CheckedPoint& _furthestCheckedPoint;
  BaggageTripType _baggageTripType;
};
} // tse
