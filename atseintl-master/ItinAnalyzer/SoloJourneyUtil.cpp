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
#include "ItinAnalyzer/SoloJourneyUtil.h"

#include "Common/TrxUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"

#include <iostream>
#include <stdexcept>

namespace tse
{
//----------------------------------------------------------------------
//----------------------------------------------------------------------
SoloJourneyUtil::SoloJourneyUtil(const PricingTrx& trx, const Itin& itin) : _trx(trx)
{
  CarrierCode lastCarrier;
  SoloJourney* currentJourney = nullptr;

  size_t numSeg = itin.travelSeg().size();
  _journeys.reserve(numSeg);
  _segments.reserve(numSeg);
  for (TravelSeg* tvlSeg : itin.travelSeg())
  {
    const size_t segIdx = _segments.size();
    AirSeg* airSeg = nullptr;
    bool forceFlow = false; // Force flow journey for interline/Intraline

    if (LIKELY(tvlSeg->isAir()))
    {
      airSeg = static_cast<AirSeg*>(tvlSeg);

      if (!airSeg->carrierPref())
      {
        airSeg->carrierPref() =
            trx.dataHandle().getCarrierPreference(airSeg->carrier(), airSeg->departureDT());
      }

      // If current and next segment are from same interline/intraline group
      // we need to force this segment as flow.
      AirSeg* nextSeg = nullptr;
      if (segIdx+1 < numSeg)
        nextSeg = static_cast<AirSeg*>(itin.travelSeg()[segIdx+1]);
      else if (segIdx != 0)
        nextSeg = static_cast<AirSeg*>(itin.travelSeg()[segIdx-1]);

      if (nextSeg)
      {
        if (TrxUtil::intralineAvailabilityApply(trx, lastCarrier, airSeg->marketingCarrierCode()) ||
            TrxUtil::interlineAvailabilityApply(trx, lastCarrier, airSeg->marketingCarrierCode()))
          forceFlow = true;
      }

      if (!currentJourney ||
          ((lastCarrier != airSeg->marketingCarrierCode()) &&
           !TrxUtil::intralineAvailabilityApply(trx, lastCarrier, airSeg->marketingCarrierCode()) &&
           !TrxUtil::interlineAvailabilityApply(trx, lastCarrier, airSeg->marketingCarrierCode())))
      {
        if (currentJourney)
          currentJourney->endJourney(segIdx);

        _journeys.push_back(SoloJourney(trx, &itin, segIdx));
        currentJourney = &_journeys.back();
        lastCarrier = airSeg->marketingCarrierCode();
      }
    }
    _segments.push_back(SoloJourneySeg(_trx, currentJourney, segIdx, airSeg, forceFlow));
  }

  if (LIKELY(currentJourney))
  {
    currentJourney->endJourney(_segments.size());
  }
}

//----------------------------------------------------------------------
//----------------------------------------------------------------------
void
SoloJourneyUtil::fillAvailability(SOPUsage& sopUsage) const
{
  const SegmentRange fmRange(sopUsage.startSegment_, 1 + sopUsage.endSegment_);
  std::vector<ClassOfServiceList*>& avlSolution(sopUsage.cos_);

  if (UNLIKELY(fmRange.getEndIdx() > _segments.size()))
  {
    std::stringstream os;
    os << "SoloJourneyUtil::fillAvailability: invalid SOPUsage range " << fmRange;
    throw std::out_of_range(os.str());
  }

  avlSolution.resize(fmRange.getSize());
  for (size_t segIdx = fmRange.getStartIdx(), avlIdx = 0; segIdx < fmRange.getEndIdx(); ++segIdx)
  {
    avlSolution[avlIdx++] = _segments[segIdx].getAvailability(fmRange, sopUsage);
  }
}

} // namespace tse
