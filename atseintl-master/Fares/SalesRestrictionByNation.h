#pragma once

#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

namespace tse
{

class Itin;
class FareMarket;
class TravelSeg;
class SalesNationRestr;
class Loc;
class LocKey;
class PricingTrx;
class DiagCollector;

class SalesRestrictionByNation
{
  friend class SalesRestrictionByNationTest;
public:
  bool isRestricted(Itin& itin, FareMarket& fareMarket, PricingTrx& trx);

private:
  bool anyRestriction(const SalesNationRestr& restriction,
                      const Itin& itin,
                      FareMarket& fareMarket,
                      PricingTrx& trx,
                      std::set<CarrierCode>& restrictedValCxrs);

  bool matchUser(const Indicator& userApplType, const UserApplCode& userAppl, PricingTrx& trx);

  bool matchCarrier(const std::vector<CarrierCode>& cxrs,
                    const Indicator& exceptCxr,
                    const CarrierCode& cxr);
  bool matchCarrier(const Itin& itin,
                    const std::vector<CarrierCode>& restrictedCxrs,
                    const Indicator& exceptCxr,
                    const std::vector<CarrierCode>& validatingCxrs,
                    std::set<CarrierCode>& matchedCxrs);

  bool matchDirectionality(PricingTrx& trx,
                           const SalesNationRestr& restriction,
                           const Itin& itin,
                           const FareMarket& fareMarket);

  bool matchGlobalDirection(const GlobalDirection& restrictionGlobalDir,
                            const GlobalDirection& globalDirection);

  bool matchTravelType(const Indicator& travelType, const GeoTravelType& itinTravelType);

  bool matchVia(PricingTrx& trx, const LocKey& loc, const FareMarket& fareMarket);

  bool matchCurrency(const std::vector<CurrencyCode>& curRstrs, const FareMarket& fareMarket);

  bool matchBetween(PricingTrx& trx,
                    const LocKey& loc1,
                    const LocKey& loc2,
                    const FareMarket& fareMarket);

  bool matchFrom(PricingTrx& trx,
                 const LocKey& loc1,
                 const LocKey& loc2,
                 const Loc& origin,
                 const Loc& destination,
                 GeoTravelType geoTvlType);

  bool matchWithin(PricingTrx& trx,
                   const LocKey& loc,
                   const std::vector<TravelSeg*>& travelSegs,
                   GeoTravelType geoTvlType);

  bool matchSITILoc(PricingTrx& trx,
                    const Indicator& exceptLoc,
                    const LocKey& locKey,
                    const Loc& loc,
                    GeoTravelType geoTvlType);

  bool matchLoc(PricingTrx& trx, const LocKey& locKey, const Loc& loc, GeoTravelType geoTvlType);

  void printText(SalesNationRestr& restriction, PricingTrx& trx);

  void displayRestrItem(SalesNationRestr& restr, DiagCollector& diag);
};

} // namespace tse

