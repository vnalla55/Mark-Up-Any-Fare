/*---------------------------------------------------------------------------
 *  File:    BaggageThreadTask.h
 *
 *  Copyright Sabre 2011
 *    The copyright to the computer program(s) herein
 *    is the property of Sabre.
 *    The program(s) may be used and/or copied only with
 *    the written permission of Sabre or in accordance
 *    with the terms and conditions stipulated in the
 *    agreement/contract under which the program(s)
 *    have been supplied.
 *-------------------------------------------------------------------------*/

#include "Common/BaggageTripType.h"
#include "Common/Thread/TseCallableTrxTask.h"
#include "Common/TseEnums.h"
#include "FreeBagService/BaggageTravelInfo.h"
#include "FreeBagService/DataStrategyBase.h"

#pragma once

namespace tse
{
class PricingTrx;

class BaggageThreadTask : public TseCallableTrxTask
{
public:
  BaggageThreadTask(PricingTrx& trx,
                    const DataStrategyBase& strategy,
                    BaggageTravel* baggageTravel,
                    const BaggageTravelInfo& bagInfo,
                    const CheckedPoint& furthestCheckedPoint,
                    BaggageTripType btt,
                    Diag852Collector* dc)
    : _strategy(strategy),
      _baggageTravel(baggageTravel),
      _bagInfo(bagInfo),
      _furthestCheckedPoint(furthestCheckedPoint),
      _baggageTripType(btt),
      _dc(dc)
  {
    TseCallableTrxTask::trx(&trx);
    TseCallableTrxTask::desc("BAGGAGE TASK");
  }

  void performTask() override
  {
    _strategy.processBaggageTravel(
        _baggageTravel, _bagInfo, _furthestCheckedPoint, _baggageTripType, _dc);
  }

private:
  const DataStrategyBase& _strategy;
  BaggageTravel* _baggageTravel;
  BaggageTravelInfo _bagInfo;
  const CheckedPoint& _furthestCheckedPoint;
  BaggageTripType _baggageTripType;
  Diag852Collector* _dc;
};
}
