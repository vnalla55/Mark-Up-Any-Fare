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

#include <vector>
#include <boost/ptr_container/ptr_vector.hpp>

#include "AtpcoTaxes/DomainDataObjects/MileageGetterServer.h"
#include "AtpcoTaxes/ServiceInterfaces/MileageService.h"

namespace tax
{

class GeoPath;

class MileageServiceServer : public MileageService
{
public:
  const MileageGetter& getMileageGetter(const GeoPath& geoPath,
                                        const std::vector<FlightUsage>& flightUsages,
                                        const type::Timestamp&) const;

  boost::ptr_vector<MileageGetterServer>& mileages()
  {
    return _mileages;
  }

  const boost::ptr_vector<MileageGetterServer>& mileages() const
  {
    return _mileages;
  };

private:
  boost::ptr_vector<MileageGetterServer> _mileages;
};

} // namespace tax
