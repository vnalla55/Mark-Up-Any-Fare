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

#include "FreeBagService/BaggageLowerBoundCalculator.h"

#include "DataModel/BaggageTravel.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "FreeBagService/BaggageTravelLowerBoundCalculator.h"

namespace tse
{
BaggageLowerBoundCalculator::BaggageLowerBoundCalculator(PricingTrx& trx,
                                                         const Itin& itin,
                                                         const PaxType& paxType,
                                                         const FareMarket& fm)
  : _trx(trx), _paxData(itin.getPrecalcBagPaxData(paxType)), _btLbCalculator(initBtCalculators(fm))
{
}

bool
BaggageLowerBoundCalculator::calcLowerBound(PaxTypeFare& ptf)
{
  MoneyAmount lb = 0.0;

  for (BaggageTravelLowerBoundCalculator* btCalc : _btLbCalculator)
  {
    const MoneyAmount btLb = btCalc->calcLowerBound(ptf);

    if (btLb == BaggageTravelLowerBoundCalculator::INVALID_FEE_AMT)
    {
      ptf.setValidForBaggage(false);
      return false;
    }

    lb += btLb;
  }

  ptf.setBaggageLowerBound(lb);
  return true;
}

std::vector<BaggageTravelLowerBoundCalculator*>
BaggageLowerBoundCalculator::initBtCalculators(const FareMarket& fm)
{
  const auto& fmSegs = fm.travelSeg();
  std::vector<BaggageTravelLowerBoundCalculator*> out;

  for (const PrecalcBaggage::BagTravelData& btData : _paxData.bagTravelData)
  {
    TSE_ASSERT(btData.bagTravel->getTravelSegBegin() != btData.bagTravel->getTravelSegEnd());
    const TravelSeg* firstBtSeg = *btData.bagTravel->getTravelSegBegin();
    const bool isApplicableBt =
        std::any_of(fmSegs.begin(),
                    fmSegs.end(),
                    [=](const TravelSeg* ts)
                    { return ts->segmentOrder() == firstBtSeg->segmentOrder(); });

    if (isApplicableBt)
      out.push_back(&_trx.dataHandle().safe_create<BaggageTravelLowerBoundCalculator>(btData, fm));
  }

  return out;
}
}
