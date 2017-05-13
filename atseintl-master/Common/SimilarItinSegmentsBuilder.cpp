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
//-------------------------------------------------------------------

#include "Common/SimilarItinSegmentsBuilder.h"

#include "Common/FallbackUtil.h"
#include "Common/LocUtil.h"
#include "DataModel/Itin.h"
#include "DataModel/TravelSeg.h"
#include "DataModel/PricingTrx.h"

namespace tse
{
FALLBACK_DECL(familyLogicArunk)

namespace similaritin
{
namespace
{
SegmentsBuilder::ItinTSVec
constructItinTSVecPerLeg(const Itin& itin)
{
  SegmentsBuilder::ItinTSVec result;
  int16_t currentLeg = 0;
  SegmentsBuilder::TSVec legTSVec;
  for (TravelSeg* const segment : itin.travelSeg())
  {
    if (segment->legId() != currentLeg)
    {
      result.push_back(legTSVec);
      legTSVec.clear();
      ++currentLeg;
    }
    legTSVec.push_back(segment);
  }
  result.push_back(legTSVec);
  return result;
}

bool
isSameOrigin(const TravelSeg& seg1, const TravelSeg& seg2)
{
  return LocUtil::isSamePoint(
      *(seg1.origin()), seg1.boardMultiCity(), *(seg2.origin()), seg2.boardMultiCity());
}

bool
isSameDest(const TravelSeg& seg1, const TravelSeg& seg2)
{
  return LocUtil::isSamePoint(
      *(seg1.destination()), seg1.offMultiCity(), *(seg2.destination()), seg2.offMultiCity());
}
}

SegmentsBuilder::SegmentsBuilder(const PricingTrx& trx, const Itin& mother, const Itin& similar)
  : _itinTSVec(constructItinTSVecPerLeg(similar)),
    _motherItin(mother),
    _similarItin(similar),
    _similarItinGeoConsistent(mother.isGeoConsistent(similar))
{
}

SegmentsBuilder::TSVec
SegmentsBuilder::constructByOriginAndDestination(const TSVec& originalVec) const
{
  TSVec constructedVec;

  const TravelSeg* boardSeg = originalVec.front();
  const TravelSeg* offSeg = originalVec.back();
  int16_t boardLeg = boardSeg->legId();
  int16_t offLeg = offSeg->legId();

  if (_similarItinGeoConsistent)
  {
    for (const auto elem : originalVec)
    {
      const int index = _motherItin.segmentOrder(elem) - 1;
      TSE_ASSERT(index >= 0 && index < (int)_similarItin.travelSeg().size());
      constructedVec.push_back(_similarItin.travelSeg()[index]);
    }

    return constructedVec;
  }

  typedef std::vector<TravelSeg*>::const_iterator CI;
  CI boardIt = std::find_if(_itinTSVec[boardLeg].begin(),
                            _itinTSVec[boardLeg].end(),
                            [&boardSeg](const TravelSeg* segment) -> bool
                            { return isSameOrigin(*boardSeg, *segment); });

  if (boardIt == _itinTSVec[boardLeg].end())
    return TSVec();

  CI offIt = std::find_if((boardLeg == offLeg ? boardIt : _itinTSVec[offLeg].begin()),
                          _itinTSVec[offLeg].end(),
                          [&offSeg](const TravelSeg* segment) -> bool
                          { return isSameDest(*offSeg, *segment); });

  if (offIt == _itinTSVec[offLeg].end())
    return TSVec();

  auto append = [&constructedVec](const CI& begin, const CI& end)
  { constructedVec.insert(constructedVec.end(), begin, end); };

  ++offIt;
  append(boardIt, (boardLeg == offLeg ? offIt : _itinTSVec[boardLeg].end()));

  for (uint16_t currentLeg = boardLeg + 1; currentLeg < offLeg; ++currentLeg)
    append(_itinTSVec[currentLeg].begin(), _itinTSVec[currentLeg].end());

  if (boardLeg != offLeg)
    append(_itinTSVec[offLeg].begin(), offIt);

  for (; offIt != _itinTSVec[offLeg].end() && isSameDest(*offSeg, **offIt); ++offIt)
    constructedVec.push_back(*offIt);

  return constructedVec;
}
}
}
