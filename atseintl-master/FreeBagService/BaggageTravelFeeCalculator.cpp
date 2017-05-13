//-------------------------------------------------------------------
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

#include "FreeBagService/BaggageTravelFeeCalculator.h"

#include "Common/Assert.h"
#include "Common/FreeBaggageUtil.h"
#include "DataModel/BaggageCharge.h"
#include "DataModel/BaggagePolicy.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/PricingTrx.h"
#include "FreeBagService/AllowanceFinder.h"
#include "FreeBagService/AllowanceUtil.h"
#include "FreeBagService/BaggageRevalidator.h"
#include "FreeBagService/BagValidationOpt.h"
#include "FreeBagService/ChargesUtil.h"
#include "Util/Algorithm/Bitset.h"

namespace tse
{
namespace
{
const CheckedPoint dummyCheckedPoint;
}

BaggageTravelFeeCalculator::BaggageTravelFeeCalculator(const PrecalcBaggage::BagTravelData& btData,
                                                       const Ts2ss& ts2ss)
  : _trx(*btData.bagTravel->_trx),
    _btData(btData),
    _requestedPieces(_trx.getBaggagePolicy().getRequestedBagPieces()),
    _bt(_trx.dataHandle().safe_create<BaggageTravel>(*btData.bagTravel)),
    _revalidator(&_trx.dataHandle().safe_create<BaggageRevalidator>(_trx, _bt, ts2ss))
{
}

MoneyAmount
BaggageTravelFeeCalculator::calcFee(FarePath& fp)
{
  _revalidator->onFaresUpdated(&fp);

  AllowanceFinder finder(*this, *this, _bt, dummyCheckedPoint);
  finder.findAllowanceAndCharges();

  MoneyAmount btFee = 0.0;

  for (size_t bagNo = FreeBaggageUtil::calcFirstChargedPiece(_bt._allowance);
       bagNo < _requestedPieces;
       ++bagNo)
  {
    if (!_bt._charges[bagNo])
      return INVALID_FEE_AMT;
    btFee += _bt._charges[bagNo]->feeAmount();
  }

  return btFee;
}

void
BaggageTravelFeeCalculator::dotTableCheckFailed(const BagValidationOpt& opt)
{
  opt._bt._processCharges = false;
  opt._bt._allowance = nullptr;
  opt._bt._charges.fill(nullptr);
}

AllowanceStepResult
BaggageTravelFeeCalculator::matchAllowance(const BagValidationOpt& opt)
{
  const PrecalcBaggage::CxrPair cxrPair(opt._bt._allowanceCxr, opt._deferTargetCxr);
  const auto allowanceIt = _btData.allowance.find(cxrPair);
  TSE_ASSERT(allowanceIt != _btData.allowance.end());
  const PrecalcBaggage::AllowanceRecords& allowance = allowanceIt->second;

  opt._bt._processCharges = allowance.s5Found;
  opt._bt._allowance = nullptr;
  opt._bt._charges.fill(nullptr);

  if (!allowance.s5Found)
    return S5_FAIL;

  _revalidator->setDeferTargetCxr(opt._deferTargetCxr);

  for (OCFees* oc : allowance.s7s)
  {
    if (!_revalidator->revalidateS7(*oc))
      continue;
    opt._bt._allowance = oc;
    return AllowanceUtil::isDefer(*oc->optFee()) ? S7_DEFER : S7_PASS;
  }

  return S7_FAIL;
}

void
BaggageTravelFeeCalculator::matchCharges(const BagValidationOpt& opt)
{
  if (!opt._bt._processCharges)
    return;

  const uint32_t freePieces = FreeBaggageUtil::calcFirstChargedPiece(opt._bt._allowance);

  if (freePieces >= _requestedPieces)
    return;

  const auto chargeIt = _btData.charges.find(opt._bt._allowanceCxr);
  TSE_ASSERT(chargeIt != _btData.charges.end());
  const PrecalcBaggage::ChargeRecords& charge = chargeIt->second;

  for (const auto& s5Records : charge.s7s)
    selectChargesForExcessPieces(opt, freePieces, s5Records.second);
}

void
BaggageTravelFeeCalculator::selectChargesForExcessPieces(const BagValidationOpt& opt,
                                                         const uint32_t freePieces,
                                                         const std::vector<BaggageCharge*>& charges)
    const
{
  const size_t numExcessBags = _requestedPieces - freePieces;

  std::bitset<MAX_BAG_PIECES> remainingExcessBags;
  alg::fill_bitset(remainingExcessBags, 0, numExcessBags);

  for (BaggageCharge* bc : charges)
  {
    if ((remainingExcessBags & bc->matchedBags()).none() || !_revalidator->revalidateS7(*bc))
      continue;

    for (size_t excessBagNo = 0; excessBagNo < numExcessBags; ++excessBagNo)
      if (remainingExcessBags.test(excessBagNo) && bc->matchedBag(excessBagNo))
        ChargesUtil::selectCheaper(opt._bt._charges[freePieces + excessBagNo], *bc);

    remainingExcessBags &= ~bc->matchedBags();

    if (remainingExcessBags.none())
      break;
  }
}
}
