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
#include "ItinAnalyzer/SegmentRange.h"

#include <vector>

namespace tse
{
class Itin;
class PricingTrx;
class SOPUsage;
class ClassOfService;

class SoloJourney
{
  friend class SoloJourneyTest;

public:
  SoloJourney(const PricingTrx& trx, const Itin* itin, size_t statIdx);
  void endJourney(size_t endIdx);

  const SegmentRange& getRange() const { return _range; }

  ClassOfServiceList* getFlowAvl(const size_t segIdx, const SOPUsage& sopUsage) const
  {
    return getAvl(_range, segIdx, sopUsage);
  }

  ClassOfServiceList* getLocalAvl(const size_t segIdx, const SOPUsage& sopUsage) const
  {
    return getAvl(SegmentRange(segIdx, segIdx + 1), segIdx, sopUsage);
  }

  ClassOfServiceList*
  getFlowAvl(const SegmentRange& fmRange, const size_t segIdx, const SOPUsage& sopUsage) const
  {
    return getAvl(_range.getIntersection(fmRange), segIdx, sopUsage);
  }

private:
  typedef AvailabilityMap::const_iterator AvlCI;
  typedef ClassOfServiceList COSList;

  COSList* getAvl(const SegmentRange& range, const size_t segIdx, const SOPUsage& sopUsage) const;
  uint64_t buildAvlKey(const SegmentRange& avlRange) const;
  bool isAA13HoursConnection(const size_t segIdx, const SOPUsage& sopUsage) const;

  bool isFlowAvailFromAvs(const SegmentRange& range, const std::vector<COSList>&flowAvl) const;
  size_t calcMinNumLocalClasses(const SegmentRange& range) const;
  float calcAvgNumFlowClasses(const std::vector<COSList>& flowAvl) const;

private:
  const PricingTrx* _trx;
  const Itin* _itin;
  SegmentRange _range;
  float _avsLocalToFlowCosSizeRatio;
};

typedef std::vector<SoloJourney> SoloJourneyVec;
} // namespace tse

