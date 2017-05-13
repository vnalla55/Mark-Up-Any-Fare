#pragma once

#include "Routing/RestrictionValidator.h"

namespace tse
{
class RoutingRestriction;
class TravelRoute;
class PricingTrx;

class TravelRestrictionValidator : public RestrictionValidator
{
public:
  TravelRestrictionValidator();
  virtual ~TravelRestrictionValidator();

  virtual bool validate(const TravelRoute& tvlRoute,
                        const RoutingRestriction& rest,
                        const PricingTrx& trx) override;

private:
  bool
  validateImpl(const TravelRoute& tvlRoute, const RoutingRestriction& rest, const PricingTrx& trx);
};
}
