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

#include <algorithm>
#include <iostream>
#include "Util/BranchPrediction.h"

namespace tse
{

// it is left closed and right open range.
class SegmentRange
{
public:
  SegmentRange(size_t startIdx, size_t endIdx) : _startIdx(startIdx), _endIdx(endIdx) {}

  size_t getSize() const { return _endIdx - _startIdx; }
  size_t getStartIdx() const { return _startIdx; }
  size_t getEndIdx() const { return _endIdx; }

  bool operator==(const SegmentRange& other) const
  {
    return (_startIdx == other._startIdx) && (_endIdx == other._endIdx);
  }

  bool contains(const SegmentRange& other) const
  {
    return (_startIdx <= other._startIdx) && (other._endIdx <= _endIdx);
  }

  bool intersects(const SegmentRange& other) const
  {
    return (_startIdx < other._endIdx) && (other._startIdx < _endIdx);
  }

  SegmentRange getIntersection(const SegmentRange& other) const
  {
    size_t startIdx = 0;
    size_t endIdx = 0;
    if (LIKELY(intersects(other)))
    {
      startIdx = std::max(_startIdx, other._startIdx);
      endIdx = std::min(_endIdx, other._endIdx);
    }
    return SegmentRange(startIdx, endIdx);
  }

private:
  size_t _startIdx;
  size_t _endIdx;
};

std::ostream& operator<<(std::ostream& out, const SegmentRange& range);
} // namespace tse

