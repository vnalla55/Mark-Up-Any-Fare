#pragma once
#include "Routing/RestrictionValidator.h"

// Process Restriction 7

namespace tse
{
class TravelRoute;
class RoutingRestriction;
class StopOverRestrictionValidator : public RestrictionValidator
{
public:
  StopOverRestrictionValidator();
  virtual ~StopOverRestrictionValidator();

  bool validate(const TravelRoute& tvlRoute,
                const RoutingRestriction& rest,
                const PricingTrx& trx) override;

private:
  bool searchForStopover(const TravelRoute& tvlRoute,
                         const LocCode& viaMarket,
                         TravelRouteItr city1I,
                         TravelRouteItr city2I);
};
}
