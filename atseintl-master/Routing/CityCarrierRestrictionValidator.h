//----------------------------------------------------------------------------
//  Copyright Sabre 2003
//
//  CityCarrierRestrictionValidator.cpp
//
//  Description:  Validate Routing Restrictions 1, 2, 5, 18, 19, and 21
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

#include "Routing/RestrictionValidator.h"

namespace tse
{
class RoutingRestriction;
class PricingTrx;
class TravelRoute;

class CityCarrierRestrictionValidator : public RestrictionValidator
{
public:
  CityCarrierRestrictionValidator();
  virtual ~CityCarrierRestrictionValidator();

  bool validate(const TravelRoute& travelRoute,
                const RoutingRestriction& restriction,
                const PricingTrx& trx) override;

private:
  bool
  restrictionValid(const RoutingRestriction& restriction, bool viaCityFound, bool viaCarrierFound);

  bool checkAllCarriers(const TravelRoute& tvlRoute,
                        const CarrierCode& carrier,
                        TravelRouteItr city1I,
                        TravelRouteItr city2I);

  bool searchCarrier(const TravelRoute& tvlRoute,
                     const CarrierCode& carrier,
                     TravelRouteItr city1I,
                     TravelRouteItr city2I);

  bool searchToFromCity1ViaCity3(const PricingTrx& trx,
                                 const RoutingRestriction& restriction,
                                 const TravelRoute& tvlRoute);

  bool searchToFromCarrier(const TravelRoute& tvlRoute,
                           const LocCode& city1,
                           const CarrierCode& viaCarrier);
};

} // namespace tse

