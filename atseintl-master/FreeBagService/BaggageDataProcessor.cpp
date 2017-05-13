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
#include "FreeBagService/BaggageDataProcessor.h"

#include "Common/Thread/TseRunnableExecutor.h"
#include "DataModel/BaggagePolicy.h"
#include "Diagnostic/Diag852Collector.h"
#include "Diagnostic/DiagManager.h"
#include "FreeBagService/BaggageThreadTask.h"
#include "FreeBagService/BaggageTicketingDateScope.h"
#include "FreeBagService/BaggageTravelInfo.h"
#include "FreeBagService/DataStrategyBase.h"
#include "FreeBagService/FreeBagService.h"

namespace tse
{
BaggageDataProcessor::BaggageDataProcessor(
    PricingTrx& trx,
    const std::map<uint32_t, std::vector<BaggageTravel*> >& farePath2BaggageTravels,
    const CheckedPoint& furthestCheckedPoint,
    BaggageTripType baggageTripType)
  : _trx(trx),
    _farePath2BaggageTravels(farePath2BaggageTravels),
    _furthestCheckedPoint(furthestCheckedPoint),
    _baggageTripType(baggageTripType)
{
}

void
BaggageDataProcessor::process(const DataStrategyBase& dataStrategy) const
{
  Diag852Collector* dc(nullptr);
  DiagManager diagMgr(_trx, Diagnostic852);

  if (diagMgr.isActive())
  {
    dc = &dynamic_cast<Diag852Collector&>(diagMgr.collector());
    if (dc->diagType() < Diag852Collector::FLACTIVE)
      dc = nullptr;
  }

  for (const auto& fpToBtItem : _farePath2BaggageTravels)
  {
    const uint32_t farePathIndex = fpToBtItem.first;
    const std::vector<BaggageTravel*>& baggageTravels = fpToBtItem.second;
    FarePath* farePath = baggageTravels.empty() ? nullptr : baggageTravels[0]->farePath();

    if (!farePath)
      continue;

    BaggageTicketingDateScope scopedDateSetter(_trx, farePath);

    if (dc)
      processSynchronously(dataStrategy, baggageTravels, farePathIndex, dc);
    else
      processAsynchronously(dataStrategy, baggageTravels, farePathIndex);
  }
}

void
BaggageDataProcessor::processAsynchronously(const DataStrategyBase& dataStrategy,
                                            const std::vector<BaggageTravel*>& baggageTravels,
                                            uint32_t fareIndex) const
{
  typedef std::shared_ptr<BaggageThreadTask> TaskPtr;
  std::vector<TaskPtr> tasks;
  TseRunnableExecutor taskExecutor(TseThreadingConst::BAGGAGE_TASK);

  for (uint32_t i = 0; i < baggageTravels.size(); ++i)
  {
    TaskPtr baggageTask(new BaggageThreadTask(_trx,
                                              dataStrategy,
                                              baggageTravels[i],
                                              BaggageTravelInfo(i, fareIndex),
                                              _furthestCheckedPoint,
                                              _baggageTripType,
                                              nullptr));
    tasks.push_back(baggageTask);
    taskExecutor.execute(*baggageTask);
  }
  taskExecutor.wait();
}

void
BaggageDataProcessor::processSynchronously(const DataStrategyBase& dataStrategy,
                                           const std::vector<BaggageTravel*>& baggageTravels,
                                           uint32_t fareIndex,
                                           Diag852Collector* dc) const
{
  for (uint32_t i = 0; i < baggageTravels.size(); ++i)
  {
    dataStrategy.processBaggageTravel(baggageTravels[i],
                                      BaggageTravelInfo(i, fareIndex),
                                      _furthestCheckedPoint,
                                      _baggageTripType,
                                      dc);
  }
}

} // tse
