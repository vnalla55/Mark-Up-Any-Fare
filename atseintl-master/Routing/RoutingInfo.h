//----------------------------------------------------------------------------
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
#include "DBAccess/TpdPsr.h"
#include "Routing/MapInfo.h"
#include "Routing/MileageInfo.h"
#include "Routing/RestrictionInfo.h"
#include "Routing/RtgKey.h"
#include "Routing/TravelRoute.h"

#include <map>
#include <vector>

namespace tse
{
class RoutingRestriction;
class Routing;
class RoutingInfo;

using RestrictionInfos = std::map<const RoutingRestriction*, RestrictionInfo>;
using CityCarrierVec = std::vector<TravelRoute::CityCarrier>;

class RoutingInfo
{
public:
  RtgKey& rtgKey() { return _rtgKey; }
  const RtgKey& rtgKey() const { return _rtgKey; }

  bool routingStatus() const { return _routingStatus; }
  bool& routingStatus() { return _routingStatus; }
  bool routingMapStatus() const { return _routingMapStatus; }
  bool& routingMapStatus() { return _routingMapStatus; }
  const MileageInfo* mileageInfo() const { return _mileageInfo; }
  MileageInfo*& mileageInfo() { return _mileageInfo; }
  MapInfo*& mapInfo() { return _mapInfo; }
  const MapInfo* mapInfo() const { return _mapInfo; }
  MapInfo*& rtgAddonMapInfo() { return _rtgAddonMapInfo; }
  const MapInfo* rtgAddonMapInfo() const { return _rtgAddonMapInfo; }
  const RestrictionInfos* restrictions() const { return _restrictions; }
  RestrictionInfos*& restrictions() { return _restrictions; }

  const Routing*& routing() { return _routing; }
  const Routing* routing() const { return _routing; }

  const Routing*& origAddOnRouting() { return _origAddOnRouting; }
  const Routing* origAddOnRouting() const { return _origAddOnRouting; }
  const Routing*& destAddOnRouting() { return _destAddOnRouting; }
  const Routing* destAddOnRouting() const { return _destAddOnRouting; }

  const TariffNumber& routingTariff() const { return _routingTariff; }
  TariffNumber& routingTariff() { return _routingTariff; }

  const TariffCode& tcrRoutingTariffCode() const { return _tcrRoutingTariffCode; }
  TariffCode& tcrRoutingTariffCode() { return _tcrRoutingTariffCode; }
  const TariffCode& tcrAddonTariff1Code() const { return _tcrAddonTariff1Code; }
  TariffCode& tcrAddonTariff1Code() { return _tcrAddonTariff1Code; }
  const TariffCode& tcrAddonTariff2Code() const { return _tcrAddonTariff2Code; }
  TariffCode& tcrAddonTariff2Code() { return _tcrAddonTariff2Code; }

  const LocCode& origAddOnGateway() const { return _origAddOnGateway; }
  LocCode& origAddOnGateway() { return _origAddOnGateway; }
  const LocCode& origAddOnInterior() const { return _origAddOnInterior; }
  LocCode& origAddOnInterior() { return _origAddOnInterior; }

  const LocCode& market1() const { return _market1; }
  LocCode& market1() { return _market1; }
  const LocCode& market2() const { return _market2; }
  LocCode& market2() { return _market2; }

  const LocCode& destAddOnGateway() const { return _destAddOnGateway; }
  LocCode& destAddOnGateway() { return _destAddOnGateway; }
  const LocCode& destAddOnInterior() const { return _destAddOnInterior; }
  LocCode& destAddOnInterior() { return _destAddOnInterior; }

  const CityCarrierVec& rtgAddon1CityCarrier() const { return _rtgAddon1CityCarrier; }
  CityCarrierVec& rtgAddon1CityCarrier() { return _rtgAddon1CityCarrier; }
  const CityCarrierVec& rtgAddon2CityCarrier() const { return _rtgAddon2CityCarrier; }
  CityCarrierVec& rtgAddon2CityCarrier() { return _rtgAddon2CityCarrier; }
  const CityCarrierVec& rtgBaseCityCarrier() const { return _rtgBaseCityCarrier; }
  CityCarrierVec& rtgBaseCityCarrier() { return _rtgBaseCityCarrier; }
  const CityCarrierVec& restBaseCityCarrier() const { return _restBaseCityCarrier; }
  CityCarrierVec& restBaseCityCarrier() { return _restBaseCityCarrier; }

  const std::vector<TpdPsr*>& fdPSR() const { return _fdPSR; }
  std::vector<TpdPsr*>& fdPSR() { return _fdPSR; }

  const std::vector<TpdPsr*>& fdTPD() const { return _fdTPD; }
  std::vector<TpdPsr*>& fdTPD() { return _fdTPD; }

private:
  RtgKey _rtgKey;
  bool _routingStatus = false;
  bool _routingMapStatus = false;
  MileageInfo* _mileageInfo = nullptr;
  MapInfo* _mapInfo = nullptr;
  MapInfo* _rtgAddonMapInfo = nullptr;
  RestrictionInfos* _restrictions = nullptr;
  const Routing* _routing = nullptr;
  const Routing* _origAddOnRouting = nullptr;
  const Routing* _destAddOnRouting = nullptr;
  TariffNumber _routingTariff = 0;
  TariffCode _tcrRoutingTariffCode;
  TariffCode _tcrAddonTariff1Code;
  TariffCode _tcrAddonTariff2Code;
  LocCode _origAddOnGateway;
  LocCode _origAddOnInterior;
  LocCode _market1;
  LocCode _market2;
  LocCode _destAddOnGateway;
  LocCode _destAddOnInterior;
  CityCarrierVec _rtgAddon1CityCarrier;
  CityCarrierVec _rtgAddon2CityCarrier;
  CityCarrierVec _rtgBaseCityCarrier;
  CityCarrierVec _restBaseCityCarrier;
  //---------------------------------
  // This vector is added for FareDisplay.
  // We need to display all PSR and TPD applied.
  // --------------------------------
  std::vector<TpdPsr*> _fdPSR;
  std::vector<TpdPsr*> _fdTPD;
};

using RoutingInfos = std::map<const RtgKey, RoutingInfo*>;
} // namespace tse

