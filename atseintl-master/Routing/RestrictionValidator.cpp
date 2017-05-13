//-------------------------------------------------------------------------------
// Copyright 2013, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "Routing/RestrictionValidator.h"

#include "Common/FallbackUtil.h"
#include "Common/LocUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/Vendor.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/RoutingRestriction.h"
#include "DBAccess/ZoneInfo.h"
#include "Routing/MapNode.h"

namespace tse
{

RestrictionValidator::RestrictionValidator()
{
  statesWCC = {"USCA", "USOR", "USWA"};
  statesECC = {"USNY",
               "USPA",
               "USFL",
               "USMA",
               "USGA",
               "USVA",
               "USNJ",
               "USME",
               "USCT",
               "USRI",
               "USVT",
               "USNH",
               "USMD",
               "USNC",
               "USSC"};
}

bool
RestrictionValidator::locateBetweenLocs(const PricingTrx& trx,
                                        const TravelRoute& tvlRoute,
                                        const MarketData& marketData1,
                                        const MarketData& marketData2,
                                        TravelRouteItr& city1I,
                                        TravelRouteItr& city2I)
{
  const LocCode& market1 = marketData1.second;
  const LocCode& market2 = marketData2.second;

  if (trx.getOptions()->isRtw())
    return locateBetween(trx.dataHandle(), tvlRoute.travelRoute(), marketData1,
                         marketData2, city1I, city2I);

  return locateBetween(trx.dataHandle(), tvlRoute.travelRoute(), std::make_pair('C', market1),
                       std::make_pair('C', market2), city1I, city2I);
}

bool
RestrictionValidator::searchLoc(const PricingTrx& trx,
                                const MarketData& viaMarketData,
                                TravelRouteItr city1I,
                                TravelRouteItr city2I)
{
  if (UNLIKELY(trx.getOptions()->isRtw()))
    return searchViaPoint(trx.dataHandle(), viaMarketData, city1I, city2I);

  return searchViaPoint(trx.dataHandle(), std::make_pair('C', viaMarketData.second), city1I, city2I);
}

bool RestrictionValidator::locateBetween(DataHandle& data,
                                         const TravelRouteList& tvlRouteList,
                                         const MarketData& marketData1,
                                         const MarketData& marketData2,
                                         TravelRouteItr& boardPoint,
                                         TravelRouteItr& offPoint)
{
  if(marketData1.second.empty() && marketData2.second.empty())
      return false;

  MarketData market;
  for(TravelRouteItr it = tvlRouteList.begin(); it != tvlRouteList.end(); ++it)
  {
    bool boardPointMarket1 = checkPoint(data, it->boardCity().loc(), it->boardNation(), marketData1);
    bool boardPointMarket2 = checkPoint(data, it->boardCity().loc(), it->boardNation(), marketData2);

    if(boardPointMarket1 || boardPointMarket2)
    {
      if(boardPointMarket1)
        market = marketData2;
      else
        market = marketData1;

      boardPoint = it;

      for(TravelRouteItr boardPointTmp = boardPoint; boardPointTmp != tvlRouteList.end(); ++boardPointTmp)
      {
        if(checkPoint(data, boardPointTmp->offCity().loc(), boardPointTmp->offNation(), market))
        {
          offPoint = ++boardPointTmp;
          return true;
        }
      }

    }
  }
  return false;
}

bool RestrictionValidator::checkPoint(DataHandle& data,
                                      const LocCode& city,
                                      const NationCode& nation,
                                      const MarketData& marketData)
{
    switch(marketData.first)
    {
        case MapNode::CITY:
        {
          if (LIKELY(marketData.second != EastCoastCode && marketData.second != WestCoastCode))
                return city == marketData.second;

            return validateGenericCityCode(data, city, marketData.second);
        }
        case MapNode::NATION:
            return nation == marketData.second;
        case MapNode::ZONE:
            return validateZone(data, marketData.second, nation);
        default:
            return false;
    }
}

const StateCode RestrictionValidator::getState(DataHandle& data, const LocCode& cityCode)
{
    return data.getLoc(cityCode, DateTime::localTime())->state();
}

bool RestrictionValidator::validateGenericCityCode(DataHandle& data, const LocCode& cityCode,
                                                   const LocCode& market)
{
    const StateCode state = getState(data, cityCode);
    StateCodeVec::const_iterator isGenericCity;

    if (market == WestCoastCode)
    {
        isGenericCity = std::find(statesWCC.begin(), statesWCC.end(), state);
        if(isGenericCity != statesWCC.end())
            return true;
    }
    else if (market == EastCoastCode)
    {
        isGenericCity = std::find(statesECC.begin(), statesECC.end(), state);
        if(isGenericCity != statesECC.end())
            return true;
    }
    return false;
}

const ZoneInfo* RestrictionValidator::getZoneInfo(DataHandle& data, const LocCode& cityCode)
{
    Zone zone(cityCode);
    LocUtil::padZoneNo(zone);
    return data.getZone(Vendor::ATPCO, zone, RESERVED, DateTime::localTime());
}

bool RestrictionValidator::validateZone(DataHandle& data,
                                        const LocCode& cityCode,
                                        const NationCode& nation)
{
    const ZoneInfo* zoneInfo = getZoneInfo(data, cityCode);
    if(!zoneInfo)
        return false;

    for (const std::vector<ZoneInfo::ZoneSeg>& zoneSegs : zoneInfo->sets())
      for (const ZoneInfo::ZoneSeg& zoneSeg : zoneSegs)
        if (zoneSeg.locType() == LOCTYPE_NATION && zoneSeg.loc() == nation)
          return true;

    return false;
}

bool RestrictionValidator::searchViaPoint(DataHandle& data,
                                          const MarketData& viaMarketData,
                                          TravelRouteItr city1I,
                                          TravelRouteItr city2I)
{
    for (; city1I != city2I; ++city1I)
        if (checkPoint(data, city1I->offCity().loc(), city1I->offNation(), viaMarketData))
            return true;
    return false;
}


bool
RestrictionValidator::locateBetweenLoc(DataHandle& data, TravelRoute::CityCarrier& cityCarrier,
                                       const MarketData& marketData1,
                                       const MarketData& marketData2)
{
  MarketData market;

  bool boardPointMarket1 = checkPoint(data, cityCarrier.boardCity().loc(),
      cityCarrier.boardNation(), marketData1);
  bool boardPointMarket2 = checkPoint(data, cityCarrier.boardCity().loc(),
      cityCarrier.boardNation(), marketData2);

  if (boardPointMarket1 || boardPointMarket2)
  {
    if (boardPointMarket1)
      market = marketData2;
    else
      market = marketData1;

    if (checkPoint(data, cityCarrier.offCity().loc(), cityCarrier.offNation(), market))
      return true;
  }
  return false;
}

} // namespace tse
