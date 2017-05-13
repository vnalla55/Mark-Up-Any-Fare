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

#include "FreeBagService/BaggageTravelLowerBoundCalculator.h"

#include "Common/Assert.h"
#include "Common/FreeBaggageUtil.h"
#include "DataModel/BaggageCharge.h"
#include "DataModel/BaggagePolicy.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PricingTrx.h"
#include "FreeBagService/AllowanceFinder.h"
#include "FreeBagService/AllowanceUtil.h"
#include "FreeBagService/ChargesUtil.h"
#include "FreeBagService/BaggageRevalidator.h"
#include "FreeBagService/BagValidationOpt.h"
#include "Util/Algorithm/Bitset.h"

#include <array>
#include <bitset>

#include <boost/logic/tribool.hpp>

namespace tse
{
namespace
{
const CheckedPoint dummyCheckedPoint;
}

BaggageTravelLowerBoundCalculator::BaggageTravelLowerBoundCalculator(
    const PrecalcBaggage::BagTravelData& btData, const FareMarket& fm)
  : _trx(*btData.bagTravel->_trx),
    _btData(btData),
    _requestedPieces(_trx.getBaggagePolicy().getRequestedBagPieces()),
    _bt(_trx.dataHandle().safe_create<BaggageTravel>(*btData.bagTravel)),
    _revalidator(&_trx.dataHandle().safe_create<BaggageRevalidator>(_trx, _bt, _ts2ss))
{
}

MoneyAmount
BaggageTravelLowerBoundCalculator::calcLowerBound(PaxTypeFare& ptf)
{
  _ts2ss.clear();
  for (TravelSeg* ts : ptf.fareMarket()->travelSeg())
    _ts2ss[ts] = {nullptr, &ptf};
  _revalidator->onFaresUpdated(nullptr);
  _lowerBound = INVALID_FEE_AMT;

  AllowanceFinder finder(*this, *this, _bt, dummyCheckedPoint);
  finder.findAllowanceAndCharges();

  return _lowerBound;
}

void
BaggageTravelLowerBoundCalculator::dotTableCheckFailed(const BagValidationOpt& opt)
{
  // just don't add any info regarding free pieces
}

AllowanceStepResult
BaggageTravelLowerBoundCalculator::matchAllowance(const BagValidationOpt& opt)
{
  const PrecalcBaggage::CxrPair cxrPair(_bt._allowanceCxr, opt._deferTargetCxr);
  const auto allowanceIt = _btData.allowance.find(cxrPair);
  TSE_ASSERT(allowanceIt != _btData.allowance.end());
  const PrecalcBaggage::AllowanceRecords& allowance = allowanceIt->second;

  if (!allowance.s5Found)
    return S5_FAIL;

  uint32_t localFreePieces = 0;
  boost::logic::tribool deferResult = false;
  boost::logic::tribool nonDeferResult = false;

  for (const OCFees* const oc : allowance.s7s)
  {
    const boost::logic::tribool result = _revalidator->revalidateS7(*oc, true);
    if (!result)
      continue;

    if (AllowanceUtil::isDefer(*oc->optFee()))
      deferResult = deferResult || result;
    else
    {
      localFreePieces = std::max(localFreePieces, FreeBaggageUtil::calcFirstChargedPiece(oc));
      nonDeferResult = nonDeferResult || result;
    }

    if (result) // It shouldn't be necessary but just in case
      break;
  }

  const bool isOnlyDeferPossible = deferResult && !nonDeferResult;

  if (!isOnlyDeferPossible)
  {
    uint32_t& maxFreePiecesForCxr = _maxFreePieces[cxrPair.allowanceCxr()];
    maxFreePiecesForCxr = std::max(maxFreePiecesForCxr, localFreePieces);
  }

  if (boost::indeterminate(deferResult) || deferResult) // defer possible
    return S7_DEFER;

  return nonDeferResult ? S7_PASS : S7_FAIL;
}

void
BaggageTravelLowerBoundCalculator::matchCharges(const BagValidationOpt& opt)
{
  for (const auto& cxrFreePieces : _maxFreePieces)
  {
    const uint32_t freePieces = cxrFreePieces.second;

    if (freePieces >= _requestedPieces)
    {
      _lowerBound = 0;
      break;
    }

    const auto chargeIt = _btData.charges.find(cxrFreePieces.first);
    TSE_ASSERT(chargeIt != _btData.charges.end());
    const MoneyAmount cxrLowerBound = calcLowerBoundForCxr(freePieces, chargeIt->second);
    _lowerBound = std::min(_lowerBound, cxrLowerBound);
  }
}

MoneyAmount
BaggageTravelLowerBoundCalculator::calcLowerBoundForCxr(
    uint32_t freePieces, const PrecalcBaggage::ChargeRecords& cxrRecords)
{
  const size_t numExcessBags = _requestedPieces - freePieces;

  std::array<BaggageCharge*, MAX_BAG_PIECES> charges;
  charges.fill(nullptr);

  for (const auto& s5Records : cxrRecords.s7s)
  {
    std::bitset<MAX_BAG_PIECES> remainingExcessBags;
    alg::fill_bitset(remainingExcessBags, 0, numExcessBags);

    for (BaggageCharge* const bc : s5Records.second)
    {
      if ((remainingExcessBags & bc->matchedBags()).none())
        continue;

      const boost::logic::tribool result = _revalidator->revalidateS7(*bc, true);

      if (!result)
        continue;

      for (size_t excessBagNo = 0; excessBagNo < numExcessBags; ++excessBagNo)
        if (remainingExcessBags.test(excessBagNo) && bc->matchedBag(excessBagNo))
          ChargesUtil::selectCheaper(charges[freePieces + excessBagNo], *bc);

      if (result)
        remainingExcessBags &= ~bc->matchedBags();

      if (remainingExcessBags.none())
        break;
    }
  }

  MoneyAmount lb = 0.0;

  for (size_t bagNo = freePieces; bagNo < _requestedPieces; ++bagNo)
  {
    if (!charges[bagNo])
      return INVALID_FEE_AMT;
    lb += charges[bagNo]->feeAmount();
  }

  return lb;
}
}
