//-------------------------------------------------------------------
//  File : DRVController.h
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
//-------------------------------------------------------------------

#pragma once
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Fares/RoutingController.h"
#include "Routing/RoutingInfo.h"


#include <map>
#include <vector>

namespace tse
{

class Routing;
class PricingTrx;
class Fare;
class FareMarket;
class PaxTypeFare;
class TravelRoute;

/**
* @class DRVController
* Manages the Domestic Routing Validation (DRV) process for an international
* TravelRoute when the initial attempt to validate the Specified Route
* map has failed, and RoutingController has determined that Domestic
* Routing Validation may be performed.
*
* Primary responsibilities are to select the highest oneway fare in the
* matching fare market, get the routing and call Specified Routing Validation
* again with the local carrier's routing and corresponding travel route.
*
* */

class DRVController : public RoutingController
{

  friend class DRVControllerTest;

public:
  DRVController(PricingTrx& trx) : RoutingController(trx) {}

  /**
  * Description of method.
  * @param trx PricingTrx A reference to the Itin object.
  * @param fareMarket FareMarket A reference to FareMarket object.
  * @returns bool
  *
  * */

  bool process(PricingTrx& trx,
               PaxTypeFare& paxTypeFare,
               const Routing* routing,
               const Routing* origAddOnRouting,
               const Routing* destAddOnRouting,
               const TravelRoute& tvlRoute,
               MapInfo* mapInfo);

private:
  bool performDRV(PricingTrx& trx,
                  PaxTypeFare& paxTypeFare,
                  const Routing* routing,
                  const TravelRoute& tvlRoute,
                  MapInfo* mapInfo,
                  DRVInfo* drvInfo);

  bool carrierAllowsDRV(PricingTrx& trx, int16_t missingCityIndex, const TravelRoute& tvlRoute);

  void addHiddenStopLocation(TravelRoute& tvlRoute,
                             bool& flightStopMarket,
                             std::vector<TravelSeg*>& localTvlSegs,
                             PricingTrx& trx,
                             const CarrierCode& carrier,
                             const TravelSeg* tvlSeg,
                             bool isFirstOrig,
                             const Loc& missingCityLoc,
                             const DateTime& travelDate) const;

  bool buildLocalTravelRoute(PricingTrx& trx,
                             const TravelRoute& tvlRoute,
                             int16_t cityIndex,
                             TravelRoute& localTvlRoute,
                             DRVInfo& drvInfo);

  const FareMarket*
  findMatchingFareMarket(PricingTrx& trx, TravelRoute& localTvlRoute, const DateTime& date);

  const FareMarket* buildFareMarket(PricingTrx& trx, TravelRoute& localTvlRoutei);

  PaxTypeFare* selectFare(const FareMarket& fareMarket,
                          DRVInfo& drvInfo,
                          MoneyAmount& highestAmount,
                          int& fareCount);

  bool buildIntlTravelRoute(PricingTrx& trx,
                            const TravelRoute& tvlRoute,
                            TravelRoute& intlTvlRoute,
                            int16_t missingCityIndex);

  bool validateLocalRouting(PricingTrx& trx,
                            PaxTypeFare& paxTypeFare,
                            TravelRoute& tvlRoute,
                            const Routing* routing,
                            DRVInfo* drvInfo);

  void createNewTvlSeg(PricingTrx& trx, TravelSeg* tvlSeg, TravelSeg* newTvlSeg);

  bool createNewTvlSeg(PricingTrx& trx,
                       std::vector<TravelSeg*>& localTvlSegs,
                       const LocCode& boardCity,
                       const LocCode& offCity,
                       const CarrierCode& carrier,
                       const DateTime& travelDate) const;

  bool validateIntlRouting(PricingTrx& trx,
                           TravelRoute& tvlRoute,
                           const Routing* routing,
                           const Routing* origAddOnRouting,
                           const Routing* destAddOnRouting,
                           MapInfo* mapInfo,
                           const DateTime& travelDate);

  bool
  checkHiddenStopLocation(const Loc& hiddenStop, const Loc& missingCity, LocCode& boardCity) const;

  bool checkMissingCity(PricingTrx& trx, int16_t missingCityIndex, const TravelRoute& tvlRoute);

  void setRoutingTariff(const Routing* localRtg, DRVInfo* drvInfo);

};
}

