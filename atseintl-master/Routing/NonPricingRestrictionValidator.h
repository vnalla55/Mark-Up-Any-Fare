#pragma once
#include "Routing/RestrictionValidator.h"

/**
 * Process Restrictions 14 and 15
 * */
namespace tse
{

class TravelRoute;
class RoutingRestriction;
class NonPricingRestrictionValidator : public RestrictionValidator
{
public:
  //------------------------------------------------------------------------------------------
  // Restrictions 14:  BETWEEN CITY1 AND CITY2 NO LOCAL TRAFFIC PERMITTED
  // Restrictions 15:  BAGGAGE MUST BE CHECKED THROUGH TO DESTINATION ONLY
  //------------------------------------------------------------------------------------------
  bool
  validate(const TravelRoute& tvlRoute, const RoutingRestriction& rest, const PricingTrx&) override
  {
    return true;
  }
};
}
