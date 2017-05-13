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
#pragma once
#include "Common/TagSet.h"
#include "DataModel/Common/SafeEnums.h"

namespace tax
{

struct ProcessingGroupTagToBitmask
{
  typedef type::ProcessingGroup enum_type;

  static unsigned toBit(enum_type val)
  {
    return 1U << static_cast<unsigned>(val);
  }

  static unsigned all()
  {
    static const unsigned ans = toBit(type::ProcessingGroup::OC) |
                                toBit(type::ProcessingGroup::OB) |
                                toBit(type::ProcessingGroup::ChangeFee) |
                                toBit(type::ProcessingGroup::Itinerary);
    return ans;
  }
};

typedef TagSet<ProcessingGroupTagToBitmask> ProcessingGroupTagSet;

} // namespace tax

