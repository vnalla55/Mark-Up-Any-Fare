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

#include "Pricing/Shopping/PQ/SoloFlightOnlySolutionsSopCollector.h"

#include "Common/Assert.h"
#include "Common/FallbackUtil.h"
#include "Common/Global.h"
#include "Common/GoverningCarrier.h"
#include "Common/ShoppingAltDateUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TseConsts.h"
#include "DataModel/FareMarket.h"

#include <iterator>
#include <set>

namespace tse
{


namespace fos
{

void
SoloFlightOnlySolutionsSopCollector::collectApplicableSopsAltDates(SOPCollections& collection)
{
  collection.clear();
  collection.getSopsByDate().resize(_trx.legs().size());
  collection.getSopsByLegDetails().resize(_trx.legs().size());

  for (size_t legIdx = 0; legIdx < _trx.legs().size(); ++legIdx)
  {
    ShoppingTrx::Leg& leg = _trx.legs()[legIdx];

    if (leg.stopOverLegFlag())
      continue;

    for (size_t sopIdx = 0; sopIdx < leg.sop().size(); ++sopIdx)
    {
      ShoppingTrx::SchedulingOption& sop = leg.sop()[sopIdx];
      if (sop.cabinClassValid())
      {
        processSopAltDates(collection, legIdx, sopIdx, sop);
      }
    }
  }
}

void
SoloFlightOnlySolutionsSopCollector::processSopAltDates(SOPCollections& collection,
                                                        uint32_t legIdx,
                                                        uint32_t sopIdx,
                                                        ShoppingTrx::SchedulingOption& sop)
{
  std::vector<TravelSeg*>& tvlSegs = sop.itin()->travelSeg();
  const DateTime& date = ShoppingAltDateUtil::dateOnly((*tvlSegs.begin())->departureDT());

  std::vector<TravelSeg*>::const_iterator itBegin(tvlSegs.begin()), it(itBegin + 1), itEnd(tvlSegs.end());

  if (tvlSegs.size() == 1)
  {
    addSop(collection,
           legIdx,
           sopIdx,
           date,
           sop.governingCarrier(),
           createCandidate(legIdx, tvlSegs.front()));
  }
  else
  {
    for (; it < itEnd; ++it)
    {
      std::vector<std::vector<TravelSeg*>> fmPairTS(2);
      fmPairTS[0].assign(itBegin, it);
      fmPairTS[1].assign(it, itEnd);

      addSop(collection,
             legIdx,
             sopIdx,
             date,
             sop.governingCarrier(),
             createCandidate(legIdx, fmPairTS));
    }
  }
}

CarrierCode
SoloFlightOnlySolutionsSopCollector::calculateGovCarrier(uint32_t legIdx,
                                                         std::vector<TravelSeg*>& tvlSegments,
                                                         TravelSeg* lastSegment)
{
  if (tvlSegments.size() == 1)
  {
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(tvlSegments.front());
    if (airSeg)
      return (airSeg->carrier().empty() ? INDUSTRY_CARRIER : airSeg->carrier());
    return INDUSTRY_CARRIER;
  }

  GoverningCarrier govCxrUtil(&_trx);
  CarrierSet govCxrSet;
  FMDirection fmDirection;

  if (1 == legIdx)
    fmDirection = FMDirection::OUTBOUND;
  else if (legIdx > 1 &&
           tvlSegments.back()->destination()->loc() == lastSegment->destination()->loc())
    fmDirection = FMDirection::INBOUND;
  else
    fmDirection = FMDirection::UNKNOWN; // to don't issue compile-time warning

  bool result = govCxrUtil.getGoverningCarrier(tvlSegments, govCxrSet, fmDirection);
  TSE_ASSERT(result);
  return *govCxrSet.begin();
}

void
SoloFlightOnlySolutionsSopCollector::addSop(SOPCollections& collection,
                                            uint32_t legIdx,
                                            uint32_t sopIdx,
                                            const DateTime& date,
                                            const CarrierCode& sopGovCarrier,
                                            SOPDetails details)
{
  SOPWrapper sop(sopIdx);
  SOPMap::iterator sopIt = (collection.getSopsByLegDetails())[legIdx].find(sop);
  if (sopIt == (collection.getSopsByLegDetails())[legIdx].end())
  {
    sopIt = (collection.getSopsByLegDetails())[legIdx]
                .insert(std::make_pair(sop, SOPDetailsVec()))
                .first;
  }
  sopIt->second.push_back(details);

  if (sopIt->second.size() == 1)
  {
    SopsByDate& sopsByDate = collection.getSopsByDate()[legIdx];
    sopsByDate[date].push_back(sop);

    SopsByLegByCxrByDate& sopsByCxr = collection.getSopsByLegByCxrByDate();
    SopsByLegByCxrByDate::iterator cxrIt = sopsByCxr.find(sopGovCarrier);
    if (cxrIt == sopsByCxr.end())
    {
      cxrIt = sopsByCxr.insert(std::make_pair(sopGovCarrier, SopsByLegByDate(_trx.legs().size())))
                  .first;
    }

    cxrIt->second[legIdx][date].push_back(sop);
  }
}

SOPDetails
SoloFlightOnlySolutionsSopCollector::createCandidate(uint32_t legIdx,
                                                     std::vector<std::vector<TravelSeg*>>& tvlSegments)
{
  SOPDetails details;

  details._tvlSegPortions[0] = tvlSegments[0];
  details._tvlSegPortions[1] = tvlSegments[1];
  details._cxr[0] = calculateGovCarrier(legIdx, tvlSegments[0], tvlSegments[1].back());
  details._cxr[1] = calculateGovCarrier(legIdx, tvlSegments[1], tvlSegments[1].back());

  return details;
}

SOPDetails
SoloFlightOnlySolutionsSopCollector::createCandidate(uint32_t legIdx, TravelSeg* tvlSegment)
{
  SOPDetails details;

  std::vector<TravelSeg*> tvlSegVec;
  tvlSegVec.push_back(tvlSegment);

  details._tvlSegPortions[0] = tvlSegVec;
  details._cxr[0] = calculateGovCarrier(legIdx, tvlSegVec, tvlSegVec.back());

  return details;
}

} // namespace fos
} // namespace tse
