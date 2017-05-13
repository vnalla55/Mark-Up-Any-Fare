//-------------------------------------------------------------------
//
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
//-------------------------------------------------------------------

#include "Pricing/FareMarketPath.h"

#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "Pricing/MergedFareMarket.h"

namespace tse
{
void
FareMarketPath::calculateFirstFareAmt(const std::vector<PaxType*>& paxTypes)
{
  for (const PaxType* paxType : paxTypes)
  {
    MoneyAmount firstFareAmt = 0.0;

    for (const MergedFareMarket* mfm : _fareMarketPath)
    {
      if (const auto first = mfm->firstFare(paxType))
        firstFareAmt += first->totalNucAmount;
      else // no valid fares found
      {
        firstFareAmt = std::numeric_limits<MoneyAmount>::max();
        break;
      }
    }

    _paxFirstFareAmt[paxType] = firstFareAmt;
  }
}

void
FareMarketPath::calculateThroughFarePrecedenceRankOld()
{
  _throughFarePrecedenceRank = 0;
  for (const MergedFareMarket* mfm : _fareMarketPath)
    if (!mfm->travelSeg().back()->stopOver())
      ++_throughFarePrecedenceRank;
}

void
FareMarketPath::calculateThroughFarePrecedenceRank()
{
  _throughFarePrecedenceRank = _fareMarketPath.size();
}

// in order for a fmp to be valid for IBF it has to have at least one hard pass on each leg
// ANY_BRAND_LEG_PARITY path is also checked for presence of hard pass on each leg
// as mfm->hardPassExists() is also set for this path in FareMarketPathMerger
// an exception is a NO_BRAND path (catch all bucket) in which brands are irrelevant
bool
FareMarketPath::isValidForIbf() const
{
  if (_brandCode == NO_BRAND)
    return true;

  FlatMap<uint16_t, std::vector<const MergedFareMarket*>> legMfms;

  for (const MergedFareMarket* mfm : _fareMarketPath)
    for (TravelSeg* tSeg : mfm->travelSeg())
      legMfms[tSeg->legId()].push_back(mfm);

  for (const auto& mfms : legMfms) // Loop per leg
  {
    bool hardPassPerLegExists = false;

    for (const MergedFareMarket* mfm : mfms.second)
    {
      // ANY_BRAND means brand substitution on a soldout leg in "change brand for soldout"
      // legs on which this happens are "fake" so we don't care about hard passes in this case
      if (mfm->hardPassExists() || mfm->brandCode() == ANY_BRAND)
      {
        hardPassPerLegExists = true;
        break;
      }
    }

    if (!hardPassPerLegExists)
      return false;
  }

  return true; // all legs have at least one hard pass.
}

bool
FareMarketPath::thruPricingCalculate(PricingTrx* trx)
{
  for (const MergedFareMarket* mfm : _fareMarketPath)
  {
    bool throughFM = false;

    for (const PricingTrx::OriginDestination& origDest : trx->originDest())
    {
      if (mfm->boardMultiCity() == origDest.boardMultiCity &&
          mfm->offMultiCity() == origDest.offMultiCity)
      {
        throughFM = true;
        break;
      }
    }

    if (!throughFM)
    {
      _thruPricing = Thru::NO;
      return false;
    }
  }

  _thruPricing = Thru::YES;
  return true;
}
}
