#pragma once

#include "Routing/RestrictionValidator.h"


namespace tse
{
class RoutingRestriction;
class PricingTrx;
class TravelRoute;

class StopTypeRestrictionValidator : public RestrictionValidator
{
public:
  friend class StopTypeRestrictionValidatorTest;

  bool validate(const TravelRoute& tvlRoute,
                const RoutingRestriction& restriction,
                const PricingTrx& trx) override;

  uint32_t countNonStops(DataHandle& data,
                         const TravelRouteList& tvlRouteList,
                         const MarketData& marketData1,
                         const MarketData& marketData2);

protected:
  virtual bool isDirect(const TravelRoute& tvlRoute);
  virtual bool isNonStop(const TravelRoute& tvlRoute);
  bool validateStopType(const TravelRoute& tvlRoute, const RoutingRestriction& restriction);
  bool findCity(DataHandle& data, const TravelRouteList& tvlRouteList,
                TravelRouteItr& itr, const MarketData& marketData);

private:
  bool validateRestriction3(const TravelRoute& tvlRoute, const RoutingRestriction& restriction,
                            const PricingTrx& trx);
  bool validateRestriction4(const TravelRoute& tvlRoute, const RoutingRestriction& restriction,
                            const PricingTrx& trx);
  bool validateRestriction6(const TravelRoute& tvlRoute, const RoutingRestriction& restriction,
                            const PricingTrx& trx);


};
}
