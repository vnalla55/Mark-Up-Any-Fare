//----------------------------------------------------------------------------
//  Description: Common functions required for ATSE shopping/pricing.
//  Copyright Sabre 2004
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

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "Routing/RoutingEnums.h"
#include "Routing/RoutingInfo.h"


#include <map>
#include <vector>

namespace tse
{
class DataHandle;
class Loc;
class PaxTypeFare;
class PricingTrx;
class Routing;
class RtgKey;

class RoutingUtil
{
  friend class RoutingUtilTest;

public:
  typedef std::map<PaxTypeFare*, RoutingFareType> PaxTypeFareMap;
  typedef std::map<PaxTypeFare*, RoutingFareType>::iterator PaxTypeFareMapItr;
  /**
   * Takes a set of PaxTypeFare and sets the Fare Type Flag in the value field for each of them.
   * There are Three different Fare Type
   * 1. Routing Fare : when there exist a route map for the VCTR.
   * 2. Mileage Fare : if the routing number is "0000" or if there is only
   *    restriction and one of them is either restriction 12 or restriction 16 or both.
   * 3. UNKNOWN_FARE_TYPE : Error condition.
   */

  static bool getRoutingType(PaxTypeFareMap& pMap, PricingTrx& trx);

  /**
   * Takes a PaxTypeFare and returns the fare type.
   * Essentially this method creates a routing key (VCTR)
   * and calls the getRoutingType(RtgKey&) which returns
   * the routing type.
   *
   **/
  static RoutingFareType getRoutingType(const PaxTypeFare* paxTypeFare, PricingTrx& trx);
  static RoutingFareType getRoutingType(const Routing* routing);
  static bool processBaseTravelRoute(const RoutingInfo& rtgInfo);
  static bool locMatchesOrigin(const Loc* loc, const TravelRoute& tvlRoute);
  static bool locMatchesDestination(const Loc* loc, const TravelRoute& tvlRoute);
  static bool locMatchesTvlSeg(const Loc* loc, TravelSeg& tvlSeg);

  static void updateRoutingInfo(PaxTypeFare& paxTypeFare,
                                const Routing* routing,
                                RoutingInfo& routingInfo,
                                bool fillRoutingTrafficDescriptions,
                                bool useDefaultMarkets);

  static bool isRouting(const PaxTypeFare& fare, PricingTrx& trx);

  static const Routing*
  getRoutingData(PricingTrx& trx, PaxTypeFare& paxTypeFare, const RoutingNumber& routingNumber);

  static void getVendor(const PaxTypeFare* p, DataHandle& dataHandle, VendorCode& vendor);

  static const Routing* getStaticRoutingMileageHeaderData(PricingTrx& trx,
                                                          const CarrierCode& govCxr,
                                                          const PaxTypeFare& paxTypeFare);

  static bool isSpecialRouting(const PaxTypeFare& paxTypeFare, const bool checkEmptyRouting = true);

  static bool isTicketedPointOnly(const Routing* routing, bool flightTrackingCxr);

private:
  static const Routing* getRoutingData(const PaxTypeFare* paxTypeFare, PricingTrx& trx);
  static void
  fillRoutingTrfDesc(PaxTypeFare& paxTypeFare, const Routing* routing, RoutingInfo& routingInfo);

};

} // end tse namespace

