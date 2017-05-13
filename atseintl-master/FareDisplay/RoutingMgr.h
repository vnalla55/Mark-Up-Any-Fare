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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/FareDisplayTrx.h"
#include "Routing/RoutingEnums.h"
#include "Routing/RoutingInfo.h"
#include "Routing/RtgKey.h"


#include <map>
#include <vector>

namespace tse
{
class PaxTypeFare;
class RtgKey;
class Routing;

class RoutingMgr
{
public:
  friend class RoutingMgrTest;
  //--------------------------------------------------------------------------
  // @function RoutingMgr::RoutingMgr
  //
  // Description: constructor
  //
  //--------------------------------------------------------------------------
  RoutingMgr(FareDisplayTrx& trx);

  //--------------------------------------------------------------------------
  // @function RoutingMgr::~RoutingMgr
  //
  // Description: destructor
  //
  //--------------------------------------------------------------------------
  virtual ~RoutingMgr();

  //--------------------------------------------------------------------------
  // @function RoutingMgr::buildTravelRouteAndMap
  //
  // Description:
  //
  // @param none
  // @return true if successful, false otherwise
  //--------------------------------------------------------------------------
  bool buildTravelRouteAndMap();
  bool buildAddOnRoutings();

private:

  typedef std::map<RtgKey, bool> RtgKeyMap;
  typedef std::map<const RtgKey, RoutingInfo*> RoutingKeyMap;
  typedef std::map<TravelRoute*, RoutingKeyMap*> TvlRouteMap;
  typedef std::map<FareMarket*, TravelRoute*> FareMarketMap;

  FareDisplayTrx& _trx;

  RtgKeyMap* _rtMap;
  TvlRouteMap* _tvlRouteMap;
  RoutingKeyMap* _routingKeyMap;
  FareMarketMap* _fareMarketMap;

  //--------------------------------------------------------------------------
  // @function RoutingMgr::getRoutingData
  //
  // Description:
  //
  // @param trx - A valid FareDisplayTrx
  // @return true if successful, false otherwise
  //--------------------------------------------------------------------------
  const Routing* getAddonRoutingData(const VendorCode& vendor,
                                     const TariffNumber& routingTariff1,
                                     const TariffNumber& routingTariff2,
                                     const CarrierCode& carrier,
                                     const RoutingNumber& routingNumber,
                                     const DateTime& travelDate,
                                     const Fare* fare = nullptr);

  //--------------------------------------------------------------------------
  // @function RoutingMgr::buildRtgKey
  //
  // Description:
  //
  // @param trx - A valid FareDisplayTrx
  // @return true if successful, false otherwise
  //--------------------------------------------------------------------------
  static void buildRtgKey(PaxTypeFare& paxTypeFare,
                          const FareMarket& fareMarket,
                          const Routing* routing,
                          const Routing* origAddOnRouting,
                          const Routing* destAddOnRouting,
                          RtgKey& rKey);

  //--------------------------------------------------------------------------
  // @function RoutingMgr::buildUniqueRoutings
  //
  // Description:
  //
  // @param rtgKey - reference to a valid RtgKey
  // @param tvlRoute - reference to a valid TravelRoute
  // @param paxTypeFare - pointer to a valid PaxTypeFare
  // @param routing - pointer to a valid Routing
  // @param origAddOnRouting - pointer to a valid Routing
  // @param destAddOnRouting - pointer to a valid Routing
  // @return true if successful, false otherwise
  //--------------------------------------------------------------------------
  void buildUniqueRoutings();

  bool processRouting(const std::string& rtgSeq,
                      TravelRoute& tvlRoute,
                      PaxTypeFare* paxTypeFare,
                      const Routing* routing,
                      const Routing* origAddOnRouting,
                      const Routing* destAddOnRouting);

  bool
  processAddOnRouting(const std::string&, const LocCode&, const Routing*, const FDAddOnFareInfo&);

  bool getRoutingMaps(TravelRoute& tvlRoute,
                      MapInfo* mapInfo,
                      bool isFareReversed,
                      const Routing* routing,
                      const Routing* origAddOnRouting = nullptr,
                      const Routing* destAddOnRouting = nullptr);

  bool getAddOnRoutingMaps(const LocCode&, MapInfo*, const Routing*) const;

  void processRouteStrings(RoutingMapStrings&) const;

  void
  getMPMMileageInfo(GlobalDirection global, const DateTime& travelDate, MileageInfo& mileageInfo);

  void reverseTravelRoute(const TravelRoute&, TravelRoute&) const;

  // bool reCalcStrMaxResSize(uint16_t & rtgStrMaxResSize, uint16_t resultSize) const;
};

} // end tse namespace

