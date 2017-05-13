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
#pragma once

#include "Common/TseStlTypes.h"

#include <vector>

namespace tse
{
class AirSeg;
class PricingTrx;
class SegmentRange;
class SoloJourney;
class SOPUsage;

class SoloJourneySeg
{
public:
  SoloJourneySeg(const PricingTrx& trx,
                 const SoloJourney* journey,
                 const size_t segIdx,
                 AirSeg* airSeg,
                 bool forceFlow = false);

  ClassOfServiceList*
  getAvailability(const SegmentRange& fmRange, const SOPUsage& sopUsage) const;

private:
  typedef ClassOfServiceList* (SoloJourneySeg::*AvlGetter)(
      const SegmentRange& fmRange, const SOPUsage& sopUsage) const;

  ClassOfServiceList*
  getFlowCxrAvl(const SegmentRange& fmRange, const SOPUsage& sopUsage) const;
  ClassOfServiceList*
  getLocalCxrAvl(const SegmentRange& fmRange, const SOPUsage& sopUsage) const;
  ClassOfServiceList*
  getNonJourneyCxrAvl(const SegmentRange& fmRange, const SOPUsage& sopUsage) const;
  ClassOfServiceList*
  getNonAirAvl(const SegmentRange& fmRange, const SOPUsage& sopUsage) const;

  const SoloJourney* _journey;
  size_t _segIdx;
  AvlGetter _avlGetterPtr;
  AirSeg* _airSeg;
};

typedef std::vector<SoloJourneySeg> SoloJourneySegVec;
} // namespace tse

