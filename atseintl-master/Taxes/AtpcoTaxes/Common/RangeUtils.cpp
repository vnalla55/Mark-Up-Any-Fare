// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Common/RangeUtils.h"

namespace tax
{

Range::Range() : begin(0), end(0), empty(true)
{
}

Range::Range(type::Index b, type::Index e) : begin(b), end(e), empty(b >= e)
{
}

bool
Range::operator==(const Range& other) const
{
  if (empty && other.empty)
    return true;

  return begin == other.begin && end == other.end;
}

bool
Range::operator<=(const Range& other) const
{
  if (empty)
    return true;

  return begin >= other.begin && end <= other.end;
}

Range
Range::operator&(const Range& other) const
{
  return Range(std::max(begin, other.begin), std::min(end, other.end));
}

ProperRange::ProperRange(type::Index b, type::Index e) : Range(std::min(b, e), std::max(b, e))
{
}
}
