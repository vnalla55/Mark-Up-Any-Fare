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

#include "Routing/CityCarrierRestrictionValidator.h"

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/RoutingRestriction.h"
#include "Routing/RoutingConsts.h"

#include <vector>

namespace tse
{

//----------------------------------------------------------------------------
// Constructor()
//----------------------------------------------------------------------------
CityCarrierRestrictionValidator::CityCarrierRestrictionValidator() {}

//----------------------------------------------------------------------------
// Destructor()
//----------------------------------------------------------------------------
CityCarrierRestrictionValidator::~CityCarrierRestrictionValidator() {}

//--------------------------------------------------------------------------------------
// Restrictions 1:  TRAVEL BETWEEN CITY1 AND CITY2 MUST/MUST NOT BE VIA CITY3
// Restrictions 2:  TRAVEL MUST/MUST NOT BE VIA CITY
// Restriction 18:  TRAVEL BETWEEN CITY1 AND CITY2 MUST/MUST NOT BE VIA CARRIER
//
// Restriction  5:  TRAVEL TO/FROM CITY1 MUST/MUST NOT BE VIA CITY3
// Restriction 19:  TRAVEL TO/FROM CITY1 MUST/MUST NOT BE VIA CARRIER
//
// Restriction 21:  WHEN ORIGIN IS CITY1 AND DEST IS CITY2 TRAVEL MUST/NOT BE VIA CITY3
//--------------------------------------------------------------------------------------
bool
CityCarrierRestrictionValidator::validate(const TravelRoute& tvlRoute,
                                          const RoutingRestriction& restriction,
                                          const PricingTrx& trx)
{
  bool citiesFound = true;
  bool viaCityFound = true;
  bool viaCarrierFound = true;
  bool city1Found = true;

  //-------------------------------------
  // Check for invalid restriction data
  //-------------------------------------
  if (UNLIKELY(restriction.negViaAppl() == PERMITTED || restriction.negViaAppl() == BLANK))
  {
    return true;
  }

  TravelRouteItr city1I = tvlRoute.travelRoute().begin();
  TravelRouteItr city2I = tvlRoute.travelRoute().end();

  // To/From CITY1 must/must not be via CITY3
  if (restriction.restriction() == ROUTING_RESTRICTION_5)
  {
    // Search for CITY1 then check for a via point on either side of CITY1
    return searchToFromCity1ViaCity3(trx, restriction, tvlRoute);
  }

  // ToFrom CITY1 must/must not be via CARRIER
  if (UNLIKELY(restriction.restriction() == ROUTING_RESTRICTION_19))
  {
    // Search for CITY1 then check for the specified carrier on either side of CITY1
    city1Found = searchLoc(trx, std::make_pair(' ', restriction.market1()) , city1I, city2I);
    if (city1Found)
    {
      viaCarrierFound =
          searchToFromCarrier(tvlRoute, restriction.market1(), restriction.viaCarrier());

      return restrictionValid(restriction, viaCityFound, viaCarrierFound);
    }
    else
    {
      return true; // City1 not found:  Restriction not applicable
    }
  }

  // When origin is CITY1 and destination is CITY2 then travel must be via CITY3
  if (UNLIKELY(restriction.restriction() == ROUTING_RESTRICTION_21))
  {
    if (restriction.market1() != city1I->boardCity().loc() ||
        restriction.market2() != tvlRoute.travelRoute().back().offCity().loc())
    {
      return true; // Restriction is not applicable
    }
  }

  // Travel between CITY1 and CITY2 must/must not be via
  if (restriction.restriction() == ROUTING_RESTRICTION_18 ||
      restriction.restriction() == ROUTING_RESTRICTION_1)
  {
    citiesFound =
        locateBetweenLocs(trx, tvlRoute,
                          std::make_pair(restriction.market1type(), restriction.market1()),
                          std::make_pair(restriction.market2type(), restriction.market2()),
                          city1I, city2I);
  }

  // City or Carrier MUST BE VIA
  if (citiesFound)
  {
    if (LIKELY(!restriction.viaMarket().empty()))
    {
      // Restriction 2 should search for via city within travel route, excluding orig and dest.
      // Since searchCity() compares only offCity of travel route items, we only need to move the
      // end iterator one backwards.
      viaCityFound = searchLoc(trx, std::make_pair(restriction.viaType(), restriction.viaMarket()), city1I, --city2I);
    }
    else if (restriction.negViaAppl() == NOT_PERMITTED)
    {
      viaCarrierFound = searchCarrier(tvlRoute, restriction.viaCarrier(), city1I, city2I);
    }
    else
    {
      viaCarrierFound = checkAllCarriers(tvlRoute, restriction.viaCarrier(), city1I, city2I);
    }

    return restrictionValid(restriction, viaCityFound, viaCarrierFound);
  } // Cities not found on travel route
  return true; // Restriction Not Applicable:  Cities not found
}

//-------------------------------------------------------------------------------------
// Determine whether the restriction is valid
//-------------------------------------------------------------------------------------
bool
CityCarrierRestrictionValidator::restrictionValid(const RoutingRestriction& restriction,
                                                  bool viaCityFound,
                                                  bool viaCarrierFound)
{
  if (restriction.negViaAppl() == PERMITTED || restriction.negViaAppl() == REQUIRED)
  {
    if (viaCityFound && viaCarrierFound)
    {
      return true; // City or Carrier Required and found
    }
  }
  // MUST NOT BE VIA CITY3 OR CARRIER
  else
  {
    if (!viaCityFound || !viaCarrierFound)
    {
      return true; // City or Carrier found, but not permitted
    }
  }
  return false; // Restriction Not Valid
}

//---------------------------------------------------------------------------------
// Check each carrier in the Travel Route for a match on viaCarrier.
//
// Return TRUE when all carriers in the travel route match the via carrier.
//---------------------------------------------------------------------------------
bool
CityCarrierRestrictionValidator::checkAllCarriers(const TravelRoute& tvlRoute,
                                                  const CarrierCode& carrier,
                                                  TravelRouteItr city1I,
                                                  TravelRouteItr city2I)
{
  for (; city1I != city2I; ++city1I)
  {
    if (city1I->carrier() != carrier && city1I->carrier() != SURFACE_CARRIER)
    {
      return false; // At least one instance of carrier does not match
    }
  }
  return true; // All Carriers in Travel Route match viaCarrier
}

//---------------------------------------------------------------------------------
// Search for the via Carrier in the portion of the travel Route
//
// Return TRUE if the carrier is found anywhere in the travel route.
//---------------------------------------------------------------------------------
bool
CityCarrierRestrictionValidator::searchCarrier(const TravelRoute& tvlRoute,
                                               const CarrierCode& carrier,
                                               TravelRouteItr city1I,
                                               TravelRouteItr city2I)
{
  for (; city1I != city2I; ++city1I)
  {
    if (city1I->carrier() == carrier)
    {
      return true; // Carrier was found
    }
  }
  return false; // Carrier was not found
}

//---------------------------------------------------------------------------------
// Restriction 5:  To/From City1 Must/must not be via CITY3
//
// Locate CITY1, then search for via City3.
// CITY1 may be the origin or destination of the Travel Route, but CITY3 may not.
//---------------------------------------------------------------------------------
bool
CityCarrierRestrictionValidator::searchToFromCity1ViaCity3(const PricingTrx& trx,
                                                           const RoutingRestriction& restriction,
                                                           const TravelRoute& tvlRoute)
{
  if (restriction.market1() == tvlRoute.origin() ||
      searchLoc(trx,
                std::make_pair(restriction.market1type(), restriction.market1()),
                tvlRoute.travelRoute().begin(),
                tvlRoute.travelRoute().end()))
  {
    bool viaLocFound = searchLoc(trx,
                                 std::make_pair(restriction.viaType(), restriction.viaMarket()),
                                 tvlRoute.travelRoute().begin(),
                                 tvlRoute.travelRoute().end() - 1);

    return restrictionValid(restriction, viaLocFound, true);
  }
  else
  {
    return true; // City1 not found - restriction not applicable
  }
}

//---------------------------------------------------------------------------------
// Restriction 19:  To/From City1 Must/must not be via Carrier
//
// Locate CITY1, then search for via carrier on either side of City1
// CITY1 may be the origin or destination of the Travel Route.
//---------------------------------------------------------------------------------
bool
CityCarrierRestrictionValidator::searchToFromCarrier(const TravelRoute& tvlRoute,
                                                     const LocCode& city1,
                                                     const CarrierCode& viaCarrier)
{
  bool city1Found = false;
  TravelRouteItr city1I = tvlRoute.travelRoute().begin();
  TravelRouteItr city2I = tvlRoute.travelRoute().end();

  for (; city1I != city2I; ++city1I)
  {
    if (city1I->boardCity().loc() == city1 || city1I->offCity().loc() == city1)
    {
      city1Found = true;
      if (city1I->carrier() == viaCarrier)
      {
        return true;
      }
    }
  }

  return !city1Found;
}

} // tse
