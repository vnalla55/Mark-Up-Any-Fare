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

#include "DataModel/Common/Types.h"

namespace tax
{

struct OutputOptionalServiceDetails
{
  OutputOptionalServiceDetails() : optionalServiceType(type::OptionalServiceTag::Blank) {}

  bool operator==(const OutputOptionalServiceDetails& r) const
  {
    return (subCode == r.subCode && optionalServiceType == r.optionalServiceType &&
            serviceGroup == r.serviceGroup && serviceSubGroup == r.serviceSubGroup &&
            amount == r.amount && ownerCarrier == r.ownerCarrier &&
            pointOfDeliveryLoc == r.pointOfDeliveryLoc);
  }

  type::OcSubCode subCode;
  type::ServiceGroupCode serviceGroup;
  type::ServiceGroupCode serviceSubGroup;
  type::OptionalServiceTag optionalServiceType;
  type::MoneyAmount amount;
  type::CarrierCode ownerCarrier;
  type::AirportCode pointOfDeliveryLoc;
};

} // namespace tax

