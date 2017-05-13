#pragma once

#include "Common/LocUtilEnumeration.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

namespace tse
{

class Loc;

namespace LocUtil
{
  bool isInLocImpl(const Loc& loc,
                   LocTypeCode locType,
                   const LocCode& locCode,
                   const VendorCode& vendorCode,
                   ZoneType zoneType,
                   ApplicationType applType,
                   GeoTravelType geoTvlType,
                   const CarrierCode& carrier,
                   DateTime ticketingDate);

  bool isInLocImpl(const LocCode& city,
                   LocTypeCode locType,
                   const LocCode& locCode,
                   const VendorCode& vendor,
                   ZoneType zoneType,
                   GeoTravelType geoTvlType,
                   ApplicationType applType,
                   const DateTime& ticketingDate);

  bool isInZoneCB(const Loc& loc,
                  const VendorCode& vendor,
                  const Zone& zone,
                  ZoneType zoneType,
                  ApplicationType applType,
                  GeoTravelType geoTvlType,
                  const CarrierCode& carrier,
                  DateTime ticketingDate);

}// LocUtil

} // end tse namespace
