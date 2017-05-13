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

#include <boost/function.hpp>

#include "Common/LocUtil.h"
#include "Taxes/AtpcoTaxes/DataModel/Common/Types.h"
#include "Taxes/AtpcoTaxes/ServiceInterfaces/LocService.h"

#include <memory>

namespace tax
{
class LocZone;
}

namespace tse
{
class DataHandle;

class LocServiceV2 : public tax::LocService
{

  friend class LocServiceV2Test;

public:
  LocServiceV2(const DateTime& ticketingDT);
  ~LocServiceV2() {}

  tax::type::Nation getNation(const tax::type::AirportCode& loc) const override;
  tax::type::Nation getNationByName(const tax::type::NationName& nationName) const override;
  tax::type::NationName getNationName(const tax::type::Nation& nationCode) const override;
  tax::type::CityCode getCityCode(const tax::type::AirportCode& loc) const override;
  tax::type::AlaskaZone getAlaskaZone(const tax::type::AirportCode& loc) const override;
  tax::type::StateProvinceCode getState(const tax::type::AirportCode& loc) const override;
  tax::type::CurrencyCode getCurrency(const tax::type::AirportCode& loc) const override;

  bool isInLoc(const tax::type::AirportOrCityCode& airportCode,
               const tax::LocZone& locOrZone,
               const tax::type::Vendor& vendor) const override;
  bool matchPassengerLocation(const tax::type::LocCode& passengerLocation,
                              const tax::LocZone& requiredLocation,
                              const tax::type::Vendor& vendor) const override;

private:
  typedef boost::function<bool(const Loc& city,
                               const LocTypeCode& locType,
                               const LocCode& locCode,
                               const VendorCode& vendor,
                               const ZoneType zoneType,
                               GeoTravelType geoTvlType,
                               const DateTime& ticketDate,
                               LocUtil::ApplicationType applType)> LegacyIsInLocFunction;

  LocServiceV2(const LocServiceV2&);
  LocServiceV2& operator=(const LocServiceV2&);

  LocTypeCode toTse(const tax::type::LocType& locType) const;
  ZoneType getZoneType(const tax::type::LocType& locType) const;

  std::unique_ptr<DataHandle> _dataHandle;
  const DateTime& _ticketingDT;
  LegacyIsInLocFunction _legacyIsInLoc;
};

} // namespace tse

