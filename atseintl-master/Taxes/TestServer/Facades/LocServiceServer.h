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
#include "AtpcoTaxes/DataModel/Services/Nation.h"
#include "AtpcoTaxes/DataModel/Services/IsInLoc.h"
#include "AtpcoTaxes/ServiceInterfaces/LocService.h"
#include "DataModel/Common/Types.h"

namespace tax
{
class LocZone;

class LocServiceServer : public LocService
{
public:
  LocServiceServer(void)
  {
  }
  virtual ~LocServiceServer(void)
  {
  }

  type::Nation getNation(const type::AirportCode& loc) const;
  type::Nation getNationByName(const type::NationName& /*nationName*/) const;
  type::NationName getNationName(const type::Nation& /*nationCode*/) const;
  type::CityCode getCityCode(const type::AirportCode& loc) const;
  type::AlaskaZone getAlaskaZone(const type::AirportCode& loc) const;
  type::StateProvinceCode getState(const type::AirportCode& loc) const;
  type::CurrencyCode getCurrency(const type::AirportCode& loc) const;

  boost::ptr_vector<Nation>& nations()
  {
    return _nations;
  }

  const boost::ptr_vector<IsInLoc>& isInLocs() const
  {
    return _isInLocs;
  }

  boost::ptr_vector<IsInLoc>& isInLocs()
  {
    return _isInLocs;
  }

  bool isInLoc(const type::AirportOrCityCode& airportCode, const LocZone& jrnyLoc1LocZone,
               const type::Vendor& vendor) const;

  virtual bool matchPassengerLocation(const tax::type::LocCode& /*passengerLocation*/,
                                      const tax::LocZone& /*requiredLocation*/,
                                      const tax::type::Vendor& /*vendor*/) const;

private:
  boost::ptr_vector<Nation> _nations;
  boost::ptr_vector<IsInLoc> _isInLocs;
};
}

