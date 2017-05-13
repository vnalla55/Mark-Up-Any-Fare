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

#include <memory>

namespace tax
{
class CarrierFlight;

class CarrierFlightService
{
public:
  CarrierFlightService() {}
  virtual ~CarrierFlightService() {}

  virtual std::shared_ptr<const CarrierFlight>
  getCarrierFlight(const type::Vendor& vendor, const type::Index& itemNo) const = 0;
};

} // namespace tax
