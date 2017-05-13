//-------------------------------------------------------------------
//  Copyright Sabre 2007
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
#include "DataModel/ExcItin.h"

#include "Common/Assert.h"
#include "Common/ExchangeUtil.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/NetFarePath.h"
#include "DataModel/RexPricingTrx.h"
#include "RexPricing/FarePathChangeDetermination.h"


namespace tse
{

bool
SameSegs::
operator()(const TravelSeg* newSeg, const TravelSeg* excSeg) const
{
  if (newSeg->boardMultiCity() != excSeg->boardMultiCity() ||
      newSeg->offMultiCity() != excSeg->offMultiCity() ||
      newSeg->pssDepartureDate() != excSeg->pssDepartureDate())
    return false;

  const AirSeg* newAs = dynamic_cast<const AirSeg*>(newSeg);
  const AirSeg* excAs = dynamic_cast<const AirSeg*>(excSeg);
  if (!newAs && !excAs)
    return true;

  if ((newAs && !excAs) || (!newAs && excAs))
    return false;

  if (newAs->carrier() != excAs->carrier() || newAs->flightNumber() != excAs->flightNumber())
    return false;

  return true;
}

void
ExcItin::setTktValidityDate(Year year, Month month, Day day)
{
  _tktValidityDate = DateTime(year, month, day);
}

FareCompInfo*
ExcItin::findFareCompInfo(const FareMarket* fareMarket) const
{
  for (FareCompInfo* fci : fareComponent())
    if (fareMarket == fci->fareMarket() || fareMarket == fci->secondaryFareMarket())
      return fci;

  return nullptr;
}

const DateTime&
ExcItin::travelDate() const
{
  return _rexTrx ? _rexTrx->adjustedTravelDate(_travelDate) : _travelDate;
}

void
ExcItin::setExcItinIndex(uint16_t index, size_t maxIndex)
{
  TSE_ASSERT(index < maxIndex);
  if (index >= _someSegmentsChanged.size())
    _someSegmentsChanged.resize(maxIndex);

  if (index >= _someSegmentsConfirmed.size())
    _someSegmentsConfirmed.resize(maxIndex);

  if (index >= _sameSegmentsInventoryChanged.size())
    _sameSegmentsInventoryChanged.resize(maxIndex);

  if (index >= _domesticCouponChange.size())
    _domesticCouponChange.resize(maxIndex);
  if (index >= _stopOverChange.size())
    _stopOverChange.resize(maxIndex);

  _excItinIndex = index;

  for (const auto seg : travelSeg())
  {
    if (index >= seg->changeStatusVec().size())
    {
      seg->changeStatusVec().resize(maxIndex, TravelSeg::CHANGED);
    }

    if (index >= seg->newTravelUsedToSetChangeStatusVec().size())
    {
      seg->newTravelUsedToSetChangeStatusVec().resize(maxIndex);
    }
    if (index >= seg->isCabinChangedVec().size())
    {
      seg->isCabinChangedVec().resize(maxIndex);
    }
  }

  for (const auto fm : fareMarket())
  {
    fm->setExcItinIndex(index);

    if (index >= fm->changeStatusVec().size())
      fm->changeStatusVec().resize(maxIndex, tse::UK);

    if (index >= fm->asBookChangeStatusVec().size())
      fm->asBookChangeStatusVec().resize(maxIndex, tse::UK);
  }
}

bool&
ExcItin::someSegmentsChanged()
{
  TSE_ASSERT(getExcItinIndex() < _someSegmentsChanged.size());
  return _someSegmentsChanged[getExcItinIndex()];
}
const bool
ExcItin::someSegmentsChanged() const
{
  TSE_ASSERT(getExcItinIndex() < _someSegmentsChanged.size());
  return _someSegmentsChanged[getExcItinIndex()];
}

bool&
ExcItin::someSegmentsConfirmed()
{
  TSE_ASSERT(getExcItinIndex() < _someSegmentsConfirmed.size());
  return _someSegmentsConfirmed[getExcItinIndex()];
}
const bool
ExcItin::someSegmentsConfirmed() const
{
  TSE_ASSERT(getExcItinIndex() < _someSegmentsConfirmed.size());
  return _someSegmentsConfirmed[getExcItinIndex()];
}

bool&
ExcItin::sameSegmentsInventoryChanged()
{
  TSE_ASSERT(getExcItinIndex() < _sameSegmentsInventoryChanged.size());
  return _sameSegmentsInventoryChanged[getExcItinIndex()];
}
const bool
ExcItin::sameSegmentsInventoryChanged() const
{
  TSE_ASSERT(getExcItinIndex() < _sameSegmentsInventoryChanged.size());
  return _sameSegmentsInventoryChanged[getExcItinIndex()];
}

bool&
ExcItin::stopOverChange()
{
  TSE_ASSERT(getExcItinIndex() < _stopOverChange.size());
  return _stopOverChange[getExcItinIndex()];
}

const bool
ExcItin::stopOverChange() const
{
  TSE_ASSERT(getExcItinIndex() < _stopOverChange.size());
  return _stopOverChange[getExcItinIndex()];
}

bool&
ExcItin::domesticCouponChange()
{
  TSE_ASSERT(getExcItinIndex() < _domesticCouponChange.size());
  return _domesticCouponChange[getExcItinIndex()];
}

const bool
ExcItin::domesticCouponChange() const
{
  TSE_ASSERT(getExcItinIndex() < _domesticCouponChange.size());
  return _domesticCouponChange[getExcItinIndex()];
}

bool
ExcItin::doNotUseForPricing(const FareMarket& fm, const Itin& itin)
{
  for (FareCompInfo* fci : fareComponent())
    if (fci->partialFareBreakLimitationValidation().doNotUseForPricing(fm, itin))
      return true;

  return false;
}

void
ExcItin::determineSegsChangesFor988Match()
{
  GenericRemoveOpens<std::list, const TravelSeg*> newWoOpens;
  newWoOpens.remove(_rexTrx->curNewItin()->travelSeg());

  for (const FareMarket* efm : fareMarket())
    if (!efm->breakIndicator())
      matchTSs(*efm, newWoOpens.get());
}

Money
ExcItin::getNonRefAmount() const
{
  TSE_ASSERT(_rexTrx);
  const FarePath* nfp = farePath().front()->netFarePath();
  if (_rexTrx->getRexOptions().isNetSellingIndicator() && nfp)
  {
    return ExchangeUtil::convertCurrency(*_rexTrx,
                                         nfp->getNonrefundableAmount(*_rexTrx),
                                         _rexTrx->getTotalBaseFareAmount().code(),
                                         _rexTrx->itin().front()->useInternationalRounding());
  }

  if (_rexTrx->isExcNonRefInRequest())
  {
    return _rexTrx->getExcNonRefAmount();
  }

  return ExchangeUtil::convertCurrency(*_rexTrx,
                                       farePath().front()->getNonrefundableAmount(*_rexTrx),
                                       _rexTrx->getTotalBaseFareAmount().code(),
                                       _rexTrx->itin().front()->useInternationalRounding());


}

Money
ExcItin::getNonRefAmountInNUC() const
{
  if (_rexTrx && _rexTrx->isExcNonRefInRequest())
  {
    return ExchangeUtil::convertCurrency(*_rexTrx,
                                         _rexTrx->getExcNonRefAmount(),
                                         NUC,
                                         _rexTrx->itin().front()->useInternationalRounding());
  }

  return Money(farePath().front()->getNonrefundableAmountInNUC(*_rexTrx), NUC);
}

void
ExcItin::matchTSs(const FareMarket& efm, std::list<const TravelSeg*>& ntss)
{
  _changedSegs.insert(std::make_pair(&efm, std::vector<bool>()));

  for (const TravelSeg* ets : efm.travelSeg())
  {
    if (ets->segmentType() != Open) // ets->unflown() can flown be changed ???
    {
      std::list<const TravelSeg*>::iterator samei =
          std::find_if(ntss.begin(), ntss.end(), std::bind1st(SameSegs(), ets));

      if (samei == ntss.end())
      {
        _changedSegs.at(&efm).push_back(true);
        continue;
      }

      ntss.erase(samei);
    }

    _changedSegs.at(&efm).push_back(false);
  }
}

} // namespace tse
