//----------------------------------------------------------------------------
//
//  Copyright Sabre 2003
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

#include "Common/TseConsts.h"
#include "Common/TsePrimitiveTypes.h"
#include "Routing/RoutingConsts.h"

#include <vector>

namespace tse
{

/** @class TravelRouteBuilder
 * preprocess all necessary information required by RoutingValidation Process.
 *
 * */
class FareMarket;
class PricingTrx;
class TravelRoute;
class TravelSeg;

class TravelRouteBuilder
{

public:
  friend class TravelRouteBuilderTest;
  TravelRouteBuilder();
  virtual ~TravelRouteBuilder();

  bool buildTravelRoute(const PricingTrx& trx, FareMarket& fareMarket, TravelRoute& tvlRoute);

  bool buildTravelRoute(const PricingTrx& trx,
                        FareMarket& fareMarket,
                        TravelRoute& tvlRoute,
                        const Indicator& unticketedPoint);

  bool buildTravelRoute(const PricingTrx& trx,
                        const std::vector<TravelSeg*>& tvlSegs,
                        TravelRoute& tvlRoute,
                        const Indicator& unticketedPoint);

  bool buildTempTravelRoute(const TravelRoute& oldTvlRoute, TravelRoute& tvlRoute,
                            const Indicator& unticketedPoint = TKTPTS_ANY);

  bool buildTravelRoutes(const PricingTrx& trx, FareMarket& fareMarket, TravelRoute& tvlRoute);

protected:
  void updateCityCarrier(const PricingTrx& trx, TravelRoute& tvlRoute) const;

private:
  bool fillCityCarrier(const std::vector<TravelSeg*>& tvlSegs, TravelRoute& tvlRoute);

  void setCarrierPreferences(FareMarket& fareMarket, TravelRoute& tvlRoute);
};
}
