
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

#include "DataModel/Common/Types.h"
#include "DataModel/Services/CarrierFlight.h"
#include "ServiceInterfaces/CarrierFlightService.h"

namespace tax
{
class CarrierFlightServiceServer : public CarrierFlightService
{
public:
  CarrierFlightServiceServer() {}
  virtual ~CarrierFlightServiceServer() {}

  std::shared_ptr<const CarrierFlight>
  getCarrierFlight(const type::Vendor& vendor, const type::Index& itemNo) const final;

  boost::ptr_vector<CarrierFlight>& carrierFlights() { return _carrierFlights; }

  const boost::ptr_vector<CarrierFlight>& carrierFlights() const { return _carrierFlights; };

private:
  boost::ptr_vector<CarrierFlight> _carrierFlights;
};
}
