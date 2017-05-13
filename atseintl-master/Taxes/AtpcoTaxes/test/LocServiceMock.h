#pragma once

#include "AtpcoTaxes/Common/LocZone.h"
#include "ServiceInterfaces/LocService.h"
#include <list>

namespace tax
{

class LocServiceMock : public LocService
{
public:
  LocServiceMock& clear();
  LocServiceMock();
  LocServiceMock& add(bool result, type::Index count = 1);
  bool isInLoc(const type::AirportOrCityCode& airportCode, const LocZone& jrnyLoc1LocZone,
               const type::Vendor& vendor) const;

  void setNation(const type::Nation& nation);
  type::Nation getNation(const type::AirportCode& loc) const;
  type::Nation getNationByName(const type::NationName& /*nationName*/) const
  {
    return _nation;
  }

  void setNationName(const type::NationName& nationName)
  {
    _nationName = nationName;
  }

  type::NationName getNationName(const type::Nation& /*nationCode*/) const
  {
    return _nationName;
  }

  void setCityCode(const type::CityCode& cityCode);
  type::CityCode getCityCode(const type::AirportCode& loc) const;

  type::LocCode getLocCode(const type::Index&) const
  {
    return "";
  }

  type::AlaskaZone getAlaskaZone(const type::AirportCode&) const
  {
    return tax::type::AlaskaZone::Blank;
  }

  type::StateProvinceCode getState(const type::AirportCode&) const
  {
    return tax::type::StateProvinceCode();
  }

  void setCurrency(const type::CurrencyCode& currency);
  type::CurrencyCode getCurrency(const type::AirportCode&) const;

  bool matchPassengerLocation(const type::LocCode& loc1,
                              const LocZone& loc2,
                              const type::Vendor&) const
  {
    return loc1 == loc2.code().asString();
  }

  type::Index counter() const
  {
    return _counter;
  }

  bool empty() const
  {
    return _results.empty();
  }

private:
  mutable std::list<bool> _results;
  type::Nation _nation;
  type::NationName _nationName;
  type::CityCode _cityCode;
  type::CurrencyCode _currency;
  mutable type::Index _counter;
};

} // namespace tax

