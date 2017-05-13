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
#include "test/LocServiceMock.h"

#include <stdexcept>

namespace tax
{

LocServiceMock::LocServiceMock()
  :_results(),
   _nation(),
   _cityCode(),
   _currency(),
   _counter(0)
{
}

LocServiceMock& LocServiceMock::clear()
{
  _results.clear();
  _counter = 0;
  return *this;
}

LocServiceMock& LocServiceMock::add(bool result, type::Index count)
{
  _counter = 0;
  for (type::Index idx = 0; idx < count; ++idx)
    _results.push_back(result);
  return *this;
}

bool LocServiceMock::isInLoc(const type::AirportOrCityCode& /*airportCode*/, const LocZone& /*jrnyLoc1LocZone*/,
                             const type::Vendor& /*vendor*/) const
{
  _counter++;
  if (!_results.empty()) {
    bool result = _results.front();
    _results.pop_front();
    return result;
  }
  else
    throw std::runtime_error("LocService::isInLoc() should not be called!");
}

void LocServiceMock::setNation(const type::Nation& nation)
{
  _nation = nation;
}

type::Nation LocServiceMock::getNation(const type::AirportCode& /*loc*/) const
{
  _counter++;
  return _nation;
}

void LocServiceMock::setCityCode(const type::CityCode& cityCode)
{
  _cityCode = cityCode;
}

type::CityCode LocServiceMock::getCityCode(const type::AirportCode& /*loc*/) const
{
  _counter++;
  return _cityCode;
}

void LocServiceMock::setCurrency(const type::CurrencyCode& currency)
{
  _currency = currency;
}

type::CurrencyCode LocServiceMock::getCurrency(const type::AirportCode& /*loc*/) const
{
  _counter++;
  return _currency;
}

} // namespace tax
