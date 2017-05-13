//-------------------------------------------------------------------
//
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
//-------------------------------------------------------------------

#include "FreeBagService/AllowanceFinder.h"

#include "Common/Assert.h"
#include "Common/ServiceFeeUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/Itin.h"
#include "DataModel/BaggagePolicy.h"
#include "DataModel/PricingTrx.h"
#include "FreeBagService/AllowanceUtil.h"
#include "FreeBagService/BagValidationOpt.h"
#include "FreeBagService/BaggageTravelInfo.h"

#include <algorithm>

namespace tse
{
AllowanceFinder::AllowanceFinder(AllowanceStepProcessor& allowanceProc,
                                 ChargeStepProcessor& chargeProc,
                                 BaggageTravel& bt,
                                 const CheckedPoint& furthestCp)
  : _allowanceProc(allowanceProc), _chargeProc(chargeProc), _bt(bt), _fcp(furthestCp)
{
  TSE_ASSERT(bt.itin() && !bt.itin()->travelSeg().empty());
  _isInternational = ServiceFeeUtil::isInternationalJourneyType(*bt.itin());
}

void
AllowanceFinder::findAllowanceAndCharges() const
{
  const BaggageTripType btt = _bt.itin()->getBaggageTripType();

  if (btt.isWhollyWithinUsOrCa())
    processWhollyWithin();
  else if (btt.isUsDot())
    processUsDot();
  else
    processNonUsDot();

  processCharges();
}

void
AllowanceFinder::processWhollyWithin() const
{
  static const Ts2ss emptyTs2ss;
  const BagValidationOpt opt(_bt, _fcp, emptyTs2ss, _isInternational, nullptr);

  // Process first check-in carrier
  const AirSeg* const as = _bt.itin()->firstTravelSeg()->toAirSeg();
  TSE_ASSERT(as);
  _bt._carrierTravelSeg = as;
  _bt._allowanceCxr = as->carrier();

  _allowanceProc.matchAllowance(opt);
}

void
AllowanceFinder::processUsDot() const
{
  static const Ts2ss emptyTs2ss;
  const Itin& itin = *_bt.itin();
  const AllowanceUtil::DotTableChecker dotTableCheck(*_bt._trx, itin.getBaggageTripType());

  BagValidationOpt opt(_bt, _fcp, emptyTs2ss, _isInternational, nullptr);

  // Process first check-in carrier
  const auto fciSeg = std::find_if(itin.travelSeg().begin(), itin.travelSeg().end(), dotTableCheck);

  if (fciSeg == itin.travelSeg().end())
  {
    _allowanceProc.dotTableCheckFailed(opt);
    return;
  }

  const AirSeg* const fcis = (**fciSeg).toAirSeg();
  const AirSeg* const mss = (**_bt._MSSJourney).toAirSeg();
  TSE_ASSERT(mss);
  _bt._carrierTravelSeg = fcis;
  _bt._allowanceCxr = fcis->carrier();
  opt._deferTargetCxr = mss->carrier();

  if (_allowanceProc.matchAllowance(opt) != S7_DEFER)
    return;

  // In case of defer go to most significant journey carrier
  _bt._carrierTravelSeg = mss;
  _bt._allowanceCxr = mss->carrier();
  opt._deferTargetCxr.clear();

  if (dotTableCheck(mss) && _allowanceProc.matchAllowance(opt) == S7_PASS)
    return;

  // Otherwise, go back to FCIC but skip defer records
  _bt._carrierTravelSeg = fcis;
  _bt._allowanceCxr = fcis->carrier();

  _allowanceProc.matchAllowance(opt);
}

void
AllowanceFinder::processNonUsDot() const
{
  static const Ts2ss emptyTs2ss;
  BagValidationOpt opt(_bt, _fcp, emptyTs2ss, _isInternational, nullptr);

  // Process MSC
  const AirSeg* const mss = (**_bt._MSS).toAirSeg();
  TSE_ASSERT(mss);
  _bt._carrierTravelSeg = mss;
  _bt._allowanceCxr = AllowanceUtil::getAllowanceCxrNonUsDot(*_bt._trx, *mss);
  opt._deferTargetCxr = AllowanceUtil::getDeferTargetCxrNonUsDot(*_bt._trx, *mss);

  AllowanceStepResult res = _allowanceProc.matchAllowance(opt);

  // Process MSC-defer
  if (res == S7_DEFER)
  {
    _bt._allowanceCxr = opt._deferTargetCxr;
    opt._deferTargetCxr.clear();
    res = _allowanceProc.matchAllowance(opt);
  }

  if (res != S5_FAIL)
    return;

  // Process FCIC
  const AirSeg* const fcis = (**_bt.getTravelSegBegin()).toAirSeg();
  TSE_ASSERT(fcis);

  if (fcis->carrier() == _bt._allowanceCxr) // small optimization
    return;

  _bt._carrierTravelSeg = fcis;
  _bt._allowanceCxr = AllowanceUtil::getAllowanceCxrNonUsDot(*_bt._trx, *fcis);
  opt._deferTargetCxr = AllowanceUtil::getDeferTargetCxrNonUsDot(*_bt._trx, *fcis);

  res = _allowanceProc.matchAllowance(opt);

  // Process FCIC-defer
  if (res == S7_DEFER)
  {
    _bt._allowanceCxr = opt._deferTargetCxr;
    opt._deferTargetCxr.clear();
    _allowanceProc.matchAllowance(opt);
  }
}

void
AllowanceFinder::processCharges() const
{
  static const Ts2ss emptyTs2ss;
  BagValidationOpt opt(_bt, _fcp, emptyTs2ss, _isInternational, nullptr);

  _chargeProc.matchCharges(opt);
}
}
