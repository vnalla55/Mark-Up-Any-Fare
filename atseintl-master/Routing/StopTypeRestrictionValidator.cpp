//-------------------------------------------------------------------------------
// Copyright 2014, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "Routing/StopTypeRestrictionValidator.h"

#include "Common/Logger.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/RoutingRestriction.h"
#include "Routing/RestrictionValidator.h"
#include "Routing/RoutingConsts.h"
#include "Routing/TravelRoute.h"
#include "Routing/TravelRouteBuilder.h"

#include <algorithm>
#include <vector>

namespace tse
{

static Logger
logger("atseintl.Routing.StopTypeRestrictionValidator");

bool StopTypeRestrictionValidator::validate(const TravelRoute& tvlRoute,
    const RoutingRestriction& restriction, const PricingTrx& trx)
{
  const char* ptr = restriction.restriction().c_str();
  const int RestrictionNumber = atoi(ptr);

  switch (RestrictionNumber)
  {
    case 3:
      return validateRestriction3(tvlRoute, restriction, trx);
    case 4:
      return validateRestriction4(tvlRoute, restriction, trx);
    case 6:
      return validateRestriction6(tvlRoute, restriction, trx);
    default:
      LOG4CXX_INFO(logger, " Error Condition in StopTypeRestrictionValidator::validate()");
      return true;
  }
}

bool StopTypeRestrictionValidator::validateRestriction3(const TravelRoute& tvlRoute,
    const RoutingRestriction& restriction, const PricingTrx& trx)
{
  if (UNLIKELY(trx.getOptions()->isRtw()))
    return true;

  // Build temporary travel route with flight stops in case the Fare Market governing carrier
  // was not a flight tracking carrier.
  TravelRoute tempTravelRoute;
  TravelRouteBuilder travelRouteBuilder;

  travelRouteBuilder.buildTempTravelRoute(tvlRoute, tempTravelRoute);

  LOG4CXX_DEBUG(logger, "Entered StopTypeRestrictionValidator for Restriction 3");
  return validateStopType(tempTravelRoute, restriction);
}

bool StopTypeRestrictionValidator::validateRestriction4(const TravelRoute& tvlRoute,
    const RoutingRestriction& restriction, const PricingTrx& trx)
{
  LOG4CXX_DEBUG(logger, "Entered StopTypeRestrictionValidator for Restriction 4");
  TravelRouteItr startItr, endItr;
  bool cityFound = false;

  if (UNLIKELY(trx.getOptions()->isRtw()))
    cityFound = locateBetween(trx.dataHandle(), tvlRoute.travelRoute(),
        std::make_pair(restriction.market1type(), restriction.market1()),
        std::make_pair(restriction.market2type(), restriction.market2()), startItr, endItr);
  else
    cityFound = locateBetween(trx.dataHandle(), tvlRoute.travelRoute(),
        std::make_pair('C', restriction.market1()), std::make_pair('C', restriction.market2()),
        startItr, endItr);

  if (!cityFound)
    return true; // if the city is not if travelRoute, it passes.
  else
  {
    TravelRoute tmpTravelRoute;

    if (startItr < endItr)
      copy(startItr, endItr, back_inserter(tmpTravelRoute.travelRoute()));
    else
      copy(endItr, startItr, back_inserter(tmpTravelRoute.travelRoute()));
    return validateStopType(tmpTravelRoute, restriction);
  }
}

bool StopTypeRestrictionValidator::validateRestriction6(const TravelRoute& tvlRoute,
    const RoutingRestriction& restriction, const PricingTrx& trx)
{
  LOG4CXX_DEBUG(logger, "Entered StopTypeRestrictionValidator for Restriction 6");

  TravelRouteItr itr;
  bool cityFound;
  if (trx.getOptions()->isRtw())
    cityFound = findCity(trx.dataHandle(), tvlRoute.travelRoute(), itr,
        std::make_pair(restriction.market1type(), restriction.market1()));
  else
    cityFound = findCity(trx.dataHandle(), tvlRoute.travelRoute(), itr,
        std::make_pair('C', restriction.market1()));

  TravelRoute tmpTravelRoute;
  if (!cityFound)
    return true;
  else if (itr == tvlRoute.travelRoute().begin())
    tmpTravelRoute.travelRoute().push_back(*itr);
  else if (itr == tvlRoute.travelRoute().end())
    tmpTravelRoute.travelRoute().push_back(*(itr - 1)); // validate travel to this point is nonStop/Direct
  else
  {
    TravelRouteItr startItr = itr--;
    TravelRouteItr endItr = itr;
    // Assumption : the order of city1 and city2 in the route of travel
    // doesnt have any impact in validation

    if (startItr < endItr)
      copy(startItr, endItr, back_inserter(tmpTravelRoute.travelRoute()));
    else
      copy(endItr, startItr, back_inserter(tmpTravelRoute.travelRoute()));
  }

  return validateStopType(tmpTravelRoute, restriction);
}

uint32_t StopTypeRestrictionValidator::countNonStops(DataHandle& data,
    const TravelRouteList& tvlRouteList, const MarketData& marketData1,
    const MarketData& marketData2)
{
  uint32_t nonStopCounter = 0;

  for (TravelRoute::CityCarrier it : tvlRouteList)
  {
    if(it.carrier().equalToConst("XX"))
      continue;
    if(locateBetweenLoc(data, it, marketData1, marketData2))
      nonStopCounter++;
  }

  return nonStopCounter;
}

bool StopTypeRestrictionValidator::isDirect(const TravelRoute& tvlRoute)
{
  if (tvlRoute.travelRoute().size() < 2) // empty tvlRoute! size=0 and size=1
  {
    return true;
  }
  else
  {
    std::vector<TravelRoute::CityCarrier>::const_iterator itr = tvlRoute.travelRoute().begin();
    for (; itr != tvlRoute.travelRoute().end(); itr++)
    {
      if (itr->offCity().isHiddenCity())
        continue;
      else if (itr + 1 == tvlRoute.travelRoute().end())
        return true;
      else
        return false;
    }
    return true;
  }
}

bool StopTypeRestrictionValidator::isNonStop(const TravelRoute& tvlRoute)
{
  std::vector<TravelRoute::CityCarrier>::const_iterator itr = tvlRoute.travelRoute().begin();

  if (UNLIKELY(tvlRoute.travelRoute().empty()))
    return true;
  else if (tvlRoute.travelRoute().size() == 1 && !(itr->offCity().isHiddenCity()))
    return true;
  else
    return false;
  // return tvlRoute.travelRoute().size() < 2; //if proved correct, above commented code should be
  // erased
}

bool StopTypeRestrictionValidator::validateStopType(const TravelRoute& tvlRoute,
    const RoutingRestriction& restriction)
{
  // Check for Invalid restriction data
  if (UNLIKELY(restriction.negViaAppl() == PERMITTED || restriction.negViaAppl() == BLANK))
  {
    return true;
  }

  if (restriction.nonStopDirectInd() == EITHER && restriction.negViaAppl() == REQUIRED)
  {
    // must be nonStop or direct
    return (isDirect(tvlRoute) || isNonStop(tvlRoute));
  }
  else if (restriction.nonStopDirectInd() == EITHER && restriction.negViaAppl() == NOT_PERMITTED)
  {
    // must not be non-stop or direct
    return !(isDirect(tvlRoute) || isNonStop(tvlRoute));
  }

  else if (restriction.nonStopDirectInd() == DIRECT)
  {
    bool isDIRECT = isDirect(tvlRoute);

    if (restriction.negViaAppl() == REQUIRED)
    {
      return isDIRECT;
    }
    else if (LIKELY(restriction.negViaAppl() == NOT_PERMITTED))
    {
      return !isDIRECT;
    }
    else
    {
      // what if the field is empty
      return isDIRECT;
    }
  }

  else if (LIKELY(restriction.nonStopDirectInd() == NONSTOP))
  {
    bool isNONSTOP = isNonStop(tvlRoute);

    if (restriction.negViaAppl() == REQUIRED)
    {
      return isNONSTOP;
    }
    else if (restriction.negViaAppl() == NOT_PERMITTED)
    {
      return !isNONSTOP;
    }
    else
    {
      // what if the field is empty
      return isNONSTOP;
    }
  }
  else
  {
    return true; // Error Condition!
  }
}

bool StopTypeRestrictionValidator::findCity(DataHandle& data, const TravelRouteList& tvlRouteList,
    TravelRouteItr& itr, const MarketData& marketData)
{
  for (itr = tvlRouteList.begin(); itr != tvlRouteList.end(); itr++)
  {
    if (checkPoint(data, itr->boardCity().loc(), itr->boardNation(), marketData))
      return true;
    else if (checkPoint(data, itr->offCity().loc(), itr->offNation(), marketData))
    {
      ++itr;
      return true;
    }
    else
      continue;
  }
  return false;
}

} // namespace tse
