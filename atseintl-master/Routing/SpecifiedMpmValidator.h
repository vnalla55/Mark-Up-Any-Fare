#pragma once

#include "Routing/RestrictionValidator.h"

// Process Restriction 8

namespace tse
{

class SpecifiedMpmValidator : public RestrictionValidator
{
public:
  SpecifiedMpmValidator();
  virtual ~SpecifiedMpmValidator();

  bool validate(const TravelRoute& tvlRoute, const RoutingRestriction& rest);
  bool validate(const TravelRoute& tvlRoute,
                const RoutingRestriction& rest,
                const PricingTrx& trx) override;
};
}
