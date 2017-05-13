// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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
#include "Common/SafeEnumToString.h"
#include "Common/LocZone.h"
#include "Diagnostic/LoggingLocService.h"
#include "DataModel/Common/CodeIO.h"
#include "DataModel/Common/CompactOptional.h"
#include "DataModel/Common/OptionalCode.h"

#include <boost/format.hpp>
#include <cassert>
#include <map>
#include <set>
#include <sstream>

namespace tax
{

template <>
struct CompactOptionalTraits<type::AlaskaZone>
{
  static type::AlaskaZone singularValue() { return type::AlaskaZone::Blank; }
  static bool isSingularValue (const type::AlaskaZone& v) { return v == type::AlaskaZone::Blank; }
};

// TODO: StateProvince to be changed to Code
template <>
struct CompactOptionalTraits<std::string>
{
  static std::string singularValue() { return std::string(1, '^'); }
  static bool isSingularValue (const std::string& v) { return v == "^"; }
};

namespace
{

struct LocZoneKey
{
  LocZone locZone;
  type::Vendor vendor;

  friend bool operator<(const LocZoneKey& l, const LocZoneKey& r)
  {
    return std::tie(l.locZone.type(), l.locZone.code(), l.vendor)
         < std::tie(r.locZone.type(), r.locZone.code(), r.vendor);
  }
};

struct LocZoneCheck
{
  type::AirportOrCityCode airport;
  bool positive;

  friend bool operator<(const LocZoneCheck& l, const LocZoneCheck& r)
  {
    return l.airport < r.airport;
  }
};

struct PaxLocZoneCheck
{
  type::LocCode locCode;
  bool positive;

  friend bool operator<(const PaxLocZoneCheck& l, const PaxLocZoneCheck& r)
  {
    return l.locCode < r.locCode;
  }
};

struct LocZoneData
{
  std::set<LocZoneCheck> portLocations;
  std::set<PaxLocZoneCheck> paxLocations;
};

typedef std::map<LocZoneKey, LocZoneData> LocZoneCache;

struct AirportProperties
{
  CompactOptional<type::Nation> nation;
  CompactOptional<type::CityCode> cityCode;
  CompactOptional<type::AlaskaZone> alaskaZone;
  CompactOptional<type::StateProvinceCode> state;
  CompactOptional<type::CurrencyCode> currency;
};

std::string stringizeTheMap(const std::map<type::AirportCode, AirportProperties>& map)
{
  using boost::format;
  std::ostringstream ans;

  for (auto& rec : map)
  {
    ans << format("<Loc LocCode=\"%1%\" ") % rec.first;

    if (rec.second.nation.has_value()) ans << format("NationCode=\"%1%\" ") % rec.second.nation.value();
    if (rec.second.cityCode.has_value()) ans << format("CityCode=\"%1%\" ") % rec.second.cityCode.value();
    if (rec.second.alaskaZone.has_value()) ans << format("AlaskaZone=\"%1%\" ") % rec.second.alaskaZone.value();
    if (rec.second.state.has_value()) ans << format("State=\"%1%\" ") % rec.second.state.value();
    if (rec.second.currency.has_value()) ans << format("Currency=\"%1%\" ") % rec.second.currency.value();

    ans << "/>\n";
  }

  return ans.str();
}

std::string toXml(const LocZoneCache& locZoneCache)
{
  using boost::format;
  std::ostringstream ans;

  for (const auto& zone : locZoneCache)
  {
    ans << format("<IsInLoc LocCode=\"%1%\" LocType=\"%2%\" Vendor=\"%3%\" IsPartial=\"1\">\n")
         % zone.first.locZone.code()
         % zone.first.locZone.type()
         % zone.first.vendor;

    for (const LocZoneCheck& portCheck : zone.second.portLocations)
    {
      ans << format("  <LocUsage LocCode=\"%1%\" Included=\"%2%\" />\n")
           % portCheck.airport
           % (portCheck.positive ? 1 : 0);
    }
    for (const PaxLocZoneCheck& paxCheck : zone.second.paxLocations)
    {
      ans << format("  <PaxLocUsage LocCode=\"%1%\" Included=\"%2%\" />\n")
           % paxCheck.locCode
           % (paxCheck.positive ? 1 : 0);
    }
    ans << "</IsInLoc>\n";
  }
  return ans.str();
}

} // anonymous namespace

class LoggingLocService::Impl
{
  std::unique_ptr<LocService> _base;
  mutable std::map<type::AirportCode, AirportProperties> _airportProperties;
  mutable LocZoneCache _locZoneCache;

  friend class LoggingLocService;

public:
  Impl(std::unique_ptr<LocService> base) : _base{std::move(base)} {}
};

LoggingLocService::LoggingLocService(std::unique_ptr<BaseService> base)
: _impl {new Impl{std::move(base)}}
{
}

LoggingLocService::~LoggingLocService() {}

const LocService&
LoggingLocService::base() const
{
  assert (_impl);
  assert (_impl->_base);
  return *_impl->_base;
}

const LoggingLocService::Impl& LoggingLocService::impl() const
{
  assert (_impl);
  return *_impl;
}

LoggingLocService::Impl& LoggingLocService::impl()
{
  assert (_impl);
  return *_impl;
}

type::Nation
LoggingLocService::getNationByName(const type::NationName& nationName) const
{
  type::Nation ans =  base().getNationByName(nationName);
  // TODO: cache this also
  return ans;
}

type::NationName
LoggingLocService::getNationName(const type::Nation& nationCode) const
{
  type::NationName ans = base().getNationName(nationCode);
  // TODO: cache this also
  return ans;
}

type::Nation
LoggingLocService::getNation(const type::AirportCode& airport) const
{
  type::Nation ans =  base().getNation(airport);
  impl()._airportProperties[airport].nation = ans;
  return ans;
}

type::CityCode
LoggingLocService::getCityCode(const type::AirportCode& airport) const
{
  type::CityCode ans = base().getCityCode(airport);
  impl()._airportProperties[airport].cityCode = ans;
  return ans;
}

type::AlaskaZone
LoggingLocService::getAlaskaZone(const type::AirportCode& airport) const
{
  type::AlaskaZone ans = base().getAlaskaZone(airport);
  impl()._airportProperties[airport].alaskaZone = ans;
  return ans;
}

type::StateProvinceCode
LoggingLocService::getState(const type::AirportCode& airport) const
{
  type::StateProvinceCode ans = base().getState(airport);
  impl()._airportProperties[airport].state = ans;
  return ans;
}

type::CurrencyCode
LoggingLocService::getCurrency(const type::AirportCode& airport) const
{
  type::CurrencyCode ans = base().getCurrency(airport);
  impl()._airportProperties[airport].currency = ans;
  return ans;
}

bool
LoggingLocService::isInLoc(const type::AirportOrCityCode& airportCode,
                           const LocZone& jrnyLoc1LocZone,
                           const type::Vendor& vendor) const
{
  bool ans = base().isInLoc(airportCode, jrnyLoc1LocZone, vendor);
  impl()._locZoneCache[{jrnyLoc1LocZone, vendor}].portLocations.insert({airportCode, ans});
  return ans;
}

bool
LoggingLocService::matchPassengerLocation(const type::LocCode& passengerLoc,
                                          const LocZone& requiredLoc,
                                          const type::Vendor& vendor) const
{
  bool ans = base().matchPassengerLocation(passengerLoc, requiredLoc, vendor);
  impl()._locZoneCache[{requiredLoc, vendor}].paxLocations.insert({passengerLoc, ans});
  return ans;
}

std::string
LoggingLocService::getLog()
{
  return stringizeTheMap(impl()._airportProperties) + toXml(impl()._locZoneCache);
}

} // namespace tax
