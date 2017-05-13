// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include <boost/ptr_container/ptr_vector.hpp>
#include "Common/LocZone.h"
#include "DataModel/Common/Types.h"
#include "DataModel/Services/SubCache.h"

namespace tax
{

struct PassengerTypeCodeItem
{
  type::PtcApplTag applTag {type::PtcApplTag::Blank};
  type::PassengerCode passengerType {UninitializedCode};
  int16_t minimumAge {-1};
  int16_t maximumAge {-1};
  type::PassengerStatusTag statusTag {type::PassengerStatusTag::Blank};
  LocZone location {type::LocType::Nation};
  type::PtcMatchIndicator matchIndicator {type::PtcMatchIndicator::Input};
};

typedef boost::ptr_vector<PassengerTypeCodeItem> PassengerTypeCodeItems;
typedef CacheItem<PassengerTypeCodeItem> PassengerTypeCode;

} // namespace tax

