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

class Nation
{
public:
  Nation(void) : _id(0), _alaskaZone(type::AlaskaZone::Blank) {}
  ~Nation(void) {}

  const type::Index& id() const { return _id; }

  type::Index& id() { return _id; }

  type::Nation& nationCode() { return _nationCode; }

  const type::Nation& nationCode() const { return _nationCode; }

  type::AirportCode& locCode() { return _locCode; }

  const type::AirportCode& locCode() const { return _locCode; }

  type::CityCode& cityCode() { return _cityCode; }

  const type::CityCode& cityCode() const { return _cityCode; }

  type::AlaskaZone& alaskaZone() { return _alaskaZone; }

  const type::AlaskaZone& alaskaZone() const { return _alaskaZone; }

  type::StateProvinceCode& state() { return _state; }

  const type::StateProvinceCode& state() const { return _state; }

  type::CurrencyCode& currency() { return _currency; }

  const type::CurrencyCode& currency() const { return _currency; }

private:
  type::Index _id;
  type::Nation _nationCode;
  type::AirportCode _locCode;
  type::CityCode _cityCode;
  type::AlaskaZone _alaskaZone;
  type::StateProvinceCode _state;
  type::CurrencyCode _currency;
};
}

