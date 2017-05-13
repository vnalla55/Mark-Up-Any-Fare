//----------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-----------------------------------------------------------------

#include "Rules/TransfersStopoversUtil.h"

#include "Common/LocUtil.h"
#include "Common/RtwUtil.h"
#include "Common/TseConsts.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"

namespace tse
{
namespace TransStopUtil
{
LocCode
getCity(const PricingTrx& trx, const FareMarket* fm, const Loc& loc)
{
  if (RtwUtil::isRtw(trx) && loc.loc() == LOC_EWR)
    return LOC_NYC;

  if (loc.cityInd() && loc.transtype() == 'P')
    return loc.loc();

  const GeoTravelType geoType = fm ? fm->geoTravelType() : GeoTravelType::International;
  const DateTime& travelDate = fm ? fm->travelDate() : trx.ticketingDate();
  const LocCode city = trx.dataHandle().getMultiTransportCityCode(loc.loc(), "", geoType, travelDate);

  return !city.empty() ? city : loc.loc();
}

std::string
getLocOfType(const PricingTrx& trx,
             const FareMarket* fm,
             const VendorCode& vendor,
             const Loc& loc,
             const LocTypeCode& locType)
{
  Zone zoneCode;

  switch (locType)
  {
  case LOCTYPE_AREA:
    return loc.area();
  case LOCTYPE_SUBAREA:
    return loc.subarea();
  case LOCTYPE_ZONE:
    if (LocUtil::getContinentalZoneCode(zoneCode, loc, vendor))
    {
      return zoneCode;
    }
    break;
  case LOCTYPE_STATE:
    return loc.state();
  case LOCTYPE_NATION:
    return loc.nation();
  case LOCTYPE_CITY:
    return getCity(trx, fm, loc);
  default:
    break;
  }

  return loc.loc();
}

bool
isInLoc(const PricingTrx& trx,
        const VendorCode& vendor,
        const Loc& loc,
        const LocKey& containingLoc)
{
  const LocTypeCode contLocType = containingLoc.locType();
  const LocCode& contLoc = containingLoc.loc();

  if (UNLIKELY(RtwUtil::isRtw(trx) && loc.loc() == LOC_EWR && contLocType == LOCTYPE_CITY && contLoc == LOC_NYC))
    return true;

  if (LocUtil::isInLoc(loc,
                       contLocType,
                       contLoc,
                       vendor,
                       RESERVED,
                       LocUtil::OTHER,
                       GeoTravelType::International,
                       "",
                       trx.ticketingDate()))
    return true;

  if (UNLIKELY(contLocType == LOCTYPE_CITY && LocUtil::isSameCity(loc.loc(), contLoc, trx.dataHandle())))
    return true;

  return false;
}

} // namespace TransStopUtil
}
