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
#include "Common/LocUtil.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Nation.h"
#include "Taxes/AtpcoTaxes/Common/LocZone.h"
#include "Taxes/LegacyFacades/ConvertCode.h"
#include "Taxes/LegacyFacades/DaoDataFormatConverter.h"
#include "Taxes/LegacyFacades/LocServiceV2.h"

#include <algorithm>

namespace tse
{
LocServiceV2::LocServiceV2(const DateTime& ticketingDT)
  : _dataHandle(new DataHandle(ticketingDT)),
    _ticketingDT(ticketingDT),
    _legacyIsInLoc(LocUtil::isAirportInLoc)
{
}

tax::type::Nation
LocServiceV2::getNation(const tax::type::AirportCode& loc) const
{
  const Loc* tseLoc = _dataHandle->getLoc(toTseAirportCode(loc), _ticketingDT);
  if (UNLIKELY(!tseLoc))
    return tax::type::Nation(tax::UninitializedCode);

  LocCode code = LocUtil::getAtpcoNation(*tseLoc);

  tax::type::Nation ans;
  bool result = codeFromString(&code[0], ans);
  if (UNLIKELY(!result))
    throw std::runtime_error("Nation code of size different than 2");
  return ans;
}

tax::type::Nation
LocServiceV2::getNationByName(const tax::type::NationName& nationName) const
{
  const std::vector<Nation*> nations = _dataHandle->getAllNation(_ticketingDT);
  std::vector<Nation*>::const_iterator found;
  found = std::find_if( nations.begin(), nations.end(), [&nationName](const Nation* nation)
      {
        return nation->description() == nationName;
      });

  if(found == nations.end())
    return tax::type::Nation(tax::UninitializedCode);

  return toTaxNationCode((*found)->nation());
}

tax::type::NationName
LocServiceV2::getNationName(const tax::type::Nation& nationCode) const
{
  const NationCode tseNationCode = toTseNationCode(nationCode);
  const Nation* nation = _dataHandle->getNation(tseNationCode, _ticketingDT);

  if(!nation)
    return tax::type::NationName();

  return nation->description();
}

tax::type::CityCode
LocServiceV2::getCityCode(const tax::type::AirportCode& loc) const
{
  const Loc* tseLoc = _dataHandle->getLoc(toTseAirportCode(loc), _ticketingDT);
  tse::LocCode cityCode = tseLoc ? tseLoc->city() : "";

  return toTaxCityCode(cityCode);
}

tax::type::AlaskaZone
LocServiceV2::getAlaskaZone(const tax::type::AirportCode& loc) const
{
  const Loc* tseLoc = _dataHandle->getLoc(toTseAirportCode(loc), _ticketingDT);
  tse::AlaskaZoneCode alaskaZoneCode = tseLoc ? tseLoc->alaskazone() : Indicator();

  tax::type::AlaskaZone ans;
  setTaxEnumValue(ans, alaskaZoneCode);
  return ans;
}

tax::type::StateProvinceCode
LocServiceV2::getState(const tax::type::AirportCode& loc) const
{
  const Loc* tseLoc = _dataHandle->getLoc(toTseAirportCode(loc), _ticketingDT);
  tse::StateCode stateCode = tseLoc ? tseLoc->state() : "";

  return stateCode;
}

tax::type::CurrencyCode
LocServiceV2::getCurrency(const tax::type::AirportCode& loc) const
{
  const Loc* tseLoc = _dataHandle->getLoc(toTseAirportCode(loc), _ticketingDT);
  if (!tseLoc)
  {
    return tax::type::CurrencyCode(tax::UninitializedCode);
  }

  const Nation* nation = _dataHandle->getNation(tseLoc->nation(), _ticketingDT);
  if (!nation)
  {
    return tax::type::CurrencyCode(tax::UninitializedCode);
  }

  return toTaxCurrencyCode(nation->primeCur());
}

bool
LocServiceV2::isInLoc(const tax::type::AirportOrCityCode& airportCode,
                      const tax::LocZone& locOrZone,
                      const tax::type::Vendor& vendor) const
{
  const Loc* tseLoc = _dataHandle->getLoc(toTseAirportOrCityCode(airportCode), _ticketingDT);

  tse::VendorCode tseVendor = locOrZone.type() == tax::type::LocType::Miscellaneous
       ? tse::VendorCode("SABR")
       : toTseVendorCode(vendor);

  return _legacyIsInLoc(*tseLoc,
                        toTse(locOrZone.type()),
                        toTseLocZoneCode(locOrZone.code()),
                        tseVendor,
                        getZoneType(locOrZone.type()),
                        GeoTravelType::International, // geo travel type
                        _ticketingDT, // date is constant
                        LocUtil::ATPCO_TAXES); // application type
}

LocTypeCode
LocServiceV2::toTse(const tax::type::LocType& locType) const
{
  typedef std::map<tax::type::LocType, tse::LocTypeCode> MappingType;

  static const std::map<tax::type::LocType, tse::LocTypeCode> mapping =
  {
    {tax::type::LocType::Blank, tse::LOCTYPE_NONE},
    {tax::type::LocType::Area, tse::LOCTYPE_AREA},
    {tax::type::LocType::City, tse::LOCTYPE_CITY},
    {tax::type::LocType::Airport, tse::LOCTYPE_AIRPORT},
    {tax::type::LocType::Nation, tse::LOCTYPE_NATION},
    {tax::type::LocType::StateProvince, tse::LOCTYPE_STATE},
    {tax::type::LocType::UserZone178, tse::LOCTYPE_ZONE},
    {tax::type::LocType::Miscellaneous, tse::LOCTYPE_ZONE},
    {tax::type::LocType::ZoneReserved, tse::LOCTYPE_ZONE},
  };

  MappingType::const_iterator it = mapping.find(locType);
  if (UNLIKELY(it == mapping.end()))
  {
    return LocTypeCode();
  }

  // return mapping.at(locType);
  return it->second;
}

ZoneType
LocServiceV2::getZoneType(const tax::type::LocType& locType) const
{
  if (locType == tax::type::LocType::UserZone178)
    return TAX_ZONE; // in case of UserZone178, we have to pass 'T' to LocUtil

  if (locType == tax::type::LocType::ZoneReserved)
    return RESERVED;

  return MANUAL; // in other case it doesn't matter what is the zone type
}

bool
LocServiceV2::matchPassengerLocation(const tax::type::LocCode& passengerLocation,
                                     const tax::LocZone& requiredLocation,
                                     const tax::type::Vendor& vendor) const
{
  LocKey locKey;
  locKey.loc() = requiredLocation.code().asString();
  locKey.locType() = static_cast<tse::LocTypeCode>(requiredLocation.type());
  return LocUtil::matchPaxLoc(
      locKey, toTseVendorCode(vendor), passengerLocation, *_dataHandle, GeoTravelType::International, _ticketingDT);
}

} // namespace tse
