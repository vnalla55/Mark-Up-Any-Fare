//----------------------------------------------------------------------------
//
// Copyright Sabre 2004
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseConsts.h"
#include "Routing/MileageRouteItem.h"

namespace tse
{

class DataHandle;
class TpdPsr;
class DiagCollector;
class TPMExclusion;

class GDPrompt
{
public:
  const LocCode& origin() const { return _origin; }
  LocCode& origin() { return _origin; }
  const LocCode& destination() const { return _destination; }
  LocCode& destination() { return _destination; }
  GlobalDirection currentGD() const { return _currentGD; }
  GlobalDirection& currentGD() { return _currentGD; }
  const std::vector<GlobalDirection>& alternateGDs() const { return _alternateGDs; }
  std::vector<GlobalDirection>& alternateGDs() { return _alternateGDs; }

private:
  LocCode _origin, _destination;
  GlobalDirection _currentGD;
  std::vector<GlobalDirection> _alternateGDs;
};

/**
* @class MileageRoute.
* uniform data container class for all mileage processing.
* RoutingController passes a travelRoute for MileageValidation.
* and MileageService takes a MileageTrx for mileage processing
* MileageRoute represent both the inputs.
* Its a colllection of MileageRouteItems which are similar to
* TravelSegs of FareMarket and Items of MileageTrx.
*/
class MileageRoute
{
public:
  MileageRoute()
    : _mpm(0),
      _southAtlanticExclusion(false),
      _mileageRouteTPM(0),
      _globalDirection(GlobalDirection::ZZ),
      _tpd(0),
      _ems(0),
      _mileageEqualizationApplies(false),
      _governingCarrier(ANY_CARRIER),
      _dataHandle(nullptr),
      _stopoverCount(0),
      _hipExempt(false),
      _psrApplies(false),
      _psrMayApply(false),
      _applicablePSR(nullptr),
      _psrSetNumber(0),
      _geoLocMatched(false),
      _gdPrompt(nullptr),
      _isSouthAtlanticException(false),
      _isYYFare(false),
      _diagnosticHandle(nullptr),
      _crs(EMPTY_STRING()),
      _multiHost(EMPTY_STRING()),
      _tpmExclusion(nullptr),
      _specificCXR(false),
      _isOutbound(true)
  {
  }
  //_fareTypeAppl(' ') {}

  void partialInitialize(const MileageRoute& src)
  {
    _ticketingDT = src.ticketingDT();
    _travelDT = src.travelDT();

    _globalDirection = src.globalDirection();
    _governingCarrier = src.governingCarrier();

    _dataHandle = src.dataHandle();
  }
  void clone(const MileageRoute& src)
  {
    _ticketingDT = src._ticketingDT;
    _travelDT = src._travelDT;
    _mpm = src._mpm;
    _southAtlanticExclusion = src._southAtlanticExclusion;
    _mileageRouteTPM = src._mileageRouteTPM;
    _globalDirection = src._globalDirection;
    _tpd = src._tpd;
    _ems = src._ems;
    _mileageEqualizationApplies = src._mileageEqualizationApplies;
    _governingCarrier = src._governingCarrier;
    _dataHandle = src._dataHandle;
    _stopoverCount = src._stopoverCount;
    _hipExempt = src._hipExempt;
    _psrApplies = src._psrApplies;
    _psrMayApply = src._psrMayApply;
    _applicablePSR = src._applicablePSR;
    _psrSetNumber = src._psrSetNumber;
    _geoLocMatched = src._geoLocMatched;
    _gdPrompt = src._gdPrompt;
    _isSouthAtlanticException = src._isSouthAtlanticException;
    _isYYFare = src._isYYFare;
    _diagnosticHandle = src._diagnosticHandle;
    _crs = src._crs;
    _multiHost = src._multiHost;
    _tpmExclusion = src._tpmExclusion;
    _specificCXR = src._specificCXR;
    _isOutbound = src._isOutbound;
  }

  DateTime& ticketingDT() { return _ticketingDT; }
  const DateTime& ticketingDT() const { return _ticketingDT; }

  DateTime& travelDT() { return _travelDT; }
  const DateTime& travelDT() const { return _travelDT; }

  const MileageRouteItems& mileageRouteItems() const { return _mileageRouteItems; }
  MileageRouteItems& mileageRouteItems() { return _mileageRouteItems; }

  uint16_t mileageRouteMPM() const { return _mpm; }
  uint16_t& mileageRouteMPM() { return _mpm; }

  // This field is already subtracted by the route TPD value.
  uint16_t mileageRouteTPM() const { return _mileageRouteTPM; }
  uint16_t& mileageRouteTPM() { return _mileageRouteTPM; }

  GlobalDirection globalDirection() const { return _globalDirection; }
  GlobalDirection& globalDirection() { return _globalDirection; }

  const CarrierCode& governingCarrier() const { return _governingCarrier; }
  CarrierCode& governingCarrier() { return _governingCarrier; }

  // Mileage Exclusions
  bool southAtlanticExclusion() const { return _southAtlanticExclusion; }
  bool& southAtlanticExclusion() { return _southAtlanticExclusion; }

  uint16_t tpd() const { return _tpd; }
  uint16_t& tpd() { return _tpd; }

  uint16_t& ems() { return _ems; }
  uint16_t ems() const { return _ems; }

  bool mileageEqualizationApplies() const { return _mileageEqualizationApplies; }
  bool& mileageEqualizationApplies() { return _mileageEqualizationApplies; }

  DataHandle* dataHandle() const { return _dataHandle; }
  DataHandle*& dataHandle() { return _dataHandle; }

  uint16_t stopoverCount() const { return _stopoverCount; }
  uint16_t& stopoverCount() { return _stopoverCount; }

  bool hipExempt() const { return _hipExempt; }
  bool& hipExempt() { return _hipExempt; }

  bool psrApplies() const { return _psrApplies; }
  bool& psrApplies() { return _psrApplies; }

  bool psrMayApply() const { return _psrMayApply; }
  bool& psrMayApply() { return _psrMayApply; }

  TpdPsr* applicablePSR() const { return _applicablePSR; }
  TpdPsr*& applicablePSR() { return _applicablePSR; }

  uint32_t psrSetNumber() const { return _psrSetNumber; }
  uint32_t& psrSetNumber() { return _psrSetNumber; }

  bool geoLocMatched() const { return _geoLocMatched; }
  bool& geoLocMatched() { return _geoLocMatched; }

  bool southAtlanticExceptionApplies() const { return _isSouthAtlanticException; }
  bool& southAtlanticExceptionApplies() { return _isSouthAtlanticException; }

  // Indicator fareTypeAppl() const { return _fareTypeAppl; }
  // Indicator &fareTypeAppl() { return _fareTypeAppl; }

  GDPrompt* gdPrompt() const { return _gdPrompt; }
  GDPrompt*& gdPrompt() { return _gdPrompt; }

  bool& isYYFare() { return _isYYFare; }
  const bool& isYYFare() const { return _isYYFare; }

  DiagCollector*& diagnosticHandle() { return _diagnosticHandle; }
  DiagCollector* diagnosticHandle() const { return _diagnosticHandle; }

  std::string& crs() { return _crs; }
  const std::string& crs() const { return _crs; }

  std::string& multiHost() { return _multiHost; }
  const std::string& multiHost() const { return _multiHost; }

  TPMExclusion*& tpmExclusion() { return _tpmExclusion; }
  const TPMExclusion* tpmExclusion() const { return _tpmExclusion; }

  bool specificCXR() const { return _specificCXR; }
  bool& specificCXR() { return _specificCXR; }

  bool isOutbound() const { return _isOutbound; }
  bool& isOutbound() { return _isOutbound; }

  /* debug */
  void dump(std::ostream& os) const
  {
    std::string gd;
    globalDirectionToStr(gd, _globalDirection);
    os << "Route:" << std::endl << "GlobalDir: " << gd << std::endl
       << "GovCxr: " << _governingCarrier << std::endl << "TPD: " << _tpd << std::endl
       << "Items:" << std::endl;
    std::copy(_mileageRouteItems.begin(),
              _mileageRouteItems.end(),
              std::ostream_iterator<MileageRouteItem>(os));
  }

private:
  DateTime _ticketingDT;
  DateTime _travelDT;
  MileageRouteItems _mileageRouteItems;
  uint16_t _mpm;
  bool _southAtlanticExclusion;
  uint16_t _mileageRouteTPM;
  GlobalDirection _globalDirection;
  uint16_t _tpd;
  uint16_t _ems;
  bool _mileageEqualizationApplies;
  CarrierCode _governingCarrier;
  DataHandle* _dataHandle;
  uint16_t _stopoverCount;
  bool _hipExempt;
  bool _psrApplies;
  bool _psrMayApply;
  TpdPsr* _applicablePSR;
  uint32_t _psrSetNumber;
  bool _geoLocMatched;
  GDPrompt* _gdPrompt;
  bool _isSouthAtlanticException;
  // TPM exclusion
  bool _isYYFare;
  DiagCollector* _diagnosticHandle;
  std::string _crs;
  std::string _multiHost;
  TPMExclusion* _tpmExclusion;
  bool _specificCXR;
  bool _isOutbound;
  // Indicator _fareTypeAppl;
};

} // namespace tse

