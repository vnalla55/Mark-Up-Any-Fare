//
//----------------------------------------------------------------------------
//  File:   RoutingDiagCollector.h
//
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
#include "Diagnostic/DiagCollector.h"
#include "Routing/DRVInfo.h"
#include "Routing/RoutingInfo.h"

namespace tse
{
class Routing;
class RoutingRestriction;
class TravelRoute;

class RoutingDiagCollector : public DiagCollector
{

public:
  explicit RoutingDiagCollector(Diagnostic& root)
    : DiagCollector(root),
      _constructedRouting(false),
      _noOrigAddon(false),
      _origAddonMileage(false),
      _origAddonMap(false),
      _origAddonRestriction(false),
      _baseMileage(false),
      _baseMap(false),
      _baseRestriction(false),
      _noDestAddon(false),
      _destAddonMileage(false),
      _destAddonMap(false),
      _destAddonRestriction(false)
  {
  }

  RoutingDiagCollector()
    : _constructedRouting(false),
      _noOrigAddon(false),
      _origAddonMileage(false),
      _origAddonMap(false),
      _origAddonRestriction(false),
      _baseMileage(false),
      _baseMap(false),
      _baseRestriction(false),
      _noDestAddon(false),
      _destAddonMileage(false),
      _destAddonMap(false),
      _destAddonRestriction(false)
  {
  }

  void displayRoutingStatus(const TravelRoute& tvlRoute, const RoutingInfo& rtgInfo);
  void displayRoutingStatusDepreciated(const TravelRoute& tvlRoute, const RoutingInfo& rtgInfo);

  void displayCityCarriers(const CarrierCode& carrier,
                           std::vector<TravelRoute::CityCarrier> cityCarrierVec);
  void displayRestriction(const RoutingRestriction& restriction,
                          const RestrictionInfo& restrictionStatus);
  void displayMileageMessages(const RoutingInfo& rtgInfo);
  void displayMarketVector(const std::vector<Market>& markets);
  bool displayMapMessages(const RoutingInfo& rtgInfo);
  bool displayMapResults(const MapInfo* mapInfo, const RoutingInfo& rtgInfo);
  void displayMissingCity(std::vector<TravelRoute::CityCarrier> cityCarrierVec,
                          const RoutingInfo& rtgInfo,
                          bool drvDisplay);

  void displayRestrictions(const RoutingInfo& rtgInfo);
  void displayNoRestrictionsMessage();
  void displayNoFaresMessage();
  void displayTerminalPointMessage(const TravelRoute& tvlRoute, const RoutingInfo& rtgInfo);
  void displayOrigAddonRestrictions(const RoutingInfo& rtgInfo);
  void displayDestAddonRestrictions(const RoutingInfo& rtgInfo);
  void displayBaseRestrictions(const RoutingInfo& rtgInfo);
  void displayConstructedRestriction(const RoutingRestriction& restriction, bool restrictionStatus);

  void displayMapDirectionalityInfo(const RoutingInfo& rtgInfo);

protected:
  void setRoutingTypes(const RoutingInfo& rtgInfo);
  void displayConstructedRouting(const TravelRoute& tvlRoute, const RoutingInfo& rtgInfo);
  void
  displayConstructedRoutingDepreciated(const TravelRoute& tvlRoute, const RoutingInfo& rtgInfo);
  void displayOrigAddon(const RoutingInfo& rtgInfo);
  void displayBase(const TravelRoute& tvlRoute, const RoutingInfo& rtgInfo);
  void displayDestAddon(const TravelRoute& tvlRoute, const RoutingInfo& rtgInfo);

  void displayRestriction1(const RoutingRestriction& restriction);
  void displayRestriction2(const RoutingRestriction& restriction);
  void displayRestriction3(const RoutingRestriction& restriction);
  void displayRestriction4(const RoutingRestriction& restriction);
  void displayRestriction5(const RoutingRestriction& restriction);
  void displayRestriction6(const RoutingRestriction& restriction);
  void displayRestriction7(const RoutingRestriction& restriction);
  void displayRestriction8(const RoutingRestriction& restriction);
  void displayRestriction9(const RoutingRestriction& restriction);
  void displayRestriction10(const RoutingRestriction& restriction);
  void displayRestriction11(const RoutingRestriction& restriction);
  void displayRestriction12(const RoutingRestriction& restriction);
  void displayRestriction12(const FlatSet<std::pair<Indicator, LocCode>>& cityGroup1,
                            const FlatSet<std::pair<Indicator, LocCode>>& cityGroup2);
  void displayRestriction13(const RoutingRestriction& restriction);
  void displayRestriction14(const RoutingRestriction& restriction);
  void displayRestriction15(const RoutingRestriction& restriction);
  void displayRestriction16(const RoutingRestriction& restriction);
  void displayRestriction17(const RoutingRestriction& restriction);
  void displayRestriction18(const RoutingRestriction& restriction);
  void displayRestriction19(const RoutingRestriction& restriction);
  void displayRestriction21(const RoutingRestriction& restriction);
  bool isRtw(bool expectRtw);
  bool isRtw();
  void displayRestrictionHeader(const RoutingRestriction& restriction,
                                const RestrictionInfo& restrictionStatus);

  void displayUnticketedPointInfo(const RoutingInfo& rtgInfo);
  void displayEntryExitPointInfo(const RoutingInfo& rtgInfo);
  void displayPSRs(const RoutingInfo& rtgInfo);
  void displayDRVInfos(const TravelRoute& tvlRoute, const RoutingInfo& rtgInfo);

private:
  bool _constructedRouting;
  bool _noOrigAddon;
  bool _origAddonMileage;
  bool _origAddonMap;
  bool _origAddonRestriction;
  bool _baseMileage;
  bool _baseMap;
  bool _baseRestriction;
  bool _noDestAddon;
  bool _destAddonMileage;
  bool _destAddonMap;
  bool _destAddonRestriction;
  std::string _routingStatus;
  std::string _gd;
};

} // namespace tse

