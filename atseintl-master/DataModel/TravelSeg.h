//-------------------------------------------------------------------
//
//  File:        TravelSeg.h
//  Created:
//  Authors:
//
//  Description: Common base class for any itinerary segment.
//
//               Class TravelSeg SHOULD NOT be instantiated by itself.
//               So there is no pool for such kind of objects.
//
//  Updates:
//          03/08/04 - VN - file created.
//          04/05/04 - Mike Carroll - Member updates
//          08/05/05 - Tony Lam - Date Range
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
//-------------------------------------------------------------------

#pragma once

#include "Common/Assert.h"
#include "Common/CabinType.h"
#include "Common/HiddenStopDetails.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TypeConvert.h"

#include <map>
#include <vector>

namespace tse
{
class AirSeg;
class ArunkSeg;
class CarrierPreference;
class ClassOfService;
class DataHandle;
class Itin;
class Loc;
class PaxTypeFare;
class PricingTrx;
class TimeAndUnit;

class TravelSeg
{
public:
  enum Application
  { TAXES,
    OTHER,
    BAGGAGE };
  enum ChangeStatus
  { UNCHANGED,
    CHANGED,
    INVENTORYCHANGED,
    CONFIRMOPENSEGMENT };

  enum BookedCabinChars
  { PREMIUM_FIRST_CLASS = 'P',
    FIRST_CLASS = 'F',
    PREMIUM_BUSINESS_CLASS = 'J',
    BUSINESS_CLASS = 'C',
    PREMIUM_ECONOMY_CLASS = 'S',
    ECONOMY_CLASS = 'Y' };

  enum BookedCabinCharsAnswer
  { PREMIUM_FIRST_CLASS_ANSWER = 'R',
    FIRST_CLASS_ANSWER = 'F',
    PREMIUM_BUSINESS_CLASS_ANSWER = 'J',
    BUSINESS_CLASS_ANSWER = 'C',
    PREMIUM_ECONOMY_CLASS_ANSWER = 'W',
    ECONOMY_CLASS_ANSWER = 'Y' };

  enum SecondaryRBDReasonCode
  {
    DONOTZEROOUT,
    INTERNATIONALFLIGHTS_PBKC,
    DOMESTICFLIGHTS_PBKC,
    DOMESTICFLIGHTS_NO_PBKC};

  struct RequestedFareBasis
  {
    bool operator==(const RequestedFareBasis& rhs) const;

    FareBasisCode fareBasisCode;
    PaxTypeCode passengerCode;
    VendorCode vendor;
    CarrierCode carrier;
    TariffCode tariff;
    RuleNumber rule;
    MoneyAmount amount;
  };

  virtual inline ~TravelSeg() = 0;

protected:
  TravelSeg()
    : _changeStatus(1, CHANGED), _newTravelUsedToSetChangeStatus(1), _isCabinChanged(1, false)
  {
  }

  //-------------------------------------------------------------------------
  // Members
  //-------------------------------------------------------------------------

  int16_t _pnrSegment = 0; // FSN
  int16_t _segmentOrder = 0; // SMN
  uint32_t _mileage = 0;
  GlobalDirection _globalDirection = GlobalDirection::NO_DIR;
  TravelSegType _segmentType = TravelSegType::UnknownTravelSegType; // STI
  PSSDate _pssDepartureDate; // DTE
  PSSTime _pssDepartureTime; // DEP
  DateTime _departureDT; // Hybrid from DTE DEP
  PSSDate _pssBookingDate; // DAT
  PSSTime _pssBookingTime; // TIM
  DateTime _bookingDT; // Hybrid from DAT TIM
  PSSDate _pssArrivalDate; // ADT
  PSSTime _pssArrivalTime; // ATM
  DateTime _arrivalDT; // Hybrid from ADT ATM
  BookingCode _bookingCode; // COS
  CabinType _bookedCabin; // From DB
  std::string _resStatus; // AAC
  std::string _realResStatus; // BB1
  LocCode _origAirport; // BPC
  LocCode _destAirport; // OPC
  std::string _fareBasisCode; // SBC
  char _fbcUsage = COMMAND_PRICE_FBC; // A07
  std::string _specifiedFbc; // FBC
  std::string _fareCalcFareAmt; // C50
  std::string _globalDirectionOverride; // A60
  char _forcedConx = 0; // SFC
  char _forcedStopOver = 0; // SFS
  char _forcedFareBrk = 0; // SFB
  char _forcedNoFareBrk = 0; // SNB
  char _forcedSideTrip = 0; // FST
  LocCode _boardMultiCity; // Not in PSS XML
  LocCode _offMultiCity; // Not in PSS XML
  std::vector<ClassOfService*> _classOfService; // Not in PSS XML

  std::string _validatedBookingCode; // Not in PSS XML
  DateTime _earliestDepartureDT; //
  DateTime _latestDepartureDT; //
  DateTime _earliestArrivalDT; //
  DateTime _latestArrivalDT; //
  bool _hasEmptyDate = false; // if there is not D01 in XML

  GeoTravelType _geoTravelType = GeoTravelType::UnknownGeoTravelType;

  bool _stopOver = false;

  const Loc* _origin = nullptr;
  const Loc* _destination = nullptr;

  std::vector<const Loc*> _hiddenStops;
  std::vector<HiddenStopDetails> _hiddenStopsDetails;
  std::vector<EquipmentType> _equipmentTypes;
  std::vector<RequestedFareBasis> _requestedFareBasis;

  EquipmentType _equipmentType;
  bool _furthestPoint = false;
  bool _retransited = false;

  bool _openSegAfterDatedSeg = false;

  const CarrierPreference* _carrierPref = nullptr;
  bool _isRoundTheWorld = false; // for FQD when origAirport = destAirport
  // For Reissue & Exchange
  std::vector<ChangeStatus> _changeStatus;
  bool _unflown = true;
  std::vector<std::vector<TravelSeg*>> _newTravelUsedToSetChangeStatus;
  std::deque<bool> _isCabinChanged;
  bool _isShopped = false;
  // Itin index segment is compared against ,for new itin seg always 0 because
  // there is one exc itin
  uint16_t _itinIndex = 0;

  // Q1K from request ( +1 in order to not start from 0 )
  uint16_t _originalId = TRAVEL_SEG_DEFAULT_ID;
  // Id of travel leg (-1 means that it's not set)
  int16_t _legId = -1; // Q0L

  int _originGmtAdjustment = 0;
  int _destinationGmtAdjustment = 0;
  uint16_t _ticketCouponNumber = 0;
  char _checkedPortionOfTravelInd = ' ';
  BrandCode _brandCode; // SB2
  SecondaryRBDReasonCode _secondaryRBDReasonCode = DONOTZEROOUT;
  bool _rbdReplaced = false;


public:
  virtual bool initialize();

  virtual bool isAir() const;

  virtual AirSeg* toAirSeg() { return nullptr; }
  virtual const AirSeg* toAirSeg() const { return nullptr; }
  virtual AirSeg& toAirSegRef(); /*throw runtime_error for non AirSeg*/
  virtual const AirSeg& toAirSegRef() const; /*throw runtime_error for non AirSeg*/

  virtual bool isArunk() const;

  virtual ArunkSeg* toArunkSeg() { return nullptr; }
  virtual const ArunkSeg* toArunkSeg() const { return nullptr; }
  virtual ArunkSeg& toArunkSegRef(); /*throws runtime_error for non ArunkSeg*/
  virtual const ArunkSeg& toArunkSegRef() const; /*throws runtime_error for non ArunkSeg*/

  virtual TravelSeg* clone(DataHandle& dh) const = 0;

  //--------------------------------------------------------------------------
  // Accessors
  //--------------------------------------------------------------------------
  int16_t& pnrSegment() { return _pnrSegment; }
  const int16_t& pnrSegment() const { return _pnrSegment; }

  int16_t& segmentOrder() { return _segmentOrder; }
  const int16_t& segmentOrder() const { return _segmentOrder; }

  uint32_t& mileage() { return _mileage; }
  const uint32_t& mileage() const { return _mileage; }

  GlobalDirection& globalDirection() { return _globalDirection; }
  const GlobalDirection& globalDirection() const { return _globalDirection; }

  TravelSegType& segmentType() { return _segmentType; }
  const TravelSegType& segmentType() const { return _segmentType; }

  PSSDate& pssDepartureDate() { return _pssDepartureDate; }
  const PSSDate& pssDepartureDate() const { return _pssDepartureDate; }

  PSSTime& pssDepartureTime() { return _pssDepartureTime; }
  const PSSTime& pssDepartureTime() const { return _pssDepartureTime; }

  DateTime& departureDT() { return _departureDT; }
  const DateTime& departureDT() const { return _departureDT; }

  DateTime& earliestDepartureDT() { return _earliestDepartureDT; }
  const DateTime& earliestDepartureDT() const { return _earliestDepartureDT; }

  DateTime& latestDepartureDT() { return _latestDepartureDT; }
  const DateTime& latestDepartureDT() const { return _latestDepartureDT; }

  PSSDate& pssArrivalDate() { return _pssArrivalDate; }
  const PSSDate& pssArrivalDate() const { return _pssArrivalDate; }

  PSSTime& pssArrivalTime() { return _pssArrivalTime; }
  const PSSTime& pssArrivalTime() const { return _pssArrivalTime; }

  DateTime& arrivalDT() { return _arrivalDT; }
  const DateTime& arrivalDT() const { return _arrivalDT; }

  DateTime& earliestArrivalDT() { return _earliestArrivalDT; }
  const DateTime& earliestArrivalDT() const { return _earliestArrivalDT; }

  DateTime& latestArrivalDT() { return _latestArrivalDT; }
  const DateTime& latestArrivalDT() const { return _latestArrivalDT; }

  PSSDate& pssBookingDate() { return _pssBookingDate; }
  const PSSDate& pssBookingDate() const { return _pssBookingDate; }

  PSSTime& pssBookingTime() { return _pssBookingTime; }
  const PSSTime& pssBookingTime() const { return _pssBookingTime; }

  DateTime& bookingDT() { return _bookingDT; }
  const DateTime& bookingDT() const { return _bookingDT; }

  std::vector<ClassOfService*>& classOfService() { return _classOfService; }
  const std::vector<ClassOfService*>& classOfService() const { return _classOfService; }

  std::string& validatedBookingCode() { return _validatedBookingCode; }
  const std::string& validatedBookingCode() const { return _validatedBookingCode; }

  std::string& resStatus() { return _resStatus; }
  const std::string& resStatus() const { return _resStatus; }

  std::string& realResStatus() { return _realResStatus; }
  const std::string& realResStatus() const { return _realResStatus; }

  LocCode& origAirport() { return _origAirport; }
  const LocCode& origAirport() const { return _origAirport; }

  LocCode& destAirport() { return _destAirport; }
  const LocCode& destAirport() const { return _destAirport; }

  std::string& fareBasisCode() { return _fareBasisCode; }
  const std::string& fareBasisCode() const { return _fareBasisCode; }

  char& fbcUsage() { return _fbcUsage; }
  const char& fbcUsage() const { return _fbcUsage; }

  std::string& specifiedFbc() { return _specifiedFbc; }
  const std::string& specifiedFbc() const { return _specifiedFbc; }

  std::string& fareCalcFareAmt() { return _fareCalcFareAmt; }
  const std::string& fareCalcFareAmt() const { return _fareCalcFareAmt; }

  std::string& globalDirectionOverride() { return _globalDirectionOverride; }
  const std::string& globalDirectionOverride() const { return _globalDirectionOverride; }

  char& forcedConx() { return _forcedConx; }
  const char& forcedConx() const { return _forcedConx; }
  const bool isForcedConx() const { return TypeConvert::pssCharToBool(_forcedConx); }

  char& forcedStopOver() { return _forcedStopOver; }
  const char& forcedStopOver() const { return _forcedStopOver; }
  const bool isForcedStopOver() const { return TypeConvert::pssCharToBool(_forcedStopOver); }

  char& forcedFareBrk() { return _forcedFareBrk; }
  const char& forcedFareBrk() const { return _forcedFareBrk; }
  const bool isForcedFareBrk() const { return TypeConvert::pssCharToBool(_forcedFareBrk); }

  char& forcedNoFareBrk() { return _forcedNoFareBrk; }
  const char& forcedNoFareBrk() const { return _forcedNoFareBrk; }
  const bool isForcedNoFareBrk() const { return TypeConvert::pssCharToBool(_forcedNoFareBrk); }

  char& forcedSideTrip() { return _forcedSideTrip; }
  const char& forcedSideTrip() const { return _forcedSideTrip; }
  const bool isForcedSideTrip() const { return TypeConvert::pssCharToBool(_forcedSideTrip); }

  LocCode& boardMultiCity() { return _boardMultiCity; }
  const LocCode& boardMultiCity() const { return _boardMultiCity; }

  LocCode& offMultiCity() { return _offMultiCity; }
  const LocCode& offMultiCity() const { return _offMultiCity; }

  void setBookingCode(const BookingCode& code) { _bookingCode = code; }
  const BookingCode& getBookingCode() const { return _bookingCode; }

  CabinType& bookedCabin() { return _bookedCabin; }
  const CabinType& bookedCabin() const { return _bookedCabin; }
  char bookedCabinChar() const;
  char bookedCabinCharAnswer() const;

  GeoTravelType& geoTravelType() { return _geoTravelType; }
  const GeoTravelType geoTravelType() const { return _geoTravelType; }

  bool& stopOver() { return _stopOver; }
  const bool stopOver() const { return _stopOver; }

  const Loc*& origin() { return _origin; }
  const Loc* origin() const { return _origin; }

  const Loc*& destination() { return _destination; }
  const Loc* destination() const { return _destination; }

  std::vector<const Loc*>& hiddenStops() { return _hiddenStops; }
  const std::vector<const Loc*>& hiddenStops() const { return _hiddenStops; }

  std::vector<HiddenStopDetails>& hiddenStopsDetails() { return _hiddenStopsDetails; }
  const std::vector<HiddenStopDetails>& hiddenStopsDetails() const { return _hiddenStopsDetails; }

  std::vector<EquipmentType>& equipmentTypes() { return _equipmentTypes; }
  const std::vector<EquipmentType>& equipmentTypes() const { return _equipmentTypes; }

  std::vector<RequestedFareBasis>& requestedFareBasis() { return _requestedFareBasis; }
  const std::vector<RequestedFareBasis>& requestedFareBasis() const { return _requestedFareBasis; }

  EquipmentType& equipmentType() { return _equipmentType; }
  const EquipmentType& equipmentType() const { return _equipmentType; }

  bool& furthestPoint() { return _furthestPoint; }
  const bool furthestPoint() const { return _furthestPoint; }

  bool& furthestPoint(const Itin&) { return _furthestPoint; }
  const bool furthestPoint(const Itin&) const { return _furthestPoint; }

  bool& retransited() { return _retransited; }
  const bool retransited() const { return _retransited; }

  bool isRequestedFareBasisInfoUseful() const { return !_requestedFareBasis.empty(); }
  bool isRequestedFareBasisValid(const RequestedFareBasis& req,
                                 const PaxTypeFare& ptf,
                                 PricingTrx& trx) const;
  bool matchFbc(const std::string& fbc, const PaxTypeFare& ptf, PricingTrx& trx) const;


  bool isInternationalSegment() const;

  bool isFurthestPoint(const Itin& itin) const;

  bool isStopOver(const TravelSeg* travelSeg,
                  const GeoTravelType geoTravelType,
                  Application application = OTHER,
                  int64_t minSeconds = 86400) const;
  bool isStopOver(const TravelSeg* travelSeg, int64_t minSeconds) const;
  bool isStopOverWithOutForceCnx(const TravelSeg* travelSeg, int64_t minSeconds) const;

  // Stopover for TSI.
  bool isStopOver(const TravelSeg* nextTravelSeg,
                  const GeoTravelType geoTravelType,
                  const TimeAndUnit& minTime) const;

  void setSecondaryRBDReasonCodeStatus(const SecondaryRBDReasonCode& zorc){ _secondaryRBDReasonCode = zorc; }
  SecondaryRBDReasonCode& getSecondaryRBDReasonCodeStatus() {return _secondaryRBDReasonCode;}
  const SecondaryRBDReasonCode& getSecondaryRBDReasonCodeStatus() const {return _secondaryRBDReasonCode;}

private:
  // Checks with
  bool isStopOverWithoutForceCnx(const TravelSeg* nextTravelSeg,
                                 const GeoTravelType geoTravelType,
                                 const TimeAndUnit& minTime) const;

public:
  bool isOpenWithoutDate() const;

  bool& openSegAfterDatedSeg() { return _openSegAfterDatedSeg; }
  const bool openSegAfterDatedSeg() const { return _openSegAfterDatedSeg; }

  const CarrierPreference*& carrierPref() { return _carrierPref; }

  const CarrierPreference* carrierPref() const { return _carrierPref; }

  // For Reissue & Exchange
  std::vector<ChangeStatus>& changeStatusVec() { return _changeStatus; }
  const std::vector<ChangeStatus>& changeStatusVec() const { return _changeStatus; }

  std::deque<bool>& isCabinChangedVec() { return _isCabinChanged; }
  const std::deque<bool>& isCabinChangedVec() const { return _isCabinChanged; }

  bool& unflown() { return _unflown; }
  const bool unflown() const { return _unflown; }

  bool& hasEmptyDate() { return _hasEmptyDate; }
  const bool& hasEmptyDate() const { return _hasEmptyDate; }

  std::vector<std::vector<TravelSeg*>>& newTravelUsedToSetChangeStatusVec()
  {
    return _newTravelUsedToSetChangeStatus;
  }
  const std::vector<std::vector<TravelSeg*>>& newTravelUsedToSetChangeStatusVec() const
  {
    return _newTravelUsedToSetChangeStatus;
  }

  ChangeStatus& changeStatus();
  const ChangeStatus changeStatus() const;

  std::vector<TravelSeg*>& newTravelUsedToSetChangeStatus();
  const std::vector<TravelSeg*>& newTravelUsedToSetChangeStatus() const;

  bool& isCabinChanged();
  bool isCabinChanged() const;

  bool& isShopped() { return _isShopped; }
  const bool& isShopped() const { return _isShopped; }

  // For FareDisplay
  bool& isRoundTheWorld() { return _isRoundTheWorld; }
  const bool isRoundTheWorld() const { return _isRoundTheWorld; }

  void setBookingCode(const BookingCode& bookingCode, ClassOfService* cos);

  ClassOfService* getBookingCode(const BookingCode& bookingCode);

  bool arunkMultiAirportForAvailability();

  uint16_t& itinIndex() { return _itinIndex; }
  const uint16_t& itinIndex() const { return _itinIndex; }

  uint16_t& originalId() { return _originalId; }
  const uint16_t& originalId() const { return _originalId; }

  int16_t& legId() { return _legId; }
  const int16_t& legId() const { return _legId; }

  int originGmtAdjustment() const { return _originGmtAdjustment; }
  int& originGmtAdjustment() { return _originGmtAdjustment; }
  int destinationGmtAdjustment() const { return _destinationGmtAdjustment; }
  int& destinationGmtAdjustment() { return _destinationGmtAdjustment; }
  bool isNonAirTransportation() const;
  uint16_t& ticketCouponNumber() { return _ticketCouponNumber; }
  uint16_t ticketCouponNumber() const { return _ticketCouponNumber; }
  void setCheckedPortionOfTravelInd(const char ind) { _checkedPortionOfTravelInd = ind; }
  char checkedPortionOfTravelInd() const { return _checkedPortionOfTravelInd; }
  BrandCode getBrandCode() const { return _brandCode; }
  void setBrandCode(const BrandCode& brandCode) { _brandCode = brandCode; }
  bool& rbdReplaced() { return _rbdReplaced; }
  const bool rbdReplaced() const { return _rbdReplaced; }
};

typedef std::vector<tse::TravelSeg*> TravelSegPtrVec;
typedef std::vector<tse::TravelSeg*>::iterator TravelSegPtrVecI;

class CompareTravelSeg
{
public:
  bool operator()(TravelSeg* tvlSeg1, TravelSeg* tvlSeg2)
  {
    return (tvlSeg1->segmentOrder() < tvlSeg2->segmentOrder());
  }

  bool operator()(TravelSeg& tvlSeg1, TravelSeg& tvlSeg2)
  {
    return (tvlSeg1.segmentOrder() < tvlSeg2.segmentOrder());
  }
};

class CompareSegOrderBasedOnItin
{
public:
  CompareSegOrderBasedOnItin(Itin* itin) : _itin(itin) {};

  bool operator()(TravelSeg* tvlSeg1, TravelSeg* tvlSeg2);

private:
  Itin* _itin;
};

class TimeAndUnit
{
public:
  TimeAndUnit() : _value(-1), _unit(' ') {}

  void set(int16_t value, Indicator unit)
  {
    _value = value;
    _unit = unit;
  }

  bool isEmpty() { return ((_value == -1) && (_unit == ' ')); }

  const int16_t& value() const { return _value; }

  const Indicator& unit() const { return _unit; }

private:
  int16_t _value;
  Indicator _unit;
};

class SegmentBrandCodeComparator : public std::unary_function<const TravelSeg*, bool>
{
public:
  SegmentBrandCodeComparator(const TravelSeg* travelSeg) : _travelSeg(travelSeg)
  {
    TSE_ASSERT(_travelSeg != nullptr);
  }
  bool operator()(const TravelSeg* travelSeg) const
  {
    TSE_ASSERT(travelSeg != nullptr);
    if (travelSeg->segmentType() == Arunk)
      return true;
    return _travelSeg->getBrandCode() == travelSeg->getBrandCode();
  }

private:
  const TravelSeg* _travelSeg;
};

TravelSeg::~TravelSeg() = default;
} // tse namespace

