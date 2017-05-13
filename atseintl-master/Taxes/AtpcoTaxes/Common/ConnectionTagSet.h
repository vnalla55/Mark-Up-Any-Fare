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

struct ConnectionsTagToBitmask
{
  typedef type::ConnectionsTag enum_type;

  static unsigned toBit(enum_type val)
  {
    return 1U << (static_cast<unsigned>(val) - static_cast<unsigned>(type::ConnectionsTag::TurnaroundPointForConnection)); // - 'A'
  }

  static unsigned all()
  {
    static const unsigned ans = toBit(type::ConnectionsTag::TurnaroundPointForConnection) |
                                toBit(type::ConnectionsTag::TurnaroundPoint) |
                                toBit(type::ConnectionsTag::FareBreak) |
                                toBit(type::ConnectionsTag::FurthestFareBreak) |
                                toBit(type::ConnectionsTag::GroundTransport) |
                                toBit(type::ConnectionsTag::DifferentMarketingCarrier) |
                                toBit(type::ConnectionsTag::Multiairport) |
                                toBit(type::ConnectionsTag::DomesticToInternational) |
                                toBit(type::ConnectionsTag::InternationalToDomestic) |
                                toBit(type::ConnectionsTag::InternationalToInternational);
    return ans;
  }
};

class ConnectionTagSet : public TagSet<ConnectionsTagToBitmask>
{
public:
  ConnectionTagSet(TagSet<ConnectionsTagToBitmask> const& s) : TagSet<ConnectionsTagToBitmask>(s) {}
};

} // namespace tax

