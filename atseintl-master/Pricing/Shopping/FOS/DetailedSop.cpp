// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
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
#include "Pricing/Shopping/FOS/DetailedSop.h"

#include "Common/Assert.h"
#include "Common/GoverningCarrier.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"

namespace tse
{
namespace fos
{

DetailedSop::DetailedSop(uint32_t legId, uint32_t sopId) : _legId(legId), _sopId(sopId) {}

bool
DetailedSop::
operator<(const DetailedSop& other) const
{
  if (_legId < other.getLegId())
    return true;
  else if (_legId > other.getLegId())
    return false;
  else
    return _sopId < other.getSopId();
}

void
DetailedSop::generateSopDetails(DataHandle& dataHandle,
                                std::vector<TravelSeg*>& trvSegs,
                                uint32_t legId)
{
  if (trvSegs.size() == 1)
  {
    SopDetails* newSopDetails = &dataHandle.safe_create<SopDetails>();
    newSopDetails->cxrCode[0] = calculateGovCarrier(legId, trvSegs, *trvSegs.back());
    _sopDetailsVec.push_back(newSopDetails);
  }
  else
  {
    std::vector<TravelSeg*>::const_iterator itBegin(trvSegs.begin()), it(itBegin + 1),
        itEnd(trvSegs.end());
    for (; it < itEnd; ++it)
    {
      SopDetails* newSopDetails = &dataHandle.safe_create<SopDetails>();
      std::vector<std::vector<TravelSeg*>> fmPairTS(2);
      fmPairTS[0].assign(itBegin, it);
      fmPairTS[1].assign(it, itEnd);
      newSopDetails->destAirport = fmPairTS[0].back()->destAirport();
      for (uint32_t i = 0; i < 2; ++i)
      {
        newSopDetails->cxrCode[i] = calculateGovCarrier(legId, fmPairTS[i], *fmPairTS[1].back());
        std::string key(fmPairTS[i].front()->origAirport() + fmPairTS[i].back()->destAirport());
        newSopDetails->fareMarketOD[i] = key;
      }
      _sopDetailsVec.push_back(newSopDetails);
    }
  }
}

CarrierCode
DetailedSop::calculateGovCarrier(uint32_t legIdx,
                                 const std::vector<TravelSeg*>& tvlSegments,
                                 const TravelSeg& lastSegment) const
{
  if (tvlSegments.size() == 1)
  {
    const AirSeg* airSeg = tvlSegments.front()->toAirSeg();
    if (airSeg && !airSeg->carrier().empty())
      return airSeg->carrier();
    return INDUSTRY_CARRIER;
  }

  GoverningCarrier govCxrUtil;
  CarrierSet govCxrSet;
  FMDirection fmDirection;

  if (1 == legIdx)
    fmDirection = FMDirection::OUTBOUND;
  else if (legIdx > 1 &&
           tvlSegments.back()->destination()->loc() == lastSegment.destination()->loc())
    fmDirection = FMDirection::INBOUND;
  else
    fmDirection = FMDirection::UNKNOWN; // to don't issue compile-time warning

  bool result = govCxrUtil.getGoverningCarrier(tvlSegments, govCxrSet, fmDirection);
  TSE_ASSERT(result);
  return *govCxrSet.begin();
}

} // fos
} // tse
