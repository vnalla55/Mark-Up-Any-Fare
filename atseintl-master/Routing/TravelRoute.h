//----------------------------------------------------------------------------
//
//  Copyright Sabre 2003
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

#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Routing/RoutingConsts.h"

#include <vector>

namespace tse
{

/** @class TravelRoute
 * This class selects the governing carrier for each FareMarket.
 * Each fare market calls selectGoverningCarrier() to determine
 * the governing carrier.
 * Governing Carrier selection logic varies with the travel
 * boundary of the FareMaket and a FareMarket can have exactly
 * one governing carrier.
 *
 */
class TravelSeg;
class FareMarket;
class TravelRoute
{

public:
  TravelRoute()
    : _flightTrackingCxr(false),
      _glbDir(GlobalDirection::ZZ),
      _maxPermittedMileage(0),
      _totalTPM(0),
      _doNotApplyDRV(false),
      _doNotApplyDRVExceptUS(false),
      _terminalPoints(false),
      _primarySector(nullptr),
      _geoTravelType(GeoTravelType::UnknownGeoTravelType),
      _travelRouteTktOnly(nullptr),
      _unticketedPointInd(IGNORE_TKTPTSIND) {};
  virtual ~TravelRoute() {};

  struct City
  {
  public:
    City() : _isHiddenCity(false) {}

    LocCode& loc() { return _city; }
    const LocCode& loc() const { return _city; }
    LocCode& airport() { return _airport; }
    const LocCode& airport() const { return _airport; }
    bool& isHiddenCity() { return _isHiddenCity; }
    const bool& isHiddenCity() const { return _isHiddenCity; }

  private:
    LocCode _city;
    LocCode _airport;
    bool _isHiddenCity;
  };

  struct CityCarrier
  {
    CityCarrier() : _stopover(false) {}

    City& boardCity() { return _boardCity; }
    NationCode& boardNation() { return _boardNation; }
    City& offCity() { return _offCity; }
    NationCode& offNation() { return _offNation; }
    CarrierCode& carrier() { return _carrier; }
    const CarrierCode& carrier() const { return _carrier; }
    const City& offCity() const { return _offCity; }
    const City& boardCity() const { return _boardCity; }
    const NationCode& boardNation() const { return _boardNation; }
    const NationCode& offNation() const { return _offNation; }
    bool& stopover() { return _stopover; }
    const bool& stopover() const { return _stopover; }
    GenericAllianceCode &genericAllianceCode() { return _genericAllianceCode; }
    const GenericAllianceCode &genericAllianceCode() const { return _genericAllianceCode; }

  private:
    City _boardCity;
    NationCode _boardNation;
    City _offCity;
    NationCode _offNation;
    CarrierCode _carrier;
    bool _stopover;
    GenericAllianceCode _genericAllianceCode;
  };

  // convenience methods
  //
  bool empty() const { return _travelRoute.empty(); }

  // accessors from heck
  //
  std::vector<CityCarrier>& travelRoute() { return _travelRoute; }
  const std::vector<CityCarrier>& travelRoute() const { return _travelRoute; }
  std::vector<TravelSeg*>& mileageTravelRoute() { return tvlSegs; }
  const std::vector<TravelSeg*>& mileageTravelRoute() const { return tvlSegs; }
  LocCode& origin() { return orig; }
  const LocCode& origin() const { return orig; }
  NationCode& originNation() { return origNation; }
  const NationCode& originNation() const { return origNation; }
  LocCode& destination() { return dest; }
  const LocCode& destination() const { return dest; }
  NationCode& destinationNation() { return destNation; }
  const NationCode& destinationNation() const { return destNation; }
  CarrierCode& govCxr() { return _govCxr; }
  const CarrierCode& govCxr() const { return _govCxr; }
  bool& flightTrackingCxr() { return _flightTrackingCxr; }
  const bool& flightTrackingCxr() const { return _flightTrackingCxr; }
  GlobalDirection& globalDir() { return _glbDir; }
  const GlobalDirection globalDir() const { return _glbDir; }
  DateTime& travelDate() { return _travelDate; }
  const DateTime travelDate() const { return _travelDate; }
  uint16_t& maxPermittedMileage() { return _maxPermittedMileage; }
  const uint16_t maxPermittedMileage() const { return _maxPermittedMileage; }
  uint16_t& totalTPM() { return _totalTPM; }
  const uint16_t totalTPM() const { return _totalTPM; }
  bool& doNotApplyDRV() { return _doNotApplyDRV; }
  const bool doNotApplyDRV() const { return _doNotApplyDRV; }
  bool& doNotApplyDRVExceptUS() { return _doNotApplyDRVExceptUS; }
  const bool doNotApplyDRVExceptUS() const { return _doNotApplyDRVExceptUS; }
  bool& terminalPoints() { return _terminalPoints; }
  const bool terminalPoints() const { return _terminalPoints; }
  TravelSeg*& primarySector() { return _primarySector; }
  TravelSeg* primarySector() const { return _primarySector; }
  GeoTravelType& geoTravelType() { return _geoTravelType; }
  GeoTravelType geoTravelType() const { return _geoTravelType; }
  TravelRoute*& travelRouteTktOnly() { return _travelRouteTktOnly; }
  const TravelRoute* travelRouteTktOnly() const { return _travelRouteTktOnly; }
  Indicator& unticketedPointInd() { return _unticketedPointInd; }
  const Indicator unticketedPointInd() const { return _unticketedPointInd; }

private:
  LocCode orig;
  NationCode origNation;
  LocCode dest;
  NationCode destNation;
  CarrierCode _govCxr;
  bool _flightTrackingCxr;
  GlobalDirection _glbDir;
  DateTime _travelDate;
  uint16_t _maxPermittedMileage;
  uint16_t _totalTPM;
  bool _doNotApplyDRV;
  bool _doNotApplyDRVExceptUS;
  bool _terminalPoints;
  TravelSeg* _primarySector;
  GeoTravelType _geoTravelType;
  TravelRoute* _travelRouteTktOnly;
  Indicator _unticketedPointInd;

  std::vector<CityCarrier> _travelRoute;
  std::vector<TravelSeg*> tvlSegs;
};
}
