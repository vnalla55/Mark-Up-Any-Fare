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

#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Loc.h"
#include "Routing/RoutingConsts.h"

#include <vector>

namespace tse
{

class Loc;
class LocKey;
class DataHandle;
class TpdPsrViaGeoLoc;

/* Single segment of mileage route */
class MileageRouteItem
{
public:
  MileageRouteItem()
    : _city1(nullptr),
      _city2(nullptr),
      _isSurface(false),
      _isConstructed(false),
      _tpmGlobalDirection(GlobalDirection::ZZ),
      _mpmGlobalDirection(GlobalDirection::ZZ),
      _tpm(0),
      _tpd(0),
      _mpm(0),
      _tpmSurfaceSectorExempt(false),
      _southAtlanticExclusion(false),
      _psrApplies(false),
      _psrMayApply(false),
      _segmentCarrier(INDUSTRY_CARRIER),
      _isStopover(true),
      _isDirectFromRouteBegin(true),
      _isDirectToRouteEnd(true),
      _isFirstOccurrenceFromRouteBegin(true),
      _isLastOccurrenceToRouteEnd(true),
      _failedDirService(false),
      _forcedConx('F'),
      _psrStopNotAllowed(false),
      _multiTransportOrigin(nullptr),
      _multiTransportDestination(nullptr),
      _pnrSegment(0),
      _forcedStopOver('F')
  {
  }

  Loc* city1() const { return _city1; }
  Loc*& city1() { return _city1; }
  Loc* city2() const { return _city2; }
  Loc*& city2() { return _city2; }
  const DateTime& travelDate() const { return _travelDate; }
  DateTime& travelDate() { return _travelDate; }
  bool isConstructed() const { return _isConstructed; }
  bool& isConstructed() { return _isConstructed; }
  bool isSurface() const { return _isSurface; }
  bool& isSurface() { return _isSurface; }
  bool isStopover() const { return _isStopover; }
  bool& isStopover() { return _isStopover; }
  bool isDirectFromRouteBegin() const { return _isDirectFromRouteBegin; }
  bool& isDirectFromRouteBegin() { return _isDirectFromRouteBegin; }
  bool isDirectToRouteEnd() const { return _isDirectToRouteEnd; }
  bool& isDirectToRouteEnd() { return _isDirectToRouteEnd; }
  bool isFirstOccurrenceFromRouteBegin() const { return _isFirstOccurrenceFromRouteBegin; }
  bool& isFirstOccurrenceFromRouteBegin() { return _isFirstOccurrenceFromRouteBegin; }
  bool isLastOccurrenceToRouteEnd() const { return _isLastOccurrenceToRouteEnd; }
  bool& isLastOccurrenceToRouteEnd() { return _isLastOccurrenceToRouteEnd; }
  char forcedConx() const { return _forcedConx; }
  char& forcedConx() { return _forcedConx; }
  char& forcedStopOver() { return _forcedStopOver; }
  const char forcedStopOver() const { return _forcedStopOver; }

  const CarrierCode& segmentCarrier() const { return _segmentCarrier; }
  CarrierCode& segmentCarrier() { return _segmentCarrier; }
  const GlobalDirection& tpmGlobalDirection() const { return _tpmGlobalDirection; }
  GlobalDirection& tpmGlobalDirection() { return _tpmGlobalDirection; }
  const GlobalDirection& mpmGlobalDirection() const { return _mpmGlobalDirection; }
  GlobalDirection& mpmGlobalDirection() { return _mpmGlobalDirection; }
  uint16_t tpm() const { return _tpm; }
  uint16_t& tpm() { return _tpm; }
  uint16_t tpd() const { return _tpd; }
  uint16_t& tpd() { return _tpd; }
  uint16_t mpm() const { return _mpm; }
  uint16_t& mpm() { return _mpm; }
  bool failedDirService() const { return _failedDirService; }
  bool& failedDirService() { return _failedDirService; }
  uint16_t mileage(Indicator mileageType) const { return (mileageType == TPM ? _tpm : _mpm); }
  uint16_t& mileage(Indicator mileageType) { return (mileageType == TPM ? _tpm : _mpm); }
  GlobalDirection globalDirection(Indicator mileageType) const
  {
    return (mileageType == TPM ? _tpmGlobalDirection : _mpmGlobalDirection);
  }
  GlobalDirection& globalDirection(Indicator mileageType)
  {
    return (mileageType == TPM ? _tpmGlobalDirection : _mpmGlobalDirection);
  }

  const std::vector<const Loc*>& hiddenLocs() const { return _hiddenLocs; }
  std::vector<const Loc*>& hiddenLocs() { return _hiddenLocs; }
  bool tpmSurfaceSectorExempt() const { return _tpmSurfaceSectorExempt; }
  bool& tpmSurfaceSectorExempt() { return _tpmSurfaceSectorExempt; }
  bool southAtlanticExclusion() const { return _southAtlanticExclusion; }
  bool& southAtlanticExclusion() { return _southAtlanticExclusion; }
  bool psrApplies() const { return _psrApplies; }
  bool& psrApplies() { return _psrApplies; }
  bool psrMayApply() const { return _psrMayApply; }
  bool& psrMayApply() { return _psrMayApply; }
  bool psrStopNotAllowed() const { return _psrStopNotAllowed; }
  bool& psrStopNotAllowed() { return _psrStopNotAllowed; }

  int16_t& pnrSegment() { return _pnrSegment; }
  const int16_t& pnrSegment() const { return _pnrSegment; }

  const std::vector<const TpdPsrViaGeoLoc*>& condTpdViaGeoLocs() const
  {
    return _condTpdViaGeoLocs;
  }
  std::vector<const TpdPsrViaGeoLoc*>& condTpdViaGeoLocs() { return _condTpdViaGeoLocs; }

  Loc* multiTransportOrigin() const { return _multiTransportOrigin; }
  Loc*& multiTransportOrigin() { return _multiTransportOrigin; }
  Loc* multiTransportDestination() const { return _multiTransportDestination; }
  Loc*& multiTransportDestination() { return _multiTransportDestination; }

  Loc* origCityOrAirport() const
  {
    return _multiTransportOrigin != nullptr ? _multiTransportOrigin : _city1;
  }
  Loc*& origCityOrAirport() { return _multiTransportOrigin != nullptr ? _multiTransportOrigin : _city1; }
  Loc* destCityOrAirport() const
  {
    return _multiTransportDestination != nullptr ? _multiTransportDestination : _city2;
  }
  Loc*& destCityOrAirport()
  {
    return _multiTransportDestination != nullptr ? _multiTransportDestination : _city2;
  }

  // These functions test that origin/destination is in location provided by locKey.
  // It tests both city*() and multiTransport*() fields and returns true if either of them
  // succeeds.
  bool isOriginInLoc(const LocKey& locKey) const;
  bool isDestinationInLoc(const LocKey& locKey) const;

  /* Makes a deep copy of city1 and city2 and hiddenLocs but NOT tpdViaGeoLocs */
  void clone(DataHandle&, MileageRouteItem&);

  /* debug */
  friend std::ostream& operator<<(std::ostream& os, const MileageRouteItem& item)
  {
    std::string tpmGD, mpmGD;
    globalDirectionToStr(tpmGD, item._tpmGlobalDirection);
    globalDirectionToStr(mpmGD, item._mpmGlobalDirection);
    os << "orig: " << item._city1->loc() << ", dest: " << item._city2->loc()
       << (item._isSurface ? ", surf" : "") << (item._isStopover ? ", stpOvr" : "")
       << (item._isDirectFromRouteBegin ? ", directFromOrig" : "")
       << (item._isDirectToRouteEnd ? ", directToDest" : "")
       << (item._isFirstOccurrenceFromRouteBegin ? ", stopBtwOrig" : "")
       << (item._isLastOccurrenceToRouteEnd ? ", stopBtwDest" : "")
       << (item._failedDirService ? ", failedDirService" : "") << ", cxr: " << item._segmentCarrier
       << ", tpmGD: " << tpmGD << ", mpmGD: " << mpmGD << ", hids: " << item._hiddenLocs.size()
       << ", TPD: " << item._tpd << std::endl;
    return os;
  }

private:
  Loc* _city1;
  Loc* _city2;
  DateTime _travelDate;
  bool _isSurface;
  bool _isConstructed;
  GlobalDirection _tpmGlobalDirection;
  GlobalDirection _mpmGlobalDirection;
  uint16_t _tpm;
  uint16_t _tpd;
  uint16_t _mpm;
  bool _tpmSurfaceSectorExempt;
  bool _southAtlanticExclusion;
  bool _psrApplies;
  bool _psrMayApply;
  CarrierCode _segmentCarrier;
  bool _isStopover;
  bool _isDirectFromRouteBegin;
  bool _isDirectToRouteEnd;
  bool _isFirstOccurrenceFromRouteBegin;
  bool _isLastOccurrenceToRouteEnd;
  bool _failedDirService;
  char _forcedConx;
  bool _psrStopNotAllowed;
  std::vector<const Loc*> _hiddenLocs;
  std::vector<const TpdPsrViaGeoLoc*> _condTpdViaGeoLocs;
  Loc* _multiTransportOrigin;
  Loc* _multiTransportDestination;
  int16_t _pnrSegment;
  char _forcedStopOver;
};

typedef std::vector<MileageRouteItem> MileageRouteItems;

} // namespace tse

