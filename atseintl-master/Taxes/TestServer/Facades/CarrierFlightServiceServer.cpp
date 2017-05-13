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

#include "CarrierFlightServiceServer.h"

#include <sstream>

namespace tax
{
typedef std::shared_ptr<const CarrierFlight> SharedConstValue;

SharedConstValue
CarrierFlightServiceServer::getCarrierFlight(const type::Vendor& vendor, const type::Index& itemNo) const
{
  for (const CarrierFlight& cf : _carrierFlights)
  {
    if (cf.vendor == vendor && cf.itemNo == itemNo)
    {
      return SharedConstValue(new CarrierFlight(cf));
    }
  }
  return SharedConstValue();
}

} // namespace tax
