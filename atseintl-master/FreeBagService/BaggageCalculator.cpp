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

#include "FreeBagService/BaggageCalculator.h"

#include "Common/ServiceFeeUtil.h"
#include "DataModel/FarePath.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "Diagnostic/DiagManager.h"
#include "Diagnostic/Diag852Collector.h"
#include "FreeBagService/BaggageTravelFeeCalculator.h"

namespace tse
{
BaggageCalculator::BaggageCalculator(PricingTrx& trx, const Itin& itin, const PaxType& paxType)
  : _trx(trx),
    _paxData(itin.getPrecalcBagPaxData(paxType)),
    _btFeeCalculator(initBtCalculators())
{
}

bool
BaggageCalculator::chargeFarePath(FarePath& fp)
{
  // rollback baggage lower bound
  MoneyAmount baggageLowerBound = 0;
  for (PricingUnit* pu : fp.pricingUnit())
  {
    baggageLowerBound += pu->baggageLowerBound();
    pu->rollbackBaggageLowerBound();
  }
  fp.decreaseTotalNUCAmount(baggageLowerBound);

  _ts2ss.clear();
  ServiceFeeUtil::collectSegmentStatus(fp, _ts2ss);
  MoneyAmount totalBagFee = 0.0;

  for (BaggageTravelFeeCalculator* btCalc : _btFeeCalculator)
  {
    const MoneyAmount btFee = btCalc->calcFee(fp);

    if (btFee == BaggageTravelFeeCalculator::INVALID_FEE_AMT)
      return false;

    totalBagFee += btFee;
  }

  fp.setBagChargeNUCAmount(totalBagFee);
  diagCalculationDetails(fp, baggageLowerBound);
  return true;
}

inline std::vector<BaggageTravelFeeCalculator*>
BaggageCalculator::initBtCalculators() const
{
  std::vector<BaggageTravelFeeCalculator*> out;
  out.reserve(_paxData.bagTravelData.size());

  for (const PrecalcBaggage::BagTravelData& btData : _paxData.bagTravelData)
    out.emplace_back(&_trx.dataHandle().safe_create<BaggageTravelFeeCalculator>(btData, _ts2ss));

  return out;
}

void
BaggageCalculator::diagCalculationDetails(const FarePath& fp, MoneyAmount lbound) const
{
  DiagManager dm(_trx, Diagnostic852, Diag852Collector::PQ);

  if (!dm.isActive())
    return;

  Diag852Collector& dc = static_cast<Diag852Collector&>(dm.collector());
  dc.printFarePathBaggageCharge(fp, lbound);
  for (const BaggageTravelFeeCalculator* btCalc : _btFeeCalculator)
    dc.printBaggageTravelCharges(btCalc->baggageTravel());
}
}
