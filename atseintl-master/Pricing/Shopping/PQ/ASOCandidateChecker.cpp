// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#include "Pricing/Shopping/PQ/ASOCandidateChecker.h"

#include "Common/Assert.h"
#include "Common/Config/ConfigMan.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/Logger.h"
#include "Common/TseUtil.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TravelSeg.h"

#include <algorithm>

namespace tse
{
namespace shpq
{

namespace
{
ConfigurableValue<uint32_t>
asoCandidateMileage("SHOPPING_DIVERSITY", "ASO_CANDIDATE_MILEAGE", 4000);
}

Logger
ASOCandidateChecker::_logger("atseintl.ShoppingPQ");

ASOCandidateChecker::ASOCandidateChecker(ShoppingTrx& trx)
  : _trx(trx), _asoCandidateMileage(asoCandidateMileage.getValue())
{
}

bool
ASOCandidateChecker::match(std::vector<int> sops) const
{
  TSE_ASSERT(sops.size() == _trx.legs().size());

  if (sops.size() < 2)
    return false;

  std::vector<TravelSeg*>& tvlSegsOut = _trx.legs()[0].sop()[sops[0]].itin()->travelSeg();

  if (checkDistance(*tvlSegsOut.front()->origin(), *tvlSegsOut.front()->destination()) &&
      isDiamond(sops))
      return true;

  if (isLimitHintingWithinZ021ForDomCncxTurnAround(sops))
    return true;

  return false;
}

bool
ASOCandidateChecker::checkDistance(const Loc& loc1, const Loc& loc2) const
{
  if (getMileage(loc1, loc2) > _asoCandidateMileage)
    return true;
  return false;
}

uint32_t
ASOCandidateChecker::getMileage(const Loc& loc1, const Loc& loc2) const
{
  return TseUtil::greatCircleMiles(loc1, loc2);
}

bool
ASOCandidateChecker::checkLegsOverlap(std::vector<int> sops) const
{
  std::vector<TravelSeg*>& tvlSegsOut = _trx.legs()[0].sop()[sops[0]].itin()->travelSeg();
  std::vector<TravelSeg*>& tvlSegsIn = _trx.legs()[1].sop()[sops[1]].itin()->travelSeg();

  std::set<LocCode> obIntermediatePoints = collectIntermediatePoints(tvlSegsOut);
  std::set<LocCode> inIntermediatePoints = collectIntermediatePoints(tvlSegsIn);

  /// Check if at least two segments in each direction
  if (obIntermediatePoints.empty() || inIntermediatePoints.empty())
    return false;

  std::set<LocCode> commonPoints;
  std::set_intersection(obIntermediatePoints.begin(),
                        obIntermediatePoints.end(),
                        inIntermediatePoints.begin(),
                        inIntermediatePoints.end(),
                        std::inserter(commonPoints, commonPoints.begin()));

  return !commonPoints.empty();
}

std::set<LocCode>
ASOCandidateChecker::collectIntermediatePoints(std::vector<TravelSeg*>& segments) const
{
  std::set<LocCode> intermediatePoints;

  std::vector<TravelSeg*>::const_iterator segmentIt = segments.begin();
  for (; segmentIt != segments.end(); ++segmentIt)
  {
    if (segmentIt + 1 == segments.end())
      break;
    intermediatePoints.insert((*segmentIt)->offMultiCity());
  }
  return intermediatePoints;
}

bool
ASOCandidateChecker::isDiamond(std::vector<int> sops) const
{
  return !checkLegsOverlap(sops);
}

bool
ASOCandidateChecker::isLimitHintingWithinZ021ForDomCncxTurnAround(const std::vector<int>& sops) const
{
  // Ensure at least one leg must have connection flights.
  const std::vector<TravelSeg*>& tvlSegsOut = _trx.legs()[0].sop()[sops[0]].itin()->travelSeg();
  const std::vector<TravelSeg*>& tvlSegsIn = _trx.legs()[1].sop()[sops[1]].itin()->travelSeg();

  if ((tvlSegsOut.size() < 2) && (tvlSegsIn.size() < 2))
  {
    return false;
  }

  // Get subareas of origin and destination
  // to ensure they are both witinin Europe
  const Loc* originOut = tvlSegsOut.front()->origin();
  const Loc* destinationOut = tvlSegsOut[tvlSegsOut.size()-1]->destination();
  const IATASubAreaCode& originOutSA = originOut->subarea();
  const IATASubAreaCode& destinOutSA = destinationOut->subarea();

  if ((originOutSA != IATA_SUB_AREA_21()) || (destinOutSA != IATA_SUB_AREA_21()))
  {
    return false;
  }

  // Ensure the destination and its adjacent connections
  // are within the same country
  // Outbound
  if ((tvlSegsOut.size() >= 2) && // have connections outbound
      (destinationOut->nation() != tvlSegsOut[tvlSegsOut.size()-1]->origin()->nation()))
  {
    return false;
  }

  // Inbound
  if ((tvlSegsIn.size() >= 2) && // have connections inbound
      (tvlSegsIn.front()->origin()->nation() != tvlSegsIn.front()->destination()->nation()))
  {
    return false;
  }

  return true;
}

}
} // namespace tse::shpq
