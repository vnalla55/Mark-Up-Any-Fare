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
#include "ItinAnalyzer/SoloJourneySeg.h"

#include "DataModel/AirSeg.h"
#include "ItinAnalyzer/SoloJourney.h"

namespace tse
{
SoloJourneySeg::SoloJourneySeg(const PricingTrx& trx,
                               const SoloJourney* journey,
                               const size_t segIdx,
                               AirSeg* airSeg,
                               bool forceFlow)
  : _journey(journey), _segIdx(segIdx), _airSeg(airSeg)
{
  if (UNLIKELY(airSeg == nullptr)) // is is non air seg
    _avlGetterPtr = &SoloJourneySeg::getNonAirAvl;
  else if (airSeg->flowJourneyCarrier() || forceFlow)
    _avlGetterPtr = &SoloJourneySeg::getFlowCxrAvl;
  else if (airSeg->localJourneyCarrier())
    _avlGetterPtr = &SoloJourneySeg::getLocalCxrAvl;
  else
    _avlGetterPtr = &SoloJourneySeg::getNonJourneyCxrAvl;
}

ClassOfServiceList*
SoloJourneySeg::getAvailability(const SegmentRange& fmRange, const SOPUsage& sopUsage) const
{
  return (this->*_avlGetterPtr)(fmRange, sopUsage);
}

ClassOfServiceList*
SoloJourneySeg::getFlowCxrAvl(const SegmentRange& fmRange, const SOPUsage& sopUsage) const
{
  // For flow journey carrier always use flow availability
  return _journey->getFlowAvl(_segIdx, sopUsage);
}

ClassOfServiceList*
SoloJourneySeg::getLocalCxrAvl(const SegmentRange& fmRange, const SOPUsage& sopUsage) const
{
  // Fare market is superset of journey
  if (fmRange.contains(_journey->getRange()))
    return _journey->getFlowAvl(_segIdx, sopUsage);

  // using merged availability
  return &(_airSeg->classOfService());
}

ClassOfServiceList*
SoloJourneySeg::getNonJourneyCxrAvl(const SegmentRange& fmRange, const SOPUsage& sopUsage) const
{
  // use widest O&D availability
  return _journey->getFlowAvl(fmRange, _segIdx, sopUsage);
}

ClassOfServiceList*
SoloJourneySeg::getNonAirAvl(const SegmentRange& fmRange, const SOPUsage& sopUsage) const
{
  return nullptr;
}
} // namespace tse
