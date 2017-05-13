//----------------------------------------------------------------------------
//  Copyright Sabre 2014
//
//  TravelRestrictionValidator.cpp
//
//  Description:  Validate Routing Restrictions 9 and 10
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

#include "Routing/TravelRestrictionValidator.h"

#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/RoutingRestriction.h"
#include "Routing/RestrictionValidator.h"
#include "Routing/TravelRoute.h"
#include "Routing/TravelRouteBuilder.h"

namespace tse
{

namespace
{

class CheckCityPair
{
public:
  CheckCityPair(LocCode& loc): _origin(loc) {}

  bool operator()(const TravelRoute::CityCarrier& cityCarrier) const
  {
    return cityCarrier.boardCity().loc() == _origin || cityCarrier.offCity().loc() == _origin;
  }
private:
  LocCode& _origin;
};

}

TravelRestrictionValidator::TravelRestrictionValidator() {}
TravelRestrictionValidator::~TravelRestrictionValidator() {}

bool
TravelRestrictionValidator::validate(const TravelRoute& tvlRoute, const RoutingRestriction& rest,
                                     const PricingTrx& trx)
{
  if(!trx.getOptions()->isRtw())
    return true;

  return validateImpl(tvlRoute, rest, trx);
}

//------------------------------------------------------------------------------------------
// Restrictions  9:  TRAVEL IS NOT PERMITTED VIA THE FARE ORIGIN
// Restrictions 10:  TRAVEL CANNOT CONTAIN MORE THAN ONE COUPON BETWEEN THE SAME POINTS IN THE SAME DIRECTION
//------------------------------------------------------------------------------------------
bool
TravelRestrictionValidator::validateImpl(const TravelRoute& tvlRoute,
                                         const RoutingRestriction& restriction,
                                         const PricingTrx& trx)
{
  TravelRouteBuilder travelRouteBuilder;
  TravelRoute tempTravelRoute;

  travelRouteBuilder.buildTempTravelRoute(tvlRoute, tempTravelRoute, tvlRoute.unticketedPointInd());

  if(tempTravelRoute.travelRoute().size() < 3)
    return true;

  if(restriction.restriction() == RTW_ROUTING_RESTRICTION_9)
  {
    return std::find_if(tempTravelRoute.travelRoute().begin() + 1,
          tempTravelRoute.travelRoute().end() - 1,
          CheckCityPair(tempTravelRoute.travelRoute().begin()->boardCity().loc()))
        == (tempTravelRoute.travelRoute().end() - 1);
  } else if(restriction.restriction() == RTW_ROUTING_RESTRICTION_10)
  {
    std::set<std::string> cityPairList;

    std::vector<TravelRoute::CityCarrier>::const_iterator
        tvlIt = (tempTravelRoute.travelRoute().begin() + 1),
        tvlItEnd = (tempTravelRoute.travelRoute().end() - 1);
    for(; tvlIt != tvlItEnd; ++tvlIt)
    {
      if(!cityPairList.insert(tvlIt->boardCity().loc()
          + tvlIt->offCity().loc()).second)
        return false;
    }
    return true;
  }
  return false;
}

}
