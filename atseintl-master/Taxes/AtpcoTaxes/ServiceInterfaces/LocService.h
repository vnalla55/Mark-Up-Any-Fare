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
#include "DataModel/Services/Nation.h"

namespace tax
{
class LocZone;

class LocService
{
public:
  LocService(void) {}
  virtual ~LocService(void) {}

  virtual type::Nation getNation(const type::AirportCode& loc) const = 0;
  virtual type::Nation getNationByName(const type::NationName& nationName) const = 0;
  virtual type::NationName getNationName(const type::Nation& nationCode) const = 0;
  virtual type::CityCode getCityCode(const type::AirportCode& loc) const = 0;
  virtual type::AlaskaZone getAlaskaZone(const type::AirportCode& loc) const = 0;
  virtual type::StateProvinceCode getState(const type::AirportCode& loc) const = 0;
  virtual type::CurrencyCode getCurrency(const type::AirportCode& loc) const = 0;

  virtual bool isInLoc(const type::AirportOrCityCode& airportCode,
                       const LocZone& jrnyLoc1LocZone,
                       const type::Vendor& vendor) const = 0;

  virtual bool matchPassengerLocation(const tax::type::LocCode& /*passengerLocation*/,
                                      const tax::LocZone& /*requiredLocation*/,
                                      const tax::type::Vendor& /*vendor*/) const = 0;
};
}

