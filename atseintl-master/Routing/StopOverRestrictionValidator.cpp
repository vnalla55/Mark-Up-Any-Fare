//----------------------------------------------------------------------------
//  Copyright Sabre 2003
//
//  StopOverRestrictionValidator.cpp
//
//  Description:  Validate Routing Restrictions 7
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


#include "Routing/StopOverRestrictionValidator.h"

#include "DataModel/PricingTrx.h"
#include "DBAccess/RoutingRestriction.h"
#include "Routing/RestrictionValidator.h"
#include "Routing/RoutingConsts.h"
#include "Routing/TravelRoute.h"
#include "Routing/TravelRouteBuilder.h"

namespace tse
{

StopOverRestrictionValidator::StopOverRestrictionValidator() {}

StopOverRestrictionValidator::~StopOverRestrictionValidator() {}

//------------------------------------------------------------------------------------------------
// Restriction 7:TRAVEL BETWEEN CITY1 AND CITY2 STOPOVER IN CITY3 REQUIRED/PERMITTED/NOT PERMITTED
//------------------------------------------------------------------------------------------------
bool
StopOverRestrictionValidator::validate(const TravelRoute& tvlRoute,
                                       const RoutingRestriction& restriction,
                                       const PricingTrx& trx)
{
  TravelRouteBuilder travelRouteBuilder;
  bool citiesFound = true;
  bool stopoverFound = true;

  TravelRouteItr city1I = tvlRoute.travelRoute().begin();
  TravelRouteItr city2I = tvlRoute.travelRoute().end();

  // Check for invalid restriction data
  if (restriction.negViaAppl() == BLANK || restriction.negViaAppl() == PERMITTED)
  {
    return true;
  }

  if (restriction.restriction() == ROUTING_RESTRICTION_7)
  {
    // Find CITY1 and CITY2
    citiesFound =
        locateBetweenLocs(trx, tvlRoute,
                          std::make_pair(restriction.market1type(), restriction.market1()),
                          std::make_pair(restriction.market2type(), restriction.market2()),
                          city1I, city2I);
  }

  if (citiesFound)
  {
    // Check for Stopover in City3
    stopoverFound = searchForStopover(tvlRoute, restriction.viaMarket(), city1I, city2I);

    // Stopover in City3 Required
    if (stopoverFound)
    {
      if (restriction.negViaAppl() == REQUIRED)
      {
        return true;
      }
    }
    // Stopover in City3 Not Permitted
    else // Stopover Not Found
    {
      if (restriction.negViaAppl() == NOT_PERMITTED)
      {
        return true;
      }
    }
    return false;
  }

  return true; // Restriction Not Applicable:  Cities not found
}

//---------------------------------------------------------------------------------
// Search for Stopover
//
// Return True if a StopOver is found in the viaMarket
//---------------------------------------------------------------------------------
bool
StopOverRestrictionValidator::searchForStopover(const TravelRoute& tvlRoute,
                                                const LocCode& viaMarket,
                                                TravelRouteItr city1I,
                                                TravelRouteItr city2I)
{
  for (; city1I != city2I; ++city1I)
  {
    if ((*city1I).offCity().loc() == viaMarket && (*city1I).stopover())
    {
      return true; // At least one instance of carrier does not match
    }
  }
  return false; // City not found
}
}
