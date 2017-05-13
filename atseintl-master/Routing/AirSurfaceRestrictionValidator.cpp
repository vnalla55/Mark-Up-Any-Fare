//----------------------------------------------------------------------------
//  Copyright Sabre 2003
//
//  AirSurfaceRestrictionValidator.cpp
//
//  Description:  Validate Routing Restrictions 11
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

#include "Routing/AirSurfaceRestrictionValidator.h"

#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/RoutingRestriction.h"
#include "Routing/RestrictionValidator.h"
#include "Routing/RoutingConsts.h"
#include "Routing/TravelRoute.h"
#include "Routing/TravelRouteBuilder.h"

namespace tse
{
AirSurfaceRestrictionValidator::AirSurfaceRestrictionValidator() {}

AirSurfaceRestrictionValidator::~AirSurfaceRestrictionValidator() {}

//---------------------------------------------------------------------------------------------------------
// Process Restriction 11:BETWEEN CITY1 AND CITY2 AIR/SURFACE SECTOR REQUIRED/PERMITTED/NOT
// PERMITTED
//------------------------------------------------------------------------------------------------------
bool
AirSurfaceRestrictionValidator::validate(const TravelRoute& tvlRoute,
                                         const RoutingRestriction& restriction,
                                         const PricingTrx& trx)
{
  TravelRouteBuilder travelRouteBuilder;
  bool citiesFound = false;
  bool surfaceSectorFound = true;
  bool airSectorFound = true;

  std::vector<TravelRoute::CityCarrier> validCityPair;

  const bool isRTW = trx.getOptions()->isRtw();
  MarketData market1Data = std::make_pair(isRTW?restriction.market1type():'C', restriction.market1());
  MarketData market2Data = std::make_pair(isRTW?restriction.market2type():'C', restriction.market2());

  for (TravelRoute::CityCarrier it : tvlRoute.travelRoute())
  {
    if(locateBetweenLoc(trx.dataHandle(), it, market1Data, market2Data))
    {
      citiesFound = true;
      validCityPair.push_back(it);
    }
  }
  TravelRouteItr city1I = validCityPair.begin();
  TravelRouteItr city2I = validCityPair.end();

  if (citiesFound)
  {
    // No validation necessary when Air or Surface is PERMITTED
    if (restriction.negViaAppl() == PERMITTED || restriction.negViaAppl() == ' ')
    {
      return true;
    }

    if (restriction.airSurfaceInd() == SURFACE)
    {
      if (restriction.negViaAppl() == NOT_PERMITTED)
      {
        surfaceSectorFound = checkAnySurface(tvlRoute, city1I, city2I);
      }
      else if (restriction.negViaAppl() == REQUIRED)
      {
        surfaceSectorFound = checkAllSurfaceSectors(tvlRoute, city1I, city2I);
      }
    }
    else if (restriction.airSurfaceInd() == AIR)
    {
      if (restriction.negViaAppl() == NOT_PERMITTED)
      {
        airSectorFound = checkAnyAir(tvlRoute, city1I, city2I);
      }
      else if (restriction.negViaAppl() == REQUIRED)
      {
        airSectorFound = checkAllAirSectors(tvlRoute, city1I, city2I);
      }
    }

    // Now check results
    if (restriction.negViaAppl() == PERMITTED || restriction.negViaAppl() == REQUIRED)
    {
      if (surfaceSectorFound && airSectorFound)
      {
        return true; // Surface or Air Found as Required/Permitted
      }
    }
    // MUST NOT BE VIA CITY3 OR CARRIER
    else
    {
      if (!surfaceSectorFound || !airSectorFound)
      {
        return true; // Surface or Air Not Found as Required
      }
    }
    return false; // Restriction Not Valid
  } // Cities not found on Travel Route

  return true;
}

//---------------------------------------------------------------------------------
// Search for the Surface Sectors in the portion of the travel Route
//
// Return TRUE if a surface sector is anywhere in the travel route.
//---------------------------------------------------------------------------------
bool
AirSurfaceRestrictionValidator::checkAnySurface(const TravelRoute& tvlRoute,
                                                TravelRouteItr city1I,
                                                TravelRouteItr city2I)
{
  for (; city1I != city2I; ++city1I)
  {
    if ((*city1I).carrier() == SURFACE_CARRIER)
    {
      return true; // Surface Travel was found
    }
  }
  return false; // No surface travel was found
}

//---------------------------------------------------------------------------------
// Search for Air Sectors in the portion of the travel Route
//
// Return TRUE if an air segment is found anywhere in the travel route.
//---------------------------------------------------------------------------------
bool
AirSurfaceRestrictionValidator::checkAnyAir(const TravelRoute& tvlRoute,
                                            TravelRouteItr city1I,
                                            TravelRouteItr city2I)
{
  for (; city1I != city2I; ++city1I)
  {
    if ((*city1I).carrier() != SURFACE_CARRIER)
    {
      return true; // Air Travel was found
    }
  }
  return false; // Air Travel was not found
}

//---------------------------------------------------------------------------------
// Check each sector in the Travel Route for surface travel.
//
// Return TRUE when all sectors are Air
//---------------------------------------------------------------------------------
bool
AirSurfaceRestrictionValidator::checkAllAirSectors(const TravelRoute& tvlRoute,
                                                   TravelRouteItr city1I,
                                                   TravelRouteItr city2I)
{
  for (; city1I != city2I; ++city1I)
  {
    if ((*city1I).carrier() == SURFACE_CARRIER)
    {
      return false; // At least one sector is not Air
    }
  }
  return true; // All Sectors are Air
}

//---------------------------------------------------------------------------------
// Check each sector in the Travel Route for surface travel.
//
// Return TRUE when all sectors are surface.
//---------------------------------------------------------------------------------
bool
AirSurfaceRestrictionValidator::checkAllSurfaceSectors(const TravelRoute& tvlRoute,
                                                       TravelRouteItr city1I,
                                                       TravelRouteItr city2I)
{
  for (; city1I != city2I; ++city1I)
  {
    if ((*city1I).carrier() != SURFACE_CARRIER)
    {
      return false; // At least one sector is not surface
    }
  }
  return true; // All Sectors are surface
}
}
