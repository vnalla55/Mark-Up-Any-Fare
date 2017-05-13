#include "Pricing/PUPath.h"

#include "Pricing/MergedFareMarket.h"
#include "Pricing/PU.h"

#include <algorithm>

namespace tse
{
bool
PUPath::isMarketAssigned(const MergedFareMarket* fmkt)
{
  return std::any_of(_puPath.cbegin(),
                     _puPath.cend(),
                     [fmkt](PU* pu)
                     { return pu->isMarketAssigned(fmkt); });
}

void
PUPath::countTotalFC()
{
  for (const PU* pu : _allPU)
    _totalFC += pu->fcCount();
}

bool
PUPath::
operator<(const PUPath& otherPuPath) const
{
  if (&otherPuPath == this)
    return false;

  if (puPath().size() != otherPuPath.puPath().size())
    return puPath().size() < otherPuPath.puPath().size();

  for (size_t n = 0; n < puPath().size(); ++n)
  {
    const PU* pu1 = puPath()[n];
    const PU* pu2 = otherPuPath.puPath()[n];

    if (pu1->puType() != pu2->puType())
      return pu1->puType() < pu2->puType();

    const std::vector<MergedFareMarket*>& fm1 = pu1->fareMarket();
    const std::vector<MergedFareMarket*>& fm2 = pu2->fareMarket();

    if (fm1.size() != fm2.size())
      return fm1.size() < fm2.size();

    for (size_t t = 0; t < fm1.size(); ++t)
    {
      MergedFareMarket* mfm1 = fm1[t];
      MergedFareMarket* mfm2 = fm2[t];

      if (mfm1->boardMultiCity() != mfm2->boardMultiCity())
        return mfm1->boardMultiCity() < mfm2->boardMultiCity();

      if (mfm1->offMultiCity() != mfm2->offMultiCity())
        return mfm1->offMultiCity() < mfm2->offMultiCity();

      if (mfm1->governingCarrier() != mfm2->governingCarrier())
        return mfm1->governingCarrier() < mfm2->governingCarrier();

      if (mfm1->travelSeg().size() != mfm2->travelSeg().size())
        return mfm1->travelSeg().size() < mfm2->travelSeg().size();
    }
  }
  return false;
}

void
PUPath::addSpanishResidentAmount(const LocCode& board, const LocCode& off, const CarrierCode& govCrx,
                                 const CarrierCode& valCrx, MoneyAmount referenceFareAmount)
{
  if(!_spanishData)
      return;
  _spanishData->spanishResidentAmount.emplace(std::make_tuple(board, off, govCrx, valCrx),
                                              referenceFareAmount);
}

MoneyAmount
PUPath::findSpanishResidentAmount(const FareMarket& fareMarket,
                                  const CarrierCode& govCrx,
                                  const CarrierCode& valCrx) const
{
  if (!_spanishData)
    return 0.0;

  const auto pos = _spanishData->spanishResidentAmount.find(
      std::make_tuple(fareMarket.origin()->loc(), fareMarket.destination()->loc(),
                      govCrx, valCrx));

  if (pos != _spanishData->spanishResidentAmount.cend())
    return pos->second;

  return 0.0;
}

const SpanishData::YFlexiMap&
PUPath::getSpanishResidentAmount() const
{
  return _spanishData->spanishResidentAmount;
}
} // tse namespace
