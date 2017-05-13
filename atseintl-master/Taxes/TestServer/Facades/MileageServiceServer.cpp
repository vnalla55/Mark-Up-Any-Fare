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
#include <stdexcept>


#include "DomainDataObjects/GeoPath.h"
#include "MileageServiceServer.h"

namespace tax
{

const MileageGetter&
MileageServiceServer::getMileageGetter(const GeoPath& geoPath,
                                       const std::vector<FlightUsage>&,
                                       const type::Timestamp&) const
{
  for(const MileageGetterServer & mileage : _mileages)
  {
    if (geoPath.id() == mileage.geoPathRefId())
    {
      return mileage;
    }
  }

  throw std::runtime_error("MileageServiceServer::getMiles() - Mileage for geoPath not found!");
}

} // namespace tax
