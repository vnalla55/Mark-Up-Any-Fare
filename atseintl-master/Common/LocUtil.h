//----------------------------------------------------------------------------
//  Description: Common Loc functions required for ATSE shopping/pricing.
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
#pragma once

#include "Common/LocUtilEnumeration.h"
#include "Common/TSSCacheCommon.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/Mileage.h"
#include "DBAccess/MultiAirportCity.h"
#include "DBAccess/ZoneInfo.h"

#include <vector>

namespace tse
{

class Loc;
class AirSeg;
class TravelSeg;
class PricingUnit;
class DataHandle;
class PricingTrx;

namespace LocUtil
{
  typedef std::pair<LocTypeCode, LocCode> LocPair;
  typedef std::vector<LocPair> LocList;

  bool isWithin(const LocKey& loc1,
                const Loc& origin,
                const Loc& destination,
                const DateTime& ticketingDate = DateTime::localTime());

  bool isBetween(const LocKey& loc1,
                 const LocKey& loc2,
                 const Loc& origin,
                 const Loc& destination,
                 const DateTime& ticketingDate = DateTime::localTime());

  bool isBetween(const LocKey& loc1,
                 const LocKey& loc2,
                 const Loc& origin,
                 const Loc& destination,
                 const VendorCode& vendor,
                 const ZoneType zoneType,
                 const ApplicationType applType = OTHER,
                 const DateTime& ticketingDate = DateTime::localTime());

  bool isFrom(const LocKey& loc1,
              const LocKey& loc2,
              const Loc& origin,
              const Loc& destination,
              const DateTime& ticketingDate = DateTime::localTime());

  bool isInLocImpl(const Loc& loc,
                   LocTypeCode locType,
                   const LocCode& locCode,
                   const VendorCode& vendorCode,
                   ZoneType zoneType,
                   ApplicationType applType,
                   GeoTravelType geoTvlType,
                   const CarrierCode& carrier,
                   DateTime ticketingDate);

  inline bool isInLoc(const Loc& loc,
                      LocTypeCode locType,
                      const LocCode& locCode,
                      const VendorCode& vendorCode = EMPTY_VENDOR,
                      ZoneType zoneType = RESERVED,
                      ApplicationType applType = OTHER,
                      GeoTravelType geoTvlType = GeoTravelType::International,
                      const CarrierCode& carrier = "",
                      DateTime ticketDate = DateTime::localTime())
  {
    return isInLocImpl(loc, locType, locCode, vendorCode, zoneType, applType, geoTvlType, carrier, ticketDate);
  }

  bool isAirportInLoc(const Loc& loc,
                      const LocTypeCode& locTypeCode,
                      const LocCode& locCode,
                      const VendorCode& vendorCode = EMPTY_VENDOR,
                      const ZoneType zoneType = RESERVED,
                      GeoTravelType geoTvlType = GeoTravelType::International,
                      const DateTime& ticketingDate = DateTime::localTime(),
                      ApplicationType applType = OTHER);

  inline bool isInLoc(const LocCode& city,
                      LocTypeCode locType,
                      const LocCode& locCode,
                      const VendorCode& vendor = EMPTY_VENDOR,
                      ZoneType zoneType = RESERVED,
                      GeoTravelType geoTvlType = GeoTravelType::International,
                      ApplicationType applType = OTHER,
                      DateTime ticketDate = DateTime::localTime())
  {
    return tsscache::isInLoc(
      city, locType, locCode, vendor, zoneType, geoTvlType, applType, ticketDate);
  }
  bool isInternational(const Loc& loc1, const Loc& loc2); // travel between two country
  bool isForeignDomestic(const Loc& loc1, const Loc& loc2); // travel within same country
  bool isDomestic(const Loc& loc1, const Loc& loc2); // travel within US

  bool isDomesticUSCA(const Loc& loc); // US, CA, PR, VI
  inline bool isDomesticUSCA(const Loc& loc1, const Loc& loc2)
  {
    return isDomesticUSCA(loc1) && isDomesticUSCA(loc2);
  }
  bool isDomesticRussia(const Loc& loc1, const Loc& loc2);

  bool isUSTerritory(const Loc& loc1, const Loc& loc2); // travel within US, VI, PR

  bool isWithinZone210(const Loc& loc1, const Loc& loc2); // within DEN/NO/SE

  bool isTransBorder(const Loc& loc1, const Loc& loc2); // travel within US and CA

  bool isOverWater(const Loc& loc1, const Loc& loc2);

  bool isScandinavia(const Loc& loc); // travel within Norway, Denmark, Sweden

  bool isEurope(const Loc& loc);

  bool
  isMiddleEast(const Loc& loc);

  bool oneOfNetherlandAntilles(const Loc& loc);

  bool isFrenchNationGroup(const Loc& loc); // travel within FR/GP/MQ/GF/PM/YT/RE/KM/NC/PF/WF

  bool isRussianGroup(const Loc& loc);

  bool isUSTerritoryOnly(const Loc& loc); // Is a US territory(PR/VI) but not the US

  bool isUSPossession(const Loc& loc); // Is a US possesion but not the US

  bool isUSTerritoryRule(const Loc& loc); // Is a US territory for Rule: US(PR,VI),CA,GU.

  bool isMinFareUSTerritory(const Loc& loc); // checks US territory for MinimumFares.

  bool isBaggageUSTerritory(const Loc& loc);
  /**
   * Following method is for some specific use
   * It can consider US/CA or Scandinavian Countries as one country depending on the
   * Itin travel boundry
   */
  bool isWithinSameCountry(const GeoTravelType itinGeoTvlType,
                           const bool itinWithinScandinavia,
                           const Loc& loc1,
                           const Loc& loc2);

  bool isWithinSameCountry(GeoTravelType itinGeoTvlType,
                           bool itinWithinScandinavia,
                           const Loc& loc1,
                           const Loc& loc2,
                           const Loc& outside1,
                           const Loc& outside2);

  bool isAllFlightsWithInScandinavia(const std::vector<TravelSeg*>& tvlSegs);

  bool isInSameCountry(const Loc& loc1, const Loc& loc2);

  bool isDomesticUS(const Loc& loc);

  bool isAirportInCity(const LocCode& airport,
                       const LocCode& city,
                       const CarrierCode& carrier = "",
                       const GeoTravelType geoTvlType = GeoTravelType::International,
                       const DateTime& ticketingDate = DateTime::localTime());

  LocCode getMultiTransportCity(const LocCode& locCode,
                                const CarrierCode& carrierCode,
                                GeoTravelType tvlType,
                                const DateTime& tvlDate);

  LocCode getMultiTransportCity(const LocCode& locCode, const AirSeg& airSeg);

  bool isSameMultiTransportCity(const LocCode& loc1,
                                const LocCode& loc2,
                                const CarrierCode& cxr,
                                const GeoTravelType tvlType,
                                const DateTime& tvlDate);

  const std::vector<tse::MultiAirportCity*>& getMultiCityAirports(const LocCode& locCode,
                                                                  PricingTrx* trx = nullptr);

  bool isMultiAirport(const LocCode& locCode,
                      PricingTrx* trx = nullptr);

  /**
   * @function isSameCity
   * Returns true if both locations belong to the same multi-airport city; false otherwise.
   */
  bool isSameCity(const LocCode&, const LocCode&, DataHandle&);

  bool isUS(const Loc& loc); // Is United States

  bool isCanada(const Loc& loc);

  bool isColumbia(const Loc& loc);

  bool isMexico(const Loc& loc);

  bool isHawaii(const Loc& loc);

  bool isAlaska(const Loc& loc);

  bool isPuertoRico(const Loc& loc);

  bool isTransAtlantic(const Loc& loc1, const Loc& loc2, const GlobalDirection& dir);

  bool isTransPacific(const Loc& loc1, const Loc& loc2, const GlobalDirection& dir);

  bool isTransOceanic(const Loc& loc1, const Loc& loc2, const GlobalDirection& dir);

  void
  getOverWaterSegments(const std::vector<TravelSeg*>& inSeg, std::vector<TravelSeg*>& outSeg);

  void
  getInternationalSegments(const std::vector<TravelSeg*>& inSeg, std::vector<TravelSeg*>& outSeg);

  bool isInterContinental(const Loc& loc1, const Loc& loc2, const VendorCode& vendorCode);

  bool isFormerNetherlandsAntilles(const NationCode& nation);

  uint32_t getTPM(const Loc& origin,
                  const Loc& destination,
                  const GlobalDirection globalDirection,
                  const DateTime& dateTime,
                  DataHandle& dataHandle);

  inline uint32_t getMileage(const LocCode& originLocCode,
                             const LocCode& destinationLocCode,
                             Indicator mileageType,
                             const GlobalDirection globalDirection,
                             const DateTime& dateTime,
                             DataHandle& dataHandle)
  {
    const Mileage* mileage = dataHandle.getMileage(
        originLocCode, destinationLocCode, mileageType, globalDirection, dateTime);

    return mileage ? mileage->mileage() : 0;
  }

  LocCode getCity(const Loc& loc);
  LocCode getCity(const LocCode& airport);

  bool isRussia(const Loc& loc);
  bool isJapan(const Loc& loc);
  bool isKorea(const Loc& loc);
  bool isNigeria(const Loc& loc);
  bool isNetherlandsAntilles(const Loc& loc);
  bool isStPierreMiquelon(const Loc& loc);
  bool isChina(const Loc& loc);

  bool
  isSamePoint(const Loc& loc1, const LocCode& city1, const Loc& loc2, const LocCode& city2);

  bool isSameISINation(const Loc& loc1, const Loc& loc2);

  bool isInZone(const Loc& locFrom,
                const Loc& locTo,
                const VendorCode& vendor,
                const Zone& zone,
                const ZoneType zoneType,
                const DateTime& ticketingDate = DateTime::localTime());

  bool matchPaxLoc(const LocKey& paxLoc,
                   const VendorCode& vendor,
                   const std::string& paxNationOrState,
                   DataHandle& dh,
                   GeoTravelType geoTvlType = GeoTravelType::International,
                   const DateTime& ticketingDate = DateTime::localTime());

  bool matchPaxStatus(const LocKey& paxLoc,
                      const VendorCode& vendor,
                      const Indicator& paxInd,
                      const Indicator appl,
                      StateCode& stateCode,
                      const PricingTrx& trx,
                      GeoTravelType geoTvlType = GeoTravelType::International);

  bool isInZone(const Loc& loc,
                const VendorCode& vendor,
                const Zone& zoneCode,
                const ZoneType zoneType,
                ApplicationType applType = OTHER,
                GeoTravelType geoTvlType = GeoTravelType::International,
                const CarrierCode& carrier = "",
                const DateTime& ticketingDate = DateTime::localTime());

  bool areNationsInSameATPReservedZone(const NationCode& nation1, const NationCode& nation2);

  bool locsFromDifferentRegion(const Loc& loc1,
                               const Loc& loc2,
                               const VendorCode& vendorCode,
                               const LocList& locList,
                               ZoneType zoneType);

  bool isWithinOneATPReservedZone(const std::vector<TravelSeg*>& travel);

  bool getUtcOffsetDifference(const Loc& loc1,
                              const Loc& loc2,
                              short& utcoffset,
                              DataHandle& dataHandle,
                              const DateTime&,
                              const DateTime&);

  void padZoneNo(Zone& zone);

  bool isRtw(const PricingUnit& pu);

  bool isRtwOrigArea1(const PricingUnit& pu);

  bool isRtwOrigArea2Or3(const PricingUnit& pu);

  bool getContinentalZoneCode(Zone& zoneCode, const Loc& loc, const VendorCode& vendor);

  bool isWithinSameCountryOJ(const GeoTravelType itinGeoTvlType,
                             const bool itinWithinScandinavia,
                             const Loc& loc1,
                             const Loc& loc2,
                             const Loc& otherSideLoc1,
                             const Loc& otherSideLoc2);

  bool isWholeTravelInUSTerritory(const std::vector<TravelSeg*>& travelSegs);

  bool isSpain(const Loc& loc);
  bool isWholeTravelInSpain(const std::vector<TravelSeg*>& travelSegs);
  bool isCityInSpanishOverseaIslands(const LocCode& loc);

  LocCode getAtpcoNation(const Loc&);

  int matchZone(DataHandle& dataHandle,
                const Loc& loc,
                const ZoneInfo* zoneInfo,
                ApplicationType applType = OTHER,
                GeoTravelType geoTvlType = GeoTravelType::International,
                const CarrierCode& carrier = "",
                const DateTime& ticketingDate = DateTime::localTime());

  int matchUniformZone(const Loc& loc,
                       const ZoneInfo* zone,
                       ApplicationType applType = OTHER,
                       GeoTravelType geoTvlType = GeoTravelType::International,
                       const CarrierCode& carrier = "",
                       const DateTime& ticketingDate = DateTime::localTime());

  bool isLocInZone(const Loc& loc,
                   const VendorCode& vendor,
                   const Zone& zone,
                   const ZoneType zoneType,
                   ApplicationType applType = OTHER,
                   GeoTravelType geoTvlType = GeoTravelType::International,
                   const CarrierCode& carrier = "",
                   const DateTime& ticketingDate = DateTime::localTime());

  // Flags to represent directional qualifiers
  const uint8_t MATCHES_BETWEEN = 0x01; // 'B'
  const uint8_t MATCHES_AND = 0x02; // 'A'
  const uint8_t MATCHES_FROM = 0x08; // 'F'
  const uint8_t MATCHES_TO = 0x04; // 'T'
  const uint8_t MATCHES_NONE = 0x10; // 'N' = no direction specified

  uint8_t matchZone(const Loc& loc, const std::vector<ZoneInfo::ZoneSeg>& zoneInfo);
  void flipBit(uint8_t& ret, uint8_t currentType, Indicator inclExclInd);

  typedef std::pair<Zone, Zone> ZonePair;
  typedef std::vector<Zone> ZoneList;
  typedef std::vector<ZonePair> ZonePairList;

  const char PAX_RESIDENCY = 'R';
  const char PAX_EMPLOYEE = 'E';
  const char PAX_NATIONALITY = 'N';
  const char PAX_SHIP_REGISTRY = 'S';
  const char NOT_ALLOWED = 'N';

}// LocUtil

} // end tse namespace

