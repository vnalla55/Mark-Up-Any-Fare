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
#include "ItinAnalyzer/SegmentRange.h"

namespace tse
{
std::ostream& operator<<(std::ostream& out, const SegmentRange& range)
{
  out << "[" << range.getStartIdx() << ", " << range.getEndIdx() << ")";
  return out;
}
} // namespace tse
