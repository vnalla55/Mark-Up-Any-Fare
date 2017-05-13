#pragma once
#include "Routing/RestrictionValidator.h"

/**
* Process Restrictions 11 and 13
* */
namespace tse
{
class TravelRoute;
class RoutingRestriction;
class AirSurfaceRestrictionValidator : public RestrictionValidator
{
public:
  AirSurfaceRestrictionValidator();
  virtual ~AirSurfaceRestrictionValidator();

  bool validate(const TravelRoute& tvlRoute,
                const RoutingRestriction& rest,
                const PricingTrx& trx) override;

private:
  bool checkAnySurface(const TravelRoute& tvlRoute, TravelRouteItr city1I, TravelRouteItr city2I);

  bool checkAnyAir(const TravelRoute& tvlRoute, TravelRouteItr city1I, TravelRouteItr city2I);

  bool
  checkAllAirSectors(const TravelRoute& tvlRoute, TravelRouteItr city1I, TravelRouteItr city2I);

  bool
  checkAllSurfaceSectors(const TravelRoute& tvlRoute, TravelRouteItr city1I, TravelRouteItr city2I);
};
}
