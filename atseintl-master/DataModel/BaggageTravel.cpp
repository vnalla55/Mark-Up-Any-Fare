#include "DataModel/BaggageTravel.h"

#include "Common/DateTime.h"
#include "Common/TrxUtil.h"
#include "DBAccess/FreqFlyerStatusSeg.h"
#include "DataModel/BaggagePolicy.h"
#include "DataModel/FarePath.h"
#include "DataModel/TravelSeg.h"
#include "Diagnostic/Diag852Collector.h"

#include <algorithm>

namespace tse
{
void
BaggageTravel::clone(const BaggageTravel& original, FarePath* farePath)
{
  *this = original;
  if (farePath)
    setupTravelData(*farePath);
}

void
BaggageTravel::setupTravelData(FarePath& fp)
{
  _farePath = &fp;
  _itin = fp.itin();
  _paxType = fp.paxType();
}

bool
BaggageTravel::shouldAttachToDisclosure() const
{
  switch (_trx->getBaggagePolicy().getDisclosurePolicy())
  {
  case BaggagePolicy::SELECTED_TRAVELS:
    return _containsSelectedSegment;
  case BaggagePolicy::UNFLOWN_TRAVELS:
    return !TrxUtil::isBaggage302ExchangeActivated(*_trx) || _containsUnflownSegment;
  default:
    return true;
  }
}

const TravelSeg*
BaggageTravel::getFciSeg(bool usDot) const
{
  return usDot ? *itin()->travelSeg().begin() : *_travelSegBegin;
}

void
BaggageTravel::updateSegmentsRange(const TravelSegPtrVecCI& travelSegBegin,
                                   const TravelSegPtrVecCI& travelSegEnd)
{
  _travelSegBegin = travelSegBegin;
  _travelSegEnd = travelSegEnd;
  _containsUnflownSegment = std::any_of(_travelSegBegin, _travelSegEnd, [](const TravelSeg* ts)
  {
    return ts->unflown();
  });
  _containsSelectedSegment = std::any_of(_travelSegBegin, _travelSegEnd, [](const TravelSeg* ts)
  {
    return ts->checkedPortionOfTravelInd() == 'T';
  });
}
} // tse
