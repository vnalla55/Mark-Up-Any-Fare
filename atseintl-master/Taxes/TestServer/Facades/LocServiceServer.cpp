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
#include "LocServiceServer.h"
#include "Common/LocZone.h"
#include "DataModel/Common/Types.h"
#include "DataModel/Common/CodeOps.h"

namespace tax
{

namespace
{

const Nation* getPortData(const boost::ptr_vector<Nation>& ports, type::AirportCode portCode) 
{
  for (const Nation& portData : ports)
    if (portData.locCode() == portCode)
      return &portData;

  return nullptr;
}

const Nation* getPortData(const boost::ptr_vector<Nation>& ports, type::Index index)
{
  if (index < ports.size())
    return &ports[index];
  else
    return nullptr; // defensive
}

const IsInLoc* getLocZoneData(const boost::ptr_vector<IsInLoc>& zones, const LocZone& zoneName, type::Vendor vendor)
{
  for (const IsInLoc& zoneDef : zones)
    if (zoneDef.loc() == zoneName && zoneDef.vendor() == vendor)
      return &zoneDef;

  return nullptr;
}

bool matches(const LocUsage& locUsage, const type::AirportOrCityCode& portCode, const boost::ptr_vector<Nation>& portDefs)
{
  if (!locUsage.airportCode().empty())
    return locUsage.airportCode() == portCode;

  if (const Nation* portData = getPortData(portDefs, locUsage.locRefId()))
    return portData->locCode() == portCode;

  return false;
}

bool matches(const tax::type::LocCode& passengerLocation, const Nation& portData)
{
  switch (passengerLocation.length())
  {
  case 2:
    return equal(portData.nationCode(), passengerLocation);
  case 3:
    return equal(portData.locCode(), passengerLocation);
  case 4:
    return passengerLocation.substr(2) == portData.state();
  default: // defensive
    return false;
  }
}

bool isFullCache(const boost::ptr_vector<IsInLoc>& zones)
{
  // a hack implementation: only diagnostic 830/XMLC adds "isPartial" flag
  return !zones.empty() && zones.front().isPartial();
  // Either all of them are partial or none, so it is enough to check any one.
  // In case zones.empty(), we have no way of telling, so we guess.
}

} // anonymous namespace

type::Nation
LocServiceServer::getNation(const type::AirportCode& loc) const
{
  if (const Nation* port = getPortData(_nations, loc))
    return port->nationCode();
  else
    return type::Nation(UninitializedCode);
}

type::Nation
LocServiceServer::getNationByName(const type::NationName& /*nationName*/) const
{
  return type::Nation(UninitializedCode);
}

type::NationName
LocServiceServer::getNationName(const type::Nation& /*nationCode*/) const
{
  return type::NationName();
}

type::CityCode
LocServiceServer::getCityCode(const type::AirportCode& loc) const
{
  if (const Nation* port = getPortData(_nations, loc))
    return port->cityCode();
  else
    return type::CityCode(UninitializedCode);
}

type::AlaskaZone
LocServiceServer::getAlaskaZone(const type::AirportCode& loc) const
{
  if (const Nation* port = getPortData(_nations, loc))
    return port->alaskaZone();
  else
    return type::AlaskaZone::Blank;
}

type::StateProvinceCode
LocServiceServer::getState(const type::AirportCode& loc) const
{
  if (const Nation* port = getPortData(_nations, loc))
    return port->state();
  else
    return type::StateProvinceCode();
}

type::CurrencyCode
LocServiceServer::getCurrency(const type::AirportCode& loc) const
{
  if (const Nation* port = getPortData(_nations, loc))
    return port->currency();
  else
    return type::CurrencyCode(UninitializedCode);
}

bool
LocServiceServer::isInLoc(const type::AirportOrCityCode& airportCode,
                          const LocZone& jrnyLoc1LocZone,
                          const type::Vendor& vendor) const
{
  if (jrnyLoc1LocZone.type() == type::LocType::Blank)
    return true;

  if (const IsInLoc* zoneDef = getLocZoneData(_isInLocs, jrnyLoc1LocZone, vendor))
  {
    for (const LocUsage& locUsage : zoneDef->locUsages())
    {
      if (matches(locUsage, airportCode, _nations))
        return locUsage.included();
    }
  }

  if (isFullCache(_isInLocs))
    throw std::runtime_error("Cached data does not contain loc information for " + airportCode.asString());
  else
    return false;
}

bool
LocServiceServer::matchPassengerLocation(const tax::type::LocCode& passengerLocation,
                                         const tax::LocZone& requiredLocation,
                                         const tax::type::Vendor& vendor) const
{
  if (isFullCache(_isInLocs))
  {
    if (const IsInLoc* zoneDef = getLocZoneData(_isInLocs, requiredLocation, vendor))
    {
      for (const PaxLocUsage& locUsage : zoneDef->paxLocUsages())
      {
        if (locUsage.locCode() == passengerLocation)
          return locUsage.included();
      }
    }

    throw std::runtime_error("Cached data does not contain pax loc information for " + passengerLocation);
  }
  else // old method
  {
    if (const IsInLoc* zoneDef = getLocZoneData(_isInLocs, requiredLocation, vendor))
    {
      for (const LocUsage& locUsage : zoneDef->locUsages())
      {
        const Nation* portData = getPortData(_nations, locUsage.locRefId());
        if (portData && matches(passengerLocation, *portData))
          return true;
      }
    }

    return false;
  }
}
}
