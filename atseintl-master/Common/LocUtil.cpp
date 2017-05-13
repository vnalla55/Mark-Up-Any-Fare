//----------------------------------------------------------------------------
//
//  Description: Common Loc functions required for ATSE shopping/pricing.
//
//  Updates:
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Common/LocUtil.h"

#include "Common/Thread/TseCallableTrxTask.h"
#include "Common/TseUtil.h"
#include "Common/TSSCacheCommon.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/ATPResNationZones.h"
#include "DBAccess/DBServerPool.h"
#include "DBAccess/DST.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MultiAirportCity.h"
#include "DBAccess/MultiTransport.h"
#include "Rules/RuleConst.h"


#include <algorithm>
#include <vector>

#include <inttypes.h>

namespace tse
{

namespace LocUtil
{

namespace
{
LocList
initializeInterContinentalLocs()
{
  LocList interContinentalLocs;
  interContinentalLocs.emplace_back(LOCTYPE_ZONE, "000");
  interContinentalLocs.emplace_back(LOCTYPE_ZONE, "120");
  interContinentalLocs.emplace_back(LOCTYPE_ZONE, "140");
  interContinentalLocs.emplace_back(LOCTYPE_ZONE, "160");
  interContinentalLocs.emplace_back(LOCTYPE_ZONE, "170");
  interContinentalLocs.emplace_back(LOCTYPE_ZONE, "210");
  interContinentalLocs.emplace_back(LOCTYPE_ZONE, "220");
  interContinentalLocs.emplace_back(LOCTYPE_ZONE, "230");
  interContinentalLocs.emplace_back(LOCTYPE_ZONE, "310");
  interContinentalLocs.emplace_back(LOCTYPE_ZONE, "320");
  interContinentalLocs.emplace_back(LOCTYPE_ZONE, "330");
  interContinentalLocs.emplace_back(LOCTYPE_ZONE, "340");
  interContinentalLocs.emplace_back(LOCTYPE_ZONE, "350");
  interContinentalLocs.emplace_back(LOCTYPE_ZONE, "360");
  interContinentalLocs.emplace_back(LOCTYPE_ZONE, "370");
  return interContinentalLocs;
}

std::vector<LocList>
initializeInterContinentalExc()
{
  LocList list;
  list.emplace_back(LOCTYPE_ZONE, "000");
  list.emplace_back(LOCTYPE_ZONE, "120");
  return std::vector<LocList>(1u, list);
}

LocList _interContinentalLocs = initializeInterContinentalLocs();
std::vector<LocList> _interContinentalExceptions = initializeInterContinentalExc();
}

// -----------------------------------------------------------
// <PRE>
//
// @MethodName  FDHeaderMsgController::isWithin()
//
// Verifyes whether loc == origin == origin.
//
// @param   LocKey&   - Location and Location Type
// @param   Loc&      - Origin
// @para    Loc&      - Destination
//
// @return  bool      - If matching found then return true. Otherwise false.
//
// </PRE>
// -----------------------------------------------------------
bool
isWithin(const LocKey& loc1,
         const Loc& origin,
         const Loc& destination,
         const DateTime& ticketingDate)
{

  if (isInLoc(origin.loc(),
              loc1.locType(),
              loc1.loc(),
              Vendor::SABRE,
              MANUAL,
              GeoTravelType::International,
              OTHER,
              ticketingDate) &&
      isInLoc(destination.loc(),
              loc1.locType(),
              loc1.loc(),
              Vendor::SABRE,
              MANUAL,
              GeoTravelType::International,
              OTHER,
              ticketingDate))
    return true;
  return false;
}

// -----------------------------------------------------------
// <PRE>
//
// @MethodName  FDHeaderMsgController::isFrom()
//
// Verifyes whether loc1 == origin and loc2 == destination.
//
// @param  LocKey&      - Location1 and Location1 Type
// @param  LocKey&      - Location2 and Location2 Type
// @param  Loc&        - Origin
// @param  Loc&        - Destination
//
// @return  bool      - If matching found then return true. Otherwise false.
//
// </PRE>
// -----------------------------------------------------------
bool
isFrom(const LocKey& loc1,
       const LocKey& loc2,
       const Loc& origin,
       const Loc& destination,
       const DateTime& ticketingDate)
{

  if (isInLoc(origin.loc(),
              loc1.locType(),
              loc1.loc(),
              Vendor::SABRE,
              MANUAL,
              GeoTravelType::International,
              OTHER,
              ticketingDate) &&
      isInLoc(destination.loc(),
              loc2.locType(),
              loc2.loc(),
              Vendor::SABRE,
              MANUAL,
              GeoTravelType::International,
              OTHER,
              ticketingDate))
    return true;

  return false;
}

// -----------------------------------------------------------
// <PRE>
//
// @MethodName  FDHeaderMsgController::isBetween()
//
// Verifyes whether ( loc1 == origin and loc2 == destination.)
//                              or
//                  ( loc1 == destination and loc2 == origin.)
//
// @param  LocKey&      - Location1 and Location1 Type
// @param  LocKey&      - Location2 and Location2 Type
// @param  Loc&        - Origin
// @param  Loc&        - Destination
//
// @return  bool      - If matching found then return true. Otherwise false.
//
// </PRE>
// -----------------------------------------------------------
bool
isBetween(const LocKey& loc1,
          const LocKey& loc2,
          const Loc& origin,
          const Loc& destination,
          const DateTime& ticketingDate)
{

  if (isInLoc(origin.loc(),
              loc1.locType(),
              loc1.loc(),
              Vendor::SABRE,
              MANUAL,
              GeoTravelType::International,
              OTHER,
              ticketingDate) &&
      isInLoc(destination.loc(),
              loc2.locType(),
              loc2.loc(),
              Vendor::SABRE,
              MANUAL,
              GeoTravelType::International,
              OTHER,
              ticketingDate))
    return true;
  else if (isInLoc(destination.loc(),
                   loc1.locType(),
                   loc1.loc(),
                   Vendor::SABRE,
                   MANUAL,
                   GeoTravelType::International,
                   OTHER,
                   ticketingDate) &&
           isInLoc(origin.loc(),
                   loc2.locType(),
                   loc2.loc(),
                   Vendor::SABRE,
                   MANUAL,
                   GeoTravelType::International,
                   OTHER,
                   ticketingDate))
    return true;

  return false;
}

// -----------------------------------------------------------
// <PRE>
//
// @MethodName  isBetween()
//
//         Difference with above isBetween
//           Directly use the Loc&, instead of Loc.loc() for efficiency
//              VendorCode is explicit
//              ZoneType is explicit, to have correct result when doing isInZone check
//
// @param  LocKey&      - Location1 and Location1 Type
// @param  LocKey&      - Location2 and Location2 Type
// @param  Loc&      - Origin
// @param  Loc&      - Destination
// @param  VendorCode&    - Vendor
//
// @return  bool      - If matching found then return true. Otherwise false.
//
// </PRE>
// -----------------------------------------------------------
bool
isBetween(const LocKey& loc1,
          const LocKey& loc2,
          const Loc& origin,
          const Loc& destination,
          const VendorCode& vendor,
          const ZoneType zoneType,
          const ApplicationType applType,
          const DateTime& ticketingDate)
{
  if (isInLoc(origin,
              loc1.locType(),
              loc1.loc(),
              vendor,
              zoneType,
              applType,
              GeoTravelType::International,
              EMPTY_STRING(),
              ticketingDate) &&
      isInLoc(destination,
              loc2.locType(),
              loc2.loc(),
              vendor,
              zoneType,
              applType,
              GeoTravelType::International,
              EMPTY_STRING(),
              ticketingDate))
    return true;
  else if (isInLoc(destination,
                   loc1.locType(),
                   loc1.loc(),
                   vendor,
                   zoneType,
                   applType,
                   GeoTravelType::International,
                   EMPTY_STRING(),
                   ticketingDate) &&
           isInLoc(origin,
                   loc2.locType(),
                   loc2.loc(),
                   vendor,
                   zoneType,
                   applType,
                   GeoTravelType::International,
                   EMPTY_STRING(),
                   ticketingDate))
    return true;

  return false;
}

bool
isInLocImpl(const LocCode& city,
            LocTypeCode locTypeCode,
            const LocCode& locCode,
            const VendorCode& vendorCode,
            ZoneType zoneType,
            GeoTravelType geoTvlType,
            ApplicationType locAppl,
            const DateTime& ticketingDate)
{
  if (UNLIKELY(locAppl == RECORD1_2_6 &&
      (locTypeCode == LOCTYPE_CITY || locTypeCode == LOCTYPE_AIRPORT)))
    return (city == locCode); // need exact match

  DataHandle dataHandle(ticketingDate);
  const Loc* loc = dataHandle.getLoc(city, ticketingDate);
  if (LIKELY(loc))
  {
    return isInLoc(*loc,
                   locTypeCode,
                   locCode,
                   vendorCode,
                   zoneType,
                   locAppl,
                   geoTvlType,
                   EMPTY_STRING(),
                   ticketingDate);
  }
  else
  {
    return false;
  }
}

//----------------------------------------------------------------------------
// isAirportInLoc() : For special cases that we should ignore loc.cityInd()
//----------------------------------------------------------------------------
bool
isAirportInLoc(const Loc& loc,
               const LocTypeCode& locTypeCode,
               const LocCode& locCode,
               const VendorCode& vendorCode,
               const ZoneType zoneType,
               const GeoTravelType geoTvlType,
               const DateTime& ticketingDate,
               ApplicationType applType)
{
  if (locTypeCode == LOCTYPE_AIRPORT)
  {
    return (loc.loc() == locCode);
  }

  else
  {
    return isInLoc(loc,
                   locTypeCode,
                   locCode,
                   vendorCode,
                   zoneType,
                   applType,
                   geoTvlType,
                   EMPTY_STRING(),
                   ticketingDate);
  }
}

bool
isInLocImpl(const Loc& loc,
            LocTypeCode locTypeCode,
            const LocCode& locCode,
            const VendorCode& vendorCode,
            ZoneType zoneType,
            ApplicationType applType,
            GeoTravelType geoTvlType,
            const CarrierCode& carrier,
            DateTime ticketingDate)
{
  switch (locTypeCode)
  {
  case LOCTYPE_AREA:
    return loc.area() == locCode;
  case LOCTYPE_SUBAREA:
    return loc.subarea() == locCode;
  case LOCTYPE_CITY:
    if (applType == CHK_CITY_INLOC)
      return loc.city() == locCode;
    // The reason for the fallback here is to keep old behavior of this function
    // to avoid unwanted discrepancies in taxes (APM-590)
    if (loc.loc() == locCode)
      return true;
    if (applType == TAXES)
      return loc.loc() == locCode;
    // We need to make a db call to check because city and airport
    // for entries rely on the multi airport table
    // otherwise they are not equal
    //
    return isAirportInCity(loc.loc(), locCode, carrier, geoTvlType, ticketingDate);
  case LOCTYPE_AIRPORT:
    if (loc.loc() == locCode)
      return true;
    if (applType == FLIGHT_RULE || applType == SERVICE_FEE)
      return false;
    // We need to make a db call to check because city and airport
    // for entries rely on the multi airport table
    // otherwise they are not equal
    //
    return isAirportInCity(locCode, loc.loc(), carrier, geoTvlType, ticketingDate);
  case LOCTYPE_NATION:
    if (applType == ATPCO_TAXES)
      return getAtpcoNation(loc) == locCode;
    return loc.nation() == locCode;
  case LOCTYPE_STATE:
    return loc.state() == locCode;
  case LOCTYPE_ZONE:
    return isInZone(
        loc, vendorCode, locCode, zoneType, applType, geoTvlType, carrier, ticketingDate);
  case LOCTYPE_FMSZONE:
    return isInZone(loc, vendorCode, locCode, USER_DEFINED, applType, geoTvlType, carrier, ticketingDate);
  case LOCTYPE_NONE:
    return true;
  }

  return false; // Error, only unknown types get here
}

//----------------------------------------------------------------------------
// isInZone()
//----------------------------------------------------------------------------
bool
isInZone(const Loc& loc,
         const VendorCode& vendor,
         const Zone& zone,
         const ZoneType zoneType,
         ApplicationType applType,
         GeoTravelType geoTvlType,
         const CarrierCode& carrier,
         const DateTime& ticketingDate)
{
  return tsscache::isInZone(loc,
                            vendor,
                            zone,
                            zoneType,
                            applType,
                            geoTvlType,
                            carrier,
                            ticketingDate);
}

bool isInZoneCB(const Loc& loc,
                const VendorCode& vendor,
                const Zone& zone,
                ZoneType zoneType,
                ApplicationType applType,
                GeoTravelType geoTvlType,
                const CarrierCode& carrier,
                DateTime ticketingDate)
{
  if (UNLIKELY(vendor.empty()))
    return false;
  const LocCode& locCode(loc.loc());
  if (UNLIKELY(locCode.empty()))
    return false;
  return isLocInZone(loc,
                     vendor,
                     zone,
                     zoneType,
                     applType,
                     geoTvlType,
                     carrier,
                     ticketingDate);
}

namespace
{
const VendorCode ATPVendor = "ATP";
}

bool
areNationsInSameATPReservedZone(const NationCode& nation1, const NationCode& nation2)
{
  if (nation1 == nation2)
  {
    return true;
  }
  if (UNLIKELY(nation1.empty() || nation2.empty()))
  {
    return false;
  }

  DataHandle localDataHandle;

  const std::vector<ATPResNationZones*>& nation1Zones =
      localDataHandle.getATPResNationZones(nation1);
  const std::vector<ATPResNationZones*>& nation2Zones =
      localDataHandle.getATPResNationZones(nation2);

  if (nation1Zones.empty() || nation2Zones.empty())
  {
    return false;
  }

  // Each vector should always contain only one element
  //
  const ATPResNationZones* zones1 = nation1Zones.front();
  const ATPResNationZones* zones2 = nation2Zones.front();

  for (const auto& zone1 : zones1->zones())
    for (const auto& zone2 : zones2->zones())
      if (zone1 == zone2)
        return true;

  return false;
}

bool
isWithinOneATPReservedZone(const std::vector<TravelSeg*>& travel)
{
  if (UNLIKELY(travel.empty()))
  {
    return false;
  }

  std::vector<TravelSeg*>::const_iterator iter = travel.begin();
  std::vector<TravelSeg*>::const_iterator iterEnd = travel.end();

  const TravelSeg* prevTravelSeg = nullptr;

  for (; iter != iterEnd; ++iter)
  {
    const TravelSeg* ts = *iter;
    if (!areNationsInSameATPReservedZone(ts->origin()->nation(), ts->destination()->nation()))
    {
      return false;
    }

    if (UNLIKELY(prevTravelSeg && !areNationsInSameATPReservedZone(prevTravelSeg->origin()->nation(),
                                                          ts->destination()->nation())))
    {
      return false;
    }
    prevTravelSeg = ts;
  }

  return true;
}

/**
 * Tests if a loc meets the zone criteria.
 */
bool
isLocInZone(const Loc& loc,
            const VendorCode& vendor,
            const Zone& zone,
            const ZoneType zoneType,
            ApplicationType applType,
            GeoTravelType geoTvlType,
            const CarrierCode& carrier,
            const DateTime& ticketingDate)
{
  DataHandle dataHandle(ticketingDate);
  Zone zoneNo = zone;

  // if the zone is less than seven characters, then
  // pad the front of it with '0' characters
  if (zoneNo.size() < 7)
  {
    const int diff = 7 - int(zoneNo.size());
    for (int n = 6; n >= 0; --n)
    {
      if (n - diff < 0)
      {
        zoneNo[n] = '0';
      }
      else
      {
        zoneNo[n] = zoneNo[n - diff];
      }
    }
  }

  const ZoneInfo* zoneInfo = dataHandle.getZone(vendor, zoneNo, zoneType, ticketingDate);
  if (zoneInfo == nullptr && (zoneType == 'R' || zoneType == 'T'))
    zoneInfo = dataHandle.getZone(ATPVendor, zoneNo, zoneType, ticketingDate);

  if (UNLIKELY(zoneInfo == nullptr))
    return false;

  if (matchZone(dataHandle, loc, zoneInfo, applType, geoTvlType, carrier) > 0)
    return true;

  return false;
}

// this shifts string to the right by (7 - orig size)
// filling with '0' on the left
// assumed Zone is Code<7>
void
padZoneNo(Zone& zone)
{
  const size_t origSz(zone.length());
  if (origSz < 7)
  {
    const ptrdiff_t diff(7 - origSz);
    for (ptrdiff_t i = origSz - 1; i >= 0; --i)
    {
      zone[i + diff] = zone[i];
    }
    for (ptrdiff_t j = 0; j < diff; ++j)
    {
      zone[j] = '0';
    }
    zone[7] = '\0';
  }
}

int
matchZone(DataHandle& dataHandle,
          const Loc& loc,
          const ZoneInfo* zoneInfo,
          ApplicationType applType,
          GeoTravelType geoTvlType,
          const CarrierCode& carrier,
          const DateTime& ticketingDate)
{
  if (zoneInfo->isUniform())
  {
    return matchUniformZone(loc, zoneInfo, applType, geoTvlType, carrier, ticketingDate);
  }

  int include = 0;
  int exclude = 0;

  std::vector<std::vector<ZoneInfo::ZoneSeg> >::const_iterator i = zoneInfo->sets().begin();
  for (; i != zoneInfo->sets().end(); ++i)
  {
    std::vector<ZoneInfo::ZoneSeg>::const_iterator j = i->begin();
    for (; j != i->end(); ++j)
    {
      int score = 0;
      switch (j->locType())
      {
      case LOCTYPE_CITY:
        if (j->loc() == loc.loc())
          score = 5;

        if (applType != TAXES)
        {
          if (isAirportInCity(loc.loc(), j->loc(), carrier, geoTvlType, ticketingDate))
            score = 5;
        }

        break;
      case LOCTYPE_STATE:
        if (j->loc() == loc.state())
          score = 4;
        break;
      case LOCTYPE_NATION:
        if (UNLIKELY(applType == ATPCO_TAXES))
        {
          if (getAtpcoNation(loc) == j->loc())
            score = 3;
          break;
        }
        if (j->loc() == loc.nation())
          score = 3;
        break;
      case LOCTYPE_SUBAREA:
        if (j->loc() == loc.subarea())
          score = 2;
        break;
      case LOCTYPE_AREA:
        if (j->loc() == loc.area())
          score = 1;
        break;
      case LOCTYPE_AIRPORT:
        if (j->loc() == loc.loc() ||
            isAirportInCity(loc.loc(), j->loc(), carrier, geoTvlType, ticketingDate))
          score = 6;
        break;
      case LOCTYPE_ZONE:
        Zone zoneNo = j->loc();
        padZoneNo(zoneNo);
        ZoneType zoneType = zoneInfo->zoneType();
        if (LIKELY(zoneType == 'U' || zoneType == 'T'))
          zoneType = 'R'; // user-defined zones are made up of 'R' zones

        const ZoneInfo* subZone =
            dataHandle.getZone(zoneInfo->vendor(), zoneNo, zoneType, ticketingDate);

        if (subZone == nullptr && (zoneType == 'R' || zoneType == 'T'))
          subZone = dataHandle.getZone("ATP", zoneNo, zoneType, ticketingDate);

        if (UNLIKELY(subZone == nullptr))
          continue;

        int match = matchZone(dataHandle, loc, subZone);
        if (match > 0)
          score = match;
        break;
      }
      if (score != 0)
      {
        if (j->inclExclInd() == 'I')
        {
          if (score > include)
            include = score;
        }
        else // (j->inclExclInd() == 'E')
        {
          if (score > exclude)
            exclude = score;
        }
      }
    }
  }
  if (include > exclude)
    return include;
  return 0 - exclude;
}


int
matchUniformZone(const Loc& loc,
                 const ZoneInfo* zone,
                 ApplicationType applType,
                 GeoTravelType geoTvlType,
                 const CarrierCode& carrier,
                 const DateTime& ticketingDate)
{
  typedef std::vector<ZoneInfo::ZoneSeg> ZoneLocSet;

  const ZoneLocSet& locSet(zone->sets().front());
  LocTypeCode locType(locSet.front().locType());

  switch (locType)
  {
    case LOCTYPE_CITY:
    {
      if (std::binary_search(locSet.begin(), locSet.end(), loc.loc(), ZoneSegCmpByLoc()))
        return 5;
      if (applType != TAXES && applType != ATPCO_TAXES)
      {
        const LocCode& city = getMultiTransportCity(loc.loc(), carrier, geoTvlType, ticketingDate);
        if (city != loc.loc())
        {
          if (std::binary_search(locSet.begin(), locSet.end(), city, ZoneSegCmpByLoc()))
            return 5;
        }
      }
      break;
    }
    case LOCTYPE_STATE:
    {
      if (std::binary_search(locSet.begin(), locSet.end(), loc.state(), ZoneSegCmpByLoc()))
        return 4;
      break;
    }
    case LOCTYPE_NATION:
    {
      if (applType != ATPCO_TAXES)
      {
        if (std::binary_search(locSet.begin(), locSet.end(), loc.nation(), ZoneSegCmpByLoc()))
          return 3;
      }
      else
      {
        if (std::binary_search(locSet.begin(), locSet.end(), getAtpcoNation(loc), ZoneSegCmpByLoc()))
          return 3;
      }
      break;
    }
    case LOCTYPE_SUBAREA:
    {
      if (std::binary_search(locSet.begin(), locSet.end(), loc.subarea(), ZoneSegCmpByLoc()))
        return 2;
      break;
    }
    case LOCTYPE_AREA:
    {
      if (std::binary_search(locSet.begin(), locSet.end(), loc.area(), ZoneSegCmpByLoc()))
        return 1;
      break;
    }
    case LOCTYPE_AIRPORT:
    {
      if (std::binary_search(locSet.begin(), locSet.end(), loc.loc(), ZoneSegCmpByLoc()))
        return 6;
      const LocCode& city = getMultiTransportCity(loc.loc(), carrier, geoTvlType, ticketingDate);
      if (city != loc.loc())
      {
        if (std::binary_search(locSet.begin(), locSet.end(), city, ZoneSegCmpByLoc()))
          return 6;
      }
      break;
    }
    default:
    {
      //ERROR - called for unsupported location type
      return 0;
    }
  }

  return 0;
}


/**
 * Tests if an O/D pair meets the zone criteria.
 */
bool
isInZone(const Loc& locFrom,
         const Loc& locTo,
         const VendorCode& vendor,
         const Zone& zone,
         const ZoneType zoneType,
         const DateTime& ticketingDate)
{
  DataHandle dataHandle(ticketingDate);
  Zone zoneNo = zone;
  padZoneNo(zoneNo);
  const ZoneInfo* zoneInfo = dataHandle.getZone(vendor, zoneNo, zoneType, ticketingDate);
  if (UNLIKELY(zoneInfo == nullptr))
    return false;

  std::vector<std::vector<ZoneInfo::ZoneSeg> >::const_iterator i = zoneInfo->sets().begin();
  for (; i != zoneInfo->sets().end(); ++i)
  {
    uint8_t fromMatch = matchZone(locFrom, *i);
    uint8_t toMatch = matchZone(locTo, *i);
    if (UNLIKELY((fromMatch & MATCHES_FROM) && (toMatch & MATCHES_TO)))
      return true;
    if ((fromMatch & MATCHES_BETWEEN) && (toMatch & MATCHES_AND))
      return true;
    if ((toMatch & MATCHES_BETWEEN) && (fromMatch & MATCHES_AND))
      return true;
    if ((toMatch & MATCHES_NONE) && (fromMatch & MATCHES_NONE))
      return true;
  }
  return false;
}

/**
 * matchZone
 */
uint8_t
matchZone(const Loc& loc, const std::vector<ZoneInfo::ZoneSeg>& set)
{
  uint8_t ret = 0;
  uint8_t qualifiersFound = 0;
  LocTypeCode matchBetween = ' ';
  LocTypeCode matchAnd = ' ';
  LocTypeCode matchFrom = ' ';
  LocTypeCode matchTo = ' ';
  LocTypeCode matchNone = ' ';
  std::vector<ZoneInfo::ZoneSeg>::const_iterator i = set.begin();
  for (; i != set.end(); ++i)
  {
    uint8_t currentType = 0;
    LocTypeCode* currentQualifier;

    // determine the directionalQualifier
    switch (i->directionalQualifier())
    {
    case 'B':
      currentType = MATCHES_BETWEEN;
      currentQualifier = &matchBetween;
      break;
    case 'A':
      currentType = MATCHES_AND;
      currentQualifier = &matchAnd;
      break;
    case 'T':
      currentType = MATCHES_TO;
      currentQualifier = &matchTo;
      break;
    case 'F':
      currentType = MATCHES_FROM;
      currentQualifier = &matchFrom;
      break;
    case 'N':
      currentType = MATCHES_NONE;
      currentQualifier = &matchNone;
      break;
    default:
      continue;
    }
    qualifiersFound |= currentType;

    // match the loctype of the zone segment against the last one found for the
    // directional qualifier; loop back soon as we find which one is more specific
    // TODO: process LOCTYPE_ZONE for use in isLocInZone for one loc.
    if (UNLIKELY(*currentQualifier == LOCTYPE_CITY))
      continue;
    if (UNLIKELY(i->locType() == LOCTYPE_CITY))
    {
      if (i->loc() == loc.loc())
        flipBit(ret, currentType, i->inclExclInd());
      continue;
    }
    if (UNLIKELY(*currentQualifier == LOCTYPE_STATE))
      continue;
    if (UNLIKELY(i->locType() == LOCTYPE_STATE))
    {
      if (i->loc() == loc.state())
        flipBit(ret, currentType, i->inclExclInd());
      continue;
    }
    if (UNLIKELY(*currentQualifier == LOCTYPE_NATION))
      continue;
    if (i->locType() == LOCTYPE_NATION)
    {
      if (i->loc() == loc.nation())
        flipBit(ret, currentType, i->inclExclInd());
      continue;
    }
    if (UNLIKELY(*currentQualifier == LOCTYPE_SUBAREA))
      continue;
    if (UNLIKELY(i->locType() == LOCTYPE_SUBAREA))
    {
      if (i->loc() == loc.subarea())
        flipBit(ret, currentType, i->inclExclInd());
      continue;
    }
    if (UNLIKELY(*currentQualifier == LOCTYPE_AREA))
      continue;
    if (LIKELY(i->locType() == LOCTYPE_AREA))
    {
      if (i->loc() == loc.area())
        flipBit(ret, currentType, i->inclExclInd());
      continue;
    }
  }

  // if only one side of to/from or between/and was found, the other is assumed true
  if (UNLIKELY((qualifiersFound & MATCHES_TO) && !(qualifiersFound & MATCHES_FROM)))
    ret |= MATCHES_FROM;
  if (UNLIKELY(!(qualifiersFound & MATCHES_TO) && (qualifiersFound & MATCHES_FROM)))
    ret |= MATCHES_TO;
  if (UNLIKELY((qualifiersFound & MATCHES_BETWEEN) && !(qualifiersFound & MATCHES_AND)))
    ret |= MATCHES_AND;
  if (UNLIKELY(!(qualifiersFound & MATCHES_BETWEEN) && (qualifiersFound & MATCHES_AND)))
    ret |= MATCHES_BETWEEN;

  return ret;
}

void
flipBit(uint8_t& ret, uint8_t currentType, Indicator inclExclInd)
{
  if (inclExclInd == 'I')
    ret |= currentType;
  else if (LIKELY(inclExclInd == 'E'))
    ret &= uint8_t(~currentType);
}

//----------------------------------------------------------------------------
// isOverWater()
//----------------------------------------------------------------------------
bool
isOverWater(const Loc& loc1, const Loc& loc2)
{

  NationCode* p1, *p2;
  NationCode OverWater[] = { ALASKA, HAWAII, PUERTORICO, VIRGIN_ISLAND, NATION_EMPTY };
  p1 = std::find(&OverWater[0], &OverWater[4], loc1.state());
  p2 = std::find(&OverWater[0], &OverWater[4], loc2.state());
  bool rc1 = (p1 != &OverWater[4]);
  bool rc2 = (p2 != &OverWater[4]);
  if (isDomestic(loc1, loc2))
  {
    if (loc1.state() != loc2.state())
    {
      if (((*p1 == PUERTORICO && *p2 == VIRGIN_ISLAND)) ||
          ((*p1 == VIRGIN_ISLAND && *p2 == PUERTORICO)))
      {
        return false;
      }
      return (rc1 || rc2);
    }
    return false;
  }

  else if (isTransBorder(loc1, loc2))
  {
    return (rc1 || rc2);
  }

  else if (isInternational(loc1, loc2))
  {
    if (loc1.area() != loc2.area())
    {
      return true;
    }
    if (loc1.subarea() == loc2.subarea())
    {
      return false;
    }

    if ((loc1.subarea() == IATA_SUB_AREA_13() && loc2.subarea() == IATA_SUB_AREA_14()) ||
        (loc1.subarea() == IATA_SUB_AREA_14() && loc2.subarea() == IATA_SUB_AREA_13()))
    {
      return false;
    }

    std::set<IATASubAreaCode> subAreas;
    subAreas.insert(IATA_SUB_AREA_11());
    subAreas.insert(IATA_SUB_AREA_12());
    subAreas.insert(IATA_SUB_AREA_13());
    subAreas.insert(IATA_SUB_AREA_14());

    if (subAreas.find(loc1.subarea()) != subAreas.end() &&
        subAreas.find(loc2.subarea()) != subAreas.end())
    {
      return true;
    }
  }
  return false;
}

//----------------------------------------------------------------------------
// isDomesticUSCA()
//----------------------------------------------------------------------------
bool
isDomesticUSCA(const Loc& loc)
{

  if (isDomesticUS(loc) || (loc.nation() == CANADA))
  {
    return true;
  }

  return false;
}

bool
isDomesticRussia(const Loc& loc1, const Loc& loc2)
{
  return isRussia(loc1) && isRussia(loc2);
}

//----------------------------------------------------------------------------
// isDomestic()
//----------------------------------------------------------------------------
bool
isDomestic(const Loc& loc1, const Loc& loc2)
{
  if (loc1.nation() == loc2.nation())
  {
    return (loc1.nation() == UNITED_STATES) || (loc1.nation() == CANADA);
  }
  return false;
}

//----------------------------------------------------------------------------
// isTransBorder()
//----------------------------------------------------------------------------
bool
isTransBorder(const Loc& loc1, const Loc& loc2)
{
  if ((loc1.nation() != loc2.nation()))
  {
    return (((loc1.nation() == UNITED_STATES) && (loc2.nation() == CANADA)) ||
            ((loc2.nation() == UNITED_STATES) && (loc1.nation() == CANADA)));
  }
  return false;
}

//----------------------------------------------------------------------------
// isForeignDomestic()
//----------------------------------------------------------------------------
bool
isForeignDomestic(const Loc& loc1, const Loc& loc2)
{
  if (loc1.nation() == loc2.nation())
  {
    return !isDomestic(loc1, loc2);
  }
  return false;
}

//----------------------------------------------------------------------------
// isInternational()
//----------------------------------------------------------------------------
bool
isInternational(const Loc& loc1, const Loc& loc2)
{
  return ((loc1.nation() != loc2.nation()) && !isTransBorder(loc1, loc2));
}

//----------------------------------------------------------------------------
// isUSTerritory()
//----------------------------------------------------------------------------
bool
isUSTerritory(const Loc& loc1, const Loc& loc2)
{
  static NationCode usTeritories[] = { UNITED_STATES, CANADA, GUAM };

  return (TseUtil::isMember(loc1.nation(), usTeritories) &&
          TseUtil::isMember(loc2.nation(), usTeritories));
}

//----------------------------------------------------------------------------
// isWithinZone210()
//----------------------------------------------------------------------------
bool
isWithinZone210(const Loc& loc1, const Loc& loc2)
{
  static const Zone zone("210");
  return (isInLoc(loc1, LOCTYPE_ZONE, zone, Vendor::ATPCO, USER_DEFINED) &&
          isInLoc(loc2, LOCTYPE_ZONE, zone, Vendor::ATPCO, USER_DEFINED));
}

//----------------------------------------------------------------------------
// isUSTerritoryRule()
//----------------------------------------------------------------------------
bool
isUSTerritoryRule(const Loc& loc)
{
  static NationCode usTeritories[] = { UNITED_STATES, CANADA, GUAM };
  return (TseUtil::isMember(loc.nation(), usTeritories));
}

//----------------------------------------------------------------------------
// isMinFareUSTerritory()
//----------------------------------------------------------------------------
bool
isMinFareUSTerritory(const Loc& loc)
{
  static NationCode usTeritories[] = {
    AMERICAN_SAMOA, CANTON_AND_ENDERBURY, GUAM,          MICRONESIA,    MIDWAY, NORTHERN_MARIANA,
    PUERTORICO,     SAIPAN,               UNITED_STATES, VIRGIN_ISLAND, WAKE,
  };

  return TseUtil::isMember(loc.nation(), usTeritories);
}

bool
isBaggageUSTerritory(const Loc& loc)
{
  static NationCode usTeritories[] = { "US", "AS", "FM", "GU", "MH", "MP", "UM" };

  return TseUtil::isMember(loc.nation(), usTeritories);
}

//----------------------------------------------------------------------------
// isScandinavia()
//----------------------------------------------------------------------------
bool
isScandinavia(const Loc& loc)
{
  static NationCode scandinaviaNations[] = { DENMARK, NORWAY, SWEDEN, GREENLAND };

  return TseUtil::isMember(loc.nation(), scandinaviaNations);
}

bool
isEurope(const Loc& loc)
{
  const static Zone zone("210");
  return isInZone(loc, Vendor::ATPCO, zone, RESERVED, OTHER, GeoTravelType::International);
}

bool
isMiddleEast(const Loc& loc)
{
  const static Zone zone("220");
  return isInZone(loc, Vendor::ATPCO, zone, RESERVED, OTHER, GeoTravelType::International);
}

bool
oneOfNetherlandAntilles(const Loc& loc)
{
  static NationCode NETHERLAND_ANTILLES[] = { "BQ", "CW", "SX", "AW", "AN" };
  return TseUtil::isMember(loc.nation(), NETHERLAND_ANTILLES);
}

//----------------------------------------------------------------------------
// isFrenchNationGroup()
//----------------------------------------------------------------------------
bool
isFrenchNationGroup(const Loc& loc)
{
  static NationCode frNations[] = {
    "FR", "GP", "MQ", "GF", "PM", "YT", "RE", "KM", "NC", "PF", "WF"
  };

  return TseUtil::isMember(loc.nation(), frNations);
}

//----------------------------------------------------------------------------
// isRussianGroup()
//----------------------------------------------------------------------------
bool
isRussianGroup(const Loc& loc)
{
  static NationCode russianNations[] = { "RU", "XU" };

  return TseUtil::isMember(loc.nation(), russianNations);
}

//----------------------------------------------------------------------------
// isUSTerritoryOnly()
//----------------------------------------------------------------------------
bool
isUSTerritoryOnly(const Loc& loc)
{
  static StateCode usTeritories[] = { AMERICAN_SAMOA, CANTON_AND_ENDERBURY, GUAM,       MICRONESIA,
                                      MIDWAY,         NORTHERN_MARIANA,     PUERTORICO, SAIPAN,
                                      VIRGIN_ISLAND,  WAKE };

  return TseUtil::isMember(loc.state(), usTeritories);
}

//----------------------------------------------------------------------------
// isUSPossession()
//----------------------------------------------------------------------------
bool
isUSPossession(const Loc& loc)
{
  static NationCode usPossessions[] = { AMERICAN_SAMOA, CANTON_AND_ENDERBURY, GUAM,
                                        MICRONESIA,     MIDWAY,               NORTHERN_MARIANA,
                                        PUERTORICO,     SAIPAN,               VIRGIN_ISLAND,
                                        WAKE };

  return TseUtil::isMember(loc.nation(), usPossessions);
}

//----------------------------------------------------------------------------
// isAirportInCity()
//----------------------------------------------------------------------------
bool
isAirportInCity(const LocCode& airport,
                const LocCode& city,
                const CarrierCode& carrier,
                const GeoTravelType geoTvlType,
                const DateTime& ticketingDate)
{
  return (getMultiTransportCity(airport, carrier, geoTvlType, ticketingDate) == city);
}

//----------------------------------------------------------------------------
// getMultiTransportCity() by GEO travel type
//----------------------------------------------------------------------------
LocCode
getMultiTransportCity(const LocCode& locCode,
                      const CarrierCode& carrierCode,
                      GeoTravelType tvlType,
                      const DateTime& ticketingDate)
{
  DataHandle dataHandle(ticketingDate);
  return dataHandle.getMultiTransportCityCode(locCode, carrierCode, tvlType, ticketingDate);
}

//----------------------------------------------------------------------------
// getMultiTransportCity() by tvl seg info
//----------------------------------------------------------------------------
LocCode
getMultiTransportCity(const LocCode& locCode, const AirSeg& airSeg)
{
  return getMultiTransportCity(
      locCode, airSeg.carrier(), airSeg.geoTravelType(), airSeg.departureDT());
}

bool
isSameMultiTransportCity(const LocCode& loc1,
                         const LocCode& loc2,
                         const CarrierCode& cxr,
                         const GeoTravelType tvlType,
                         const DateTime& tvlDate)
{
  LocCode city1 = getMultiTransportCity(loc1, cxr, tvlType, tvlDate);
  if (city1.empty())
    city1 = loc1;

  LocCode city2 = getMultiTransportCity(loc2, cxr, tvlType, tvlDate);
  if (city2.empty())
    city2 = loc2;

  return city1 == city2;
}

//----------------------------------------------------------------------------
// getMultiCityAirports()
//----------------------------------------------------------------------------
const std::vector<tse::MultiAirportCity*>&
getMultiCityAirports(const LocCode& locCode,
                     PricingTrx* trx)
{
  DataHandle dataHandle;
  if (LIKELY(trx))
  {
    dataHandle.setParentDataHandle(&trx->dataHandle());
  }

  return (dataHandle.getMultiCityAirport(locCode));
}

//----------------------------------------------------------------------------
// isMultiAirport()
//----------------------------------------------------------------------------
bool
isMultiAirport(const LocCode& locCode,
               PricingTrx* trx)
{
  return (getMultiCityAirports(locCode, trx).size() > 1);
}

struct SameCity
    : public std::binary_function<const MultiAirportCity*, const MultiAirportCity*, bool>
{
  bool operator()(const MultiAirportCity* city1, const MultiAirportCity* city2) const
  {
    return city1->city() == city2->city();
  }
} sameCity;

//----------------------------------------------------------------------------
// isSameCity()
//----------------------------------------------------------------------------
bool
isSameCity(const LocCode& locCode1, const LocCode& locCode2, DataHandle& dataHandle)
{
  std::vector<MultiAirportCity*> loc1(dataHandle.getMultiAirportCity(locCode1));
  std::vector<MultiAirportCity*> loc2(dataHandle.getMultiAirportCity(locCode2));
  MultiAirportCity mac1, mac2;
  mac1.city() = mac1.airportCode() = locCode1;
  mac2.city() = mac2.airportCode() = locCode2;
  loc1.push_back(&mac1);
  loc2.push_back(&mac2);
  return std::find_first_of(loc1.begin(), loc1.end(), loc2.begin(), loc2.end(), sameCity) !=
         loc1.end();
}

//----------------------------------------------------------------------------
// isUS()
//----------------------------------------------------------------------------
bool
isUS(const Loc& loc)
{
  return (loc.nation() == UNITED_STATES);
}

//----------------------------------------------------------------------------
// isCanada()
//----------------------------------------------------------------------------
bool
isCanada(const Loc& loc)
{
  return (loc.nation() == CANADA);
}

//----------------------------------------------------------------------------
// isColumbia()
//----------------------------------------------------------------------------
bool
isColumbia(const Loc& loc)
{
  return (loc.nation() == COLUMBIA);
}

//----------------------------------------------------------------------------
// isMexico()
//----------------------------------------------------------------------------
bool
isMexico(const Loc& loc)
{
  return (loc.nation() == MEXICO);
}

//----------------------------------------------------------------------------
// isHawaii()
//----------------------------------------------------------------------------
bool
isHawaii(const Loc& loc)
{
  return (loc.state() == HAWAII);
}

//----------------------------------------------------------------------------
// isAlaska()
//----------------------------------------------------------------------------
bool
isAlaska(const Loc& loc)
{
  return (loc.state() == ALASKA);
}

//----------------------------------------------------------------------------
// isPuertoRico()
//----------------------------------------------------------------------------
bool
isPuertoRico(const Loc& loc)
{
  return (loc.state() == PUERTORICO);
}

//----------------------------------------------------------------------------
// isTransAtlantic()
//----------------------------------------------------------------------------
bool
isTransAtlantic(const Loc& loc1, const Loc& loc2, const GlobalDirection& dir)
{
  bool a1_a2 = ((loc1.area() == IATA_AREA1 && loc2.area() == IATA_AREA2) ||
                (loc1.area() == IATA_AREA2 && loc2.area() == IATA_AREA1));

  bool a1_a3 = ((loc1.area() == IATA_AREA1 && loc2.area() == IATA_AREA3) ||
                (loc1.area() == IATA_AREA3 && loc2.area() == IATA_AREA1));

  return ((a1_a2 || a1_a3) && (dir == GlobalDirection::AT || dir == GlobalDirection::SA ||
                               dir == GlobalDirection::SN || dir == GlobalDirection::AP));
}

bool
isTransPacific(const Loc& loc1, const Loc& loc2, const GlobalDirection& dir)
{
  bool a1_a3 = ((loc1.area() == IATA_AREA1 && loc2.area() == IATA_AREA3) ||
                (loc1.area() == IATA_AREA3 && loc2.area() == IATA_AREA1));

  return (a1_a3 && (dir == GlobalDirection::NP || dir == GlobalDirection::PA ||
                    dir == GlobalDirection::PN || dir == GlobalDirection::AP));
}

bool
isTransOceanic(const Loc& loc1, const Loc& loc2, const GlobalDirection& dir)
{
  return (isTransAtlantic(loc1, loc2, dir) || isTransPacific(loc1, loc2, dir));
}

void
getOverWaterSegments(const std::vector<TravelSeg*>& inSeg, std::vector<TravelSeg*>& outSeg)
{
  std::vector<TravelSeg*>::const_iterator it = inSeg.begin();
  std::vector<TravelSeg*>::const_iterator ite = inSeg.end();

  bool isWithin_IATA_AREA1 = true;

  for (; it != ite; ++it)
  {
    if (UNLIKELY(((*it)->origin()->area() != IATA_AREA1) || ((*it)->destination()->area() != IATA_AREA1)))
    {
      isWithin_IATA_AREA1 = false;
      break;
    }
  }

  if (LIKELY(isWithin_IATA_AREA1))
  {
    for (it = inSeg.begin(); it != ite; ++it)
    {
      const Loc* orig = (*it)->origin();
      const Loc* dest = (*it)->destination();

      if (isOverWater(*orig, *dest))
      {
        outSeg.push_back(*it);
      }
    }
  }
  else
  {
    for (it = inSeg.begin(); it != ite; ++it)
    {
      const Loc* orig = (*it)->origin();
      const Loc* dest = (*it)->destination();

      if ((orig->area() == IATA_AREA1 && dest->area() == IATA_AREA2) ||
          (orig->area() == IATA_AREA2 && dest->area() == IATA_AREA1) ||
          (orig->area() == IATA_AREA1 && dest->area() == IATA_AREA3) ||
          (orig->area() == IATA_AREA3 && dest->area() == IATA_AREA1))
      {
        outSeg.push_back(*it);
      }
    }
  }
}

//----------------------------------------------------------------------------
// getInternationalSegments()
//----------------------------------------------------------------------------
void
getInternationalSegments(const std::vector<TravelSeg*>& inSeg,
                         std::vector<TravelSeg*>& outSeg)
{
  std::vector<TravelSeg*>::const_iterator it = inSeg.begin();
  std::vector<TravelSeg*>::const_iterator ite = inSeg.end();

  bool isUSCanada = true;

  for (; it != ite; ++it)
  {
    const Loc* orig = (*it)->origin();
    const Loc* dest = (*it)->destination();

    if (((!isDomestic(*orig, *dest)) && (!isTransBorder(*orig, *dest))) || (*it)->isRoundTheWorld())
    {
      isUSCanada = false;
      break;
    }
  }

  // For travel between US and Canada only, US <-> Canada is international
  //
  if (isUSCanada)
  {
    for (it = inSeg.begin(); it != ite; ++it)
    {
      const Loc* orig = (*it)->origin();
      const Loc* dest = (*it)->destination();

      if (isTransBorder(*orig, *dest))
      {
        outSeg.push_back(*it);
      }
    }
  }
  else
  {
    for (it = inSeg.begin(); it != ite; ++it)
    {
      const Loc* orig = (*it)->origin();
      const Loc* dest = (*it)->destination();

      if (isInternational(*orig, *dest) || (*it)->isRoundTheWorld())
      {
        outSeg.push_back(*it);
      }
    }
  }
}

//----------------------------------------------------------------------------
// isInterContinental()
//----------------------------------------------------------------------------
bool
isInterContinental(const Loc& loc1, const Loc& loc2, const VendorCode& vendorCode)
{
  for (const LocList& locs : _interContinentalExceptions)
  {
    if (UNLIKELY(locsFromDifferentRegion(loc1, loc2, vendorCode, locs, RESERVED)))
      return false;
  }

  return locsFromDifferentRegion(loc1, loc2, vendorCode, _interContinentalLocs, RESERVED);
}

bool
locsFromDifferentRegion(const Loc& loc1,
                        const Loc& loc2,
                        const VendorCode& vendorCode,
                        const LocList& locList,
                        ZoneType zoneType)
{
  for (LocList::const_iterator i = locList.begin(); i != locList.end() - 1; ++i)
  {
    for (LocList::const_iterator j = i + 1; j != locList.end(); ++j)
    {
      if ((isInLoc(loc1, i->first, i->second, vendorCode, zoneType) &&
           isInLoc(loc2, j->first, j->second, vendorCode, zoneType)) ||
          (isInLoc(loc2, i->first, i->second, vendorCode, zoneType) &&
           isInLoc(loc1, j->first, j->second, vendorCode, zoneType)))
      {
        return true;
      }
    }
  }
  return false;
}

bool
isFormerNetherlandsAntilles(const NationCode& nation)
{
  return (nation.equalToConst("SX") || nation.equalToConst("CW") || nation.equalToConst("BQ"));
}

uint32_t
getTPM(const Loc& originLoc,
       const Loc& destinationLoc,
       const GlobalDirection globalDirection,
       const DateTime& dateTime,
       DataHandle& dataHandle)
{
  const LocCode& originLocCode = originLoc.city().empty() ? originLoc.loc() : originLoc.city();
  const LocCode& destinationLocCode =
      destinationLoc.city().empty() ? destinationLoc.loc() : destinationLoc.city();

  // TODO We should avoid creating local DataHandle object once we remove setting ticketing date in
  // DataHandle while getting mileage from DB/Cache
  DataHandle dataHandleLocal;

  dataHandleLocal.setParentDataHandle(&dataHandle);

  uint32_t mileage = getMileage(
      originLocCode, destinationLocCode, TPM, globalDirection, dateTime, dataHandleLocal);

  if (mileage)
    return mileage;

  mileage = getMileage(
      originLocCode, destinationLocCode, MPM, globalDirection, dateTime, dataHandleLocal);

  if (mileage)
    return TseUtil::getTPMFromMPM(mileage);

  return TseUtil::greatCircleMiles(originLoc, destinationLoc);
}

LocCode
getCity(const Loc& loc)
{
  if (loc.cityInd() && loc.transtype() == 'P')
    return loc.loc();

  return getCity(loc.loc());
}

LocCode
getCity(const LocCode& airport)
{
  DataHandle dataHandle;
  LocCode city = dataHandle.getMultiTransportCity(airport);
  if (city.empty())
    return airport;

  return city;
}

//----------------------------------------------------------------------------
// isRussia()
//----------------------------------------------------------------------------
bool
isRussia(const Loc& loc)
{
  if (loc.nation() == RUSSIA || (loc.nation() == EAST_URAL_RUSSIA))
    return true;
  else
    return false;
}

bool
isJapan(const Loc& loc)
{
  return (loc.nation() == JAPAN);
}

bool
isKorea(const Loc& loc)
{
  return (loc.nation() == KOREA);
}

bool
isNigeria(const Loc& loc)
{
  return (loc.nation() == NIGERIA);
}

bool
isSamePoint(const Loc& loc1, const LocCode& city1, const Loc& loc2, const LocCode& city2)
{
  if (loc1.loc() == loc2.loc())
    return true;

  // Check MultiTransport Table
  //
  if (!city1.empty() && (city1 == city2))
  {
    // isSamePoint: MultiTransport Table returns TRUE
    return true;
  }

  return false;
}

bool
isSameISINation(const Loc& loc1, const Loc& loc2)
{
  return ((loc1.nation() == loc2.nation()) || // Single Nation
          (isDomesticUSCA(loc1) && isDomesticUSCA(loc2)) || // US/CA Group
          (isMinFareUSTerritory(loc1) && isMinFareUSTerritory(loc2)) || // US and US Territories
          (isScandinavia(loc1) && isScandinavia(loc2)) || // Scandinavia Group
          (isFrenchNationGroup(loc1) && isFrenchNationGroup(loc2)) || // French Nation Group
          (isRussianGroup(loc1) && isRussianGroup(loc2))); // Russia and E Ural Russia
}

bool
matchPaxStatus(const LocKey& paxLoc,
               const VendorCode& vendor,
               const Indicator& paxInd,
               const Indicator appl,
               StateCode& stateCode,
               const PricingTrx& trx,
               GeoTravelType geoTvlType)
{
  bool isMatch = false;
  const PricingOptions& option = *trx.getOptions();
  DataHandle& dh = trx.dataHandle();

  if (paxLoc.locType() == RuleConst::BLANK) // there is no GEO in rule
    isMatch = true;

  // "*(state)" qualifier (WPPGST*CO) can not be combined with
  // "EM/xx", "RY/xxyy" or "NT/xx" qualifiers (WP'EM/USCO).
  else if (!stateCode.empty())
  {
    // The US state code should be checked against the Pax_GEO
    std::string usState = "US"; // prepare state of US string.
    usState += stateCode;

    isMatch = matchPaxLoc(paxLoc, vendor, usState, dh, geoTvlType);
  }
  else
  {
    // check "ship registry/nationality/resedency/employee" passenger status
    const NationCode* optNation = nullptr;
    if (paxInd == PAX_NATIONALITY)
      optNation = &option.nationality();

    else if (paxInd == PAX_EMPLOYEE)
      optNation = &option.employment();

    else if (paxInd == PAX_SHIP_REGISTRY)
      optNation = &option.fareByRuleShipRegistry();

    if (paxInd == PAX_RESIDENCY)
    {
      const LocCode* optRes = &option.residency();
      if (*optRes == WILDCARD)
      {
        return true;
      }
      isMatch = matchPaxLoc(paxLoc, vendor, *optRes, dh, geoTvlType);
    }
    else
    {
      if (*optNation == WILDCARD)
      {
        return true;
      }
      isMatch = matchPaxLoc(paxLoc, vendor, *optNation, dh, geoTvlType);
    }

    /*
        if ( paxInd == RuleConst::PAX_NATIONALITY )
          isMatch = matchPaxLoc( paxLoc, vendor, option.nationality(), dh );

        else if ( paxInd == RuleConst::PAX_RESIDENCY )
          isMatch = matchPaxLoc( paxLoc, vendor, option.residency(), dh );

        else if ( paxInd ==RuleConst::PAX_EMPLOYEE )
          isMatch = matchPaxLoc( paxLoc, vendor, option.employment(), dh );

        else if ( paxInd ==RuleConst::PAX_SHIP_REGISTRY )
          isMatch = matchPaxLoc( paxLoc, vendor, option.fareByRuleShipRegistry(), dh );
          */
  }

  if ((isMatch && appl == NOT_ALLOWED) || (!isMatch && appl == RuleConst::BLANK))
  {
    return false;
  }
  return true;
}

bool
matchPaxLoc(const LocKey& paxLoc,
            const VendorCode& vendor,
            const std::string& paxNationOrState,
            DataHandle& dh,
            GeoTravelType geoTvlType,
            const DateTime& ticketingDate)
{
  const LocCode loc = paxLoc.loc();

  if (paxNationOrState.size() == 2) // Nation
  {
    switch (paxLoc.locType())
    {
    case LOCTYPE_NATION:
      return (paxNationOrState == loc);
      break;

    case LOCTYPE_AREA:
      return dh.isNationInArea(paxNationOrState, loc);
      break;

    case LOCTYPE_SUBAREA:
      return dh.isNationInSubArea(paxNationOrState, loc);
      break;

    case LOCTYPE_ZONE:
      return dh.isNationInZone(vendor, atoi(loc.c_str()), RESERVED, paxNationOrState);
      break;

    default:
      return false;
      break;
    }
  }

  if (paxNationOrState.size() == 4) // Nation + State
  {
    switch (paxLoc.locType())
    {
    case LOCTYPE_STATE:
      return (paxNationOrState == loc);
      break;

    case LOCTYPE_NATION:
      return (paxNationOrState.compare(0, loc.size(), loc, 0, loc.size()) == 0);
      break;

    case LOCTYPE_AREA:
      return dh.isStateInArea(paxNationOrState, loc);
      break;

    case LOCTYPE_SUBAREA:
      return dh.isStateInSubArea(paxNationOrState, loc);
      break;

    case LOCTYPE_ZONE:
      return dh.isStateInZone(vendor, atoi(loc.c_str()), RESERVED, paxNationOrState);
      break;

    default:
      return false;
      break;
    }
  }

  if (paxNationOrState.size() == 3) // Residency of city
  {

    return isInLoc(paxNationOrState,
                   paxLoc.locType(),
                   paxLoc.loc(),
                   vendor,
                   RESERVED,
                   geoTvlType,
                   OTHER,
                   ticketingDate);
  }

  // bad size
  return false;
}

/**
* Following method is for some specific use
* It can consider US/CA or Scandinavian Countries as one country depending on the
* Itin travel boundry
*/

bool
isWithinSameCountry(const GeoTravelType itinGeoTvlType,
                    const bool itinWithinScandinavia,
                    const Loc& loc1,
                    const Loc& loc2)

{

  if (itinGeoTvlType != GeoTravelType::International && itinGeoTvlType != GeoTravelType::Transborder)
  {
    return true;
  }
  if (loc1.nation() == loc2.nation()) // Single Nation
  {
    return true;
  }

  if ((isFrenchNationGroup(loc1) && isFrenchNationGroup(loc2)) || // French Nation
      (isRussianGroup(loc1) &&
       isRussianGroup(loc2)) || // Russia and E Ural Russia
      (isNetherlandsAntilles(loc1) &&
       isNetherlandsAntilles(loc2))) // AW, AN same Nation
  {
    return true;
  }

  if (itinGeoTvlType == GeoTravelType::International && !itinWithinScandinavia)
  {
    // some part could be within Scandinavia, in that case those 3 countries are 1 country
    // Also same part could be witin US and CA, in that case US/CA are 1 country
    // Itin is not Transborder either at this point
    //
    if (((loc1.nation() == NATION_US || loc1.nation() == NATION_CA) &&
         (loc2.nation() == NATION_US || loc2.nation() == NATION_CA)) ||
        (isScandinavia(loc1) && isScandinavia(loc2)))
    {
      // This part is within Scandinavia or within US-CA, Therefore within 1 country.
      //
      return true;
    }
  }

  return false;
}

bool
isWithinSameCountry(GeoTravelType itinGeoTvlType,
                    bool itinWithinScandinavia,
                    const Loc& loc1,
                    const Loc& loc2,
                    const Loc& outside1,
                    const Loc& outside2)
{
  if (isWithinSameCountry(itinGeoTvlType, itinWithinScandinavia, loc1, loc2))
  {
    return true;
  }
  if (GeoTravelType::International == itinGeoTvlType)
  {
    if (isEurope(loc1) && isEurope(loc2) && !(isEurope(outside1) && isEurope(outside2)))
    {
      return true;
    }
  }
  if (GeoTravelType::International == itinGeoTvlType)
  {
    if (UNLIKELY(oneOfNetherlandAntilles(loc1) && oneOfNetherlandAntilles(loc2) &&
        !(oneOfNetherlandAntilles(outside1) && oneOfNetherlandAntilles(outside2))))
    {
      return true;
    }
  }
  return false;
}

bool
isAllFlightsWithInScandinavia(const std::vector<TravelSeg*>& tvlSegs)
{
  if(tvlSegs.empty())
    return false;

  for (TravelSeg* tvl : tvlSegs)
  {
    const AirSeg* airSeg = dynamic_cast<AirSeg*>(tvl);

    if(airSeg &&
       (!isScandinavia(*(airSeg->origin())) ||
        !isScandinavia(*(airSeg->destination()))))
      return false;
  }
  return true;
}

//--------------------------------------------------------------------------------------
// isInSameCountry()
// according to IBF definition of the same country is
// US is USA, Virgin Islands and Puerto Rico
// Scandinavia is one country
// XU and RU is one country
//--------------------------------------------------------------------------------------
bool
isInSameCountry(const Loc& loc1, const Loc& loc2)
{
  if (loc1.nation() == loc2.nation())
    return true;
  if (isScandinavia(loc1) && isScandinavia(loc2))
    return true;
  if (isRussianGroup(loc1) && isRussianGroup(loc2))
    return true;
  if (isDomesticUS(loc1) && isDomesticUS(loc2))
    return true;

  return false;
}

//--------------------------------------------------------------------------------------
// isDomesticUS()
// According to IBF requirements US is US, Puerto Rico and Virgin Islands
//--------------------------------------------------------------------------------------

bool
isDomesticUS(const Loc& loc)
{
  if ((loc.nation() == UNITED_STATES) || (loc.state() == PUERTORICO) ||
      (loc.state() == VIRGIN_ISLAND))
  {
    return true;
  }

  return false;
}

//--------------------------------------------------------------------------------------

bool
isNetherlandsAntilles(const Loc& loc)
{
  static NationCode nations[] = { "AW", "AN" };

  return TseUtil::isMember(loc.nation(), nations);
}

bool
isStPierreMiquelon(const Loc& loc)
{
  return (loc.nation() == ST_PIERRE_MIQUELON);
}

bool
isChina(const Loc& loc)
{
  return CHINA == loc.nation();
}

bool
getUtcOffsetDifference(const Loc& loc1,
                       const Loc& loc2,
                       short& utcoffset,
                       DataHandle& dataHandle,
                       const DateTime& dateTime1,
                       const DateTime& dateTime2)
{
  return dataHandle.getUtcOffsetDifference(
      loc1.dstgrp(), loc2.dstgrp(), utcoffset, dateTime1, dateTime2);
}

bool
isRtw(const PricingUnit& pu)
{
  const std::vector<TravelSeg*>& tvlSeg = pu.travelSeg();
  if (UNLIKELY(tvlSeg.front()->origin()->nation() != tvlSeg.back()->destination()->nation()))
    return false;

  const IATAAreaCode& origArea = tvlSeg.front()->origin()->area();
  if (origArea == IATA_AREA1)
  {
    return isRtwOrigArea1(pu);
  }
  else if (LIKELY(origArea == IATA_AREA2 || origArea == IATA_AREA3))
  {
    return isRtwOrigArea2Or3(pu);
  }
  else
    return false;
}

bool
isRtwOrigArea1(const PricingUnit& pu)
{
  // check if Around the World (ATW): Travel from the point of origin and
  // return thereto, or travel from the point of origin and return to the
  // same country which involves only one crossing of the Atlantic Ocean,
  // and only one crossing of the Pacific Ocean.

  int paCnt = 0, atCnt = 0;
  for (std::vector<FareUsage*>::const_iterator i = pu.fareUsage().begin(),
                                               iend = pu.fareUsage().end();
       i != iend;
       ++i)
  {
    GlobalDirection gd = (*i)->paxTypeFare()->fareMarket()->getGlobalDirection();
    if (gd == GlobalDirection::PA)
      paCnt++;
    else if (gd == GlobalDirection::AT || gd == GlobalDirection::SA)
      atCnt++;
    else if (UNLIKELY(gd == GlobalDirection::AP))
      return false;
  }

  return (paCnt == 1 && atCnt == 1);
}

bool
isRtwOrigArea2Or3(const PricingUnit& pu)
{
  // check if Around the World (ATW): Travel from the point of origin and
  // return thereto, or travel from the point of origin and return to the
  // same country which involves only one crossing of the Atlantic Ocean,
  // and only one crossing of the Pacific Ocean.

  int paCnt = 0, atCnt = 0, apCnt = 0;
  for (std::vector<FareUsage*>::const_iterator i = pu.fareUsage().begin(),
                                               iend = pu.fareUsage().end();
       i != iend;
       ++i)
  {
    GlobalDirection gd = (*i)->paxTypeFare()->fareMarket()->getGlobalDirection();
    if (gd == GlobalDirection::PA)
      paCnt++;
    else if (gd == GlobalDirection::AT || gd == GlobalDirection::SA)
      atCnt++;
    else if (gd == GlobalDirection::AP)
      apCnt++;
  }

  if (UNLIKELY(apCnt > 0))
    return (paCnt == 0 && atCnt == 0);

  return (paCnt == 1 && atCnt == 1);
}

bool
getContinentalZoneCode(Zone& zoneCode, const Loc& loc1, const VendorCode& vendorCode)
{
  for (LocPair& loc : _interContinentalLocs)
  {
    if (isInLoc(loc1, loc.first, loc.second, vendorCode, RESERVED))
    {
      zoneCode = loc.second;
      return true;
    }
  }

  return false;
}

bool
isWithinSameCountryOJ(const GeoTravelType itinGeoTvlType,
                      const bool itinWithinScandinavia,
                      const Loc& loc1,
                      const Loc& loc2,
                      const Loc& otherSideLoc1,
                      const Loc& otherSideLoc2)
{
  if (isWithinSameCountry(itinGeoTvlType, itinWithinScandinavia, loc1, loc2))
  {
    return true;
  }
  const static Zone zone("210");
  if (UNLIKELY(isInLoc(
          loc1, LOCTYPE_ZONE, zone, Vendor::ATPCO, RESERVED, OTHER, GeoTravelType::International) &&
      isInLoc(
          loc2, LOCTYPE_ZONE, zone, Vendor::ATPCO, RESERVED, OTHER, GeoTravelType::International) &&
      !(isInLoc(otherSideLoc1,
                LOCTYPE_ZONE,
                zone,
                Vendor::ATPCO,
                RESERVED,
                OTHER,
                GeoTravelType::International) &&
        isInLoc(otherSideLoc2,
                LOCTYPE_ZONE,
                zone,
                Vendor::ATPCO,
                RESERVED,
                OTHER,
                GeoTravelType::International))))
  {
    return true;
  }
  return false;
}

bool
isWholeTravelInUSTerritory(const std::vector<TravelSeg*>& travelSegs)
{
  if (travelSegs.empty())
    return false;

  for (const TravelSeg* tvlSeg : travelSegs)
    if (!isBaggageUSTerritory(*(tvlSeg->origin())) ||
        !isBaggageUSTerritory(*(tvlSeg->destination())))
      return false;

  return true;
}

bool
isSpain(const Loc& loc)
{
  return (loc.nation() == SPAIN);
}

bool
isWholeTravelInSpain(const std::vector<TravelSeg*>& travelSegs)
{
  if (travelSegs.empty())
    return false;

  for (const TravelSeg* tvlSeg : travelSegs)
    if (!isSpain(*(tvlSeg->origin())) || !isSpain(*(tvlSeg->destination())))
      return false;

  return true;
}

bool
isCityInSpanishOverseaIslands(const LocCode& loc)
{
  return (loc == LOC_TCI || loc == LOC_PMI || loc == LOC_MLN ||
          loc == LOC_LPA || loc == LOC_FUE || loc == LOC_ACE ||
          loc == LOC_MAH || loc == LOC_IBZ || loc == LOC_JCU);
}

LocCode
getAtpcoNation(const Loc& loc)
{
  if (loc.state().equalToConst("USPR"))
    return "PR";
  if (loc.state().equalToConst("USVI"))
    return "VI";

  return loc.nation();
}

}// LocUtil

}// tse
