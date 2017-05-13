//-------------------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#pragma once

#include "Routing/TravelRoute.h"

#include <vector>

namespace tse
{
class DataHandle;
class RoutingRestriction;
class PricingTrx;
class TravelSeg;
class ZoneInfo;

class RestrictionValidator
{
public:
  RestrictionValidator();
  virtual ~RestrictionValidator() = default;

  virtual bool
  validate(const TravelRoute& tvlRoute, const RoutingRestriction& rest, const PricingTrx& trx) = 0;


  typedef std::vector<TravelRoute::CityCarrier> TravelRouteList;
  typedef std::vector<TravelRoute::CityCarrier>::const_iterator TravelRouteItr;
  typedef std::vector<TravelSeg*>::const_iterator TravelSegItr;
  typedef std::vector<StateCode> StateCodeVec;
  typedef std::pair<Indicator, LocCode> MarketData;

  bool locateBetweenLocs(const PricingTrx& trx,
                         const TravelRoute& tvlRoute,
                         const MarketData& marketData1,
                         const MarketData& marketData2,
                         TravelRouteItr& city1I,
                         TravelRouteItr& city2I);

  bool searchLoc(const PricingTrx& trx,
                 const MarketData& viaMarketData,
                 TravelRouteItr city1I,
                 TravelRouteItr city2I);

  bool locateBetween(DataHandle& data,
                     const TravelRouteList& tvlRouteList,
                     const MarketData& marketData1,
                     const MarketData& marketData2,
                     TravelRouteItr& boardPoint,
                     TravelRouteItr& offPoint);

  bool searchViaPoint(DataHandle& data,
                      const MarketData& viaMarketData,
                      TravelRouteItr city1I,
                      TravelRouteItr city2I);

protected:
  bool checkPoint(DataHandle& data,
                  const LocCode& city,
                  const NationCode& nation,
                  const MarketData& marketData);

  bool
  locateBetweenLoc(DataHandle& data, TravelRoute::CityCarrier& cityCarrier,
                   const MarketData& marketData1, const MarketData& marketData2);

private:
  bool validateGenericCityCode(DataHandle& data, const LocCode& cityCode, const LocCode& market);
  bool validateZone(DataHandle& data, const LocCode& cityCode, const NationCode& nation);

  virtual const StateCode getState(DataHandle& data, const LocCode& cityCode);
  virtual const ZoneInfo* getZoneInfo(DataHandle& data, const LocCode& cityCode);

  StateCodeVec statesECC;
  StateCodeVec statesWCC;
};

} // namespace tse

