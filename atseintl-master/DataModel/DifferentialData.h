//-------------------------------------------------------------------
//
//  File:        DifferentilaData.h
//  Created:     May 27, 2004
//  Authors:     Alexander Zagrebin
//
//  Description:
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

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

#include <vector>

namespace tse
{
class Loc;
class TravelSeg;
class FareMarket;
class PaxTypeFare;
class DataHandle;

class DifferentialData final
{
public:
  enum STATUS_TYPE : uint8_t
  {
    SC_NOT_PROCESSED_YET,
    SC_PASSED,
    SC_FAILED,
    SC_MATCH_1A,
    SC_MATCH_1B,
    SC_CONSOLIDATED_PASS,
    SC_CONSOLIDATED_FAIL,
    SC_ADJACENT_FAIL,
    SC_COMBINATION_FAIL
  };

  enum FareTypeDesignators : uint8_t
  {
    PREMIUM_FIRST_FTD = 'R',
    FIRST_FTD = 'F',
    PREMIUM_BUSINESS_FTD = 'X',
    PREMIUM_BUSINESS_FTD_NEW = 'J',
    BUSINESS_FTD = 'B',
    PREMIUM_ECONOMY_FTD = 'W',
    ECONOMY_FTD = 'E',
    BLANK_FTD = ' '
  };
  // PREMIUM_FIRST_HIP = 'P' && PREMIUM_ECONOMY_HIP = 'S' should be removed along with
  // TrxUtil::isAtpcoRbdByCabinAnswerTableActivated() removal

  enum HipRelated : uint8_t
  {
    PREMIUM_FIRST_HIP_ANSWER = 'R',
    PREMIUM_FIRST_HIP = 'P',
    FIRST_HIP = 'F',
    PREMIUM_BUSINESS_HIP = 'J',
    BUSINESS_HIP = 'C',
    PREMIUM_ECONOMY_HIP_ANSWER = 'W',
    PREMIUM_ECONOMY_HIP = 'S',
    ECONOMY_HIP = 'Y',
    NO_HIP = 'N'
  };

  const Loc*& origin() { return _origin; }
  const Loc* origin() const { return _origin; }

  const Loc*& destination() { return _destination; }
  const Loc* destination() const { return _destination; }

  FareTypeDesignators& cabin() { return _cabin; }
  const FareTypeDesignators& cabin() const { return _cabin; }

  MoneyAmount& amount() { return _amount; }
  const MoneyAmount& amount() const { return _amount; }

  DifferentialTag& tag() { return _diffTag; }
  const DifferentialTag& tag() const { return _diffTag; }

  FareClassCode& fareClassHigh() { return _fareClassHigh; }
  const FareClassCode& fareClassHigh() const { return _fareClassHigh; }

  MoneyAmount& amountFareClassHigh() { return _amountFareClassHigh; }
  const MoneyAmount& amountFareClassHigh() const { return _amountFareClassHigh; }

  FareClassCode& fareClassLow() { return _fareClassLow; }
  const FareClassCode& fareClassLow() const { return _fareClassLow; }

  MoneyAmount& amountFareClassLow() { return _amountFareClassLow; }
  const MoneyAmount& amountFareClassLow() const { return _amountFareClassLow; }

  Indicator& mileage() { return _mileage; }
  const Indicator mileage() const { return _mileage; }

  uint32_t& maxPermittedMileage() { return _maxPermittedMileage; }
  const uint32_t maxPermittedMileage() const { return _maxPermittedMileage; }

  MoneyAmount& hipAmtFareClassHigh() { return _hipAmtFareClassHigh; }
  const MoneyAmount& hipAmtFareClassHigh() const { return _hipAmtFareClassHigh; }

  MoneyAmount& hipAmtFareClassLow() { return _hipAmtFareClassLow; }
  const MoneyAmount& hipAmtFareClassLow() const { return _hipAmtFareClassLow; }

  HipRelated& hipCabinHigh() { return _hipCabinHigh; }
  const HipRelated hipCabinHigh() const { return _hipCabinHigh; }

  LocCode& hipHighOrigin() { return _hipHighOrigin; }
  const LocCode& hipHighOrigin() const { return _hipHighOrigin; }

  LocCode& hipHighDestination() { return _hipHighDestination; }
  const LocCode& hipHighDestination() const { return _hipHighDestination; }

  HipRelated& hipCabinLow() { return _hipCabinLow; }
  const HipRelated hipCabinLow() const { return _hipCabinLow; }

  LocCode& hipLowOrigin() { return _hipLowOrigin; }
  const LocCode& hipLowOrigin() const { return _hipLowOrigin; }

  LocCode& hipLowDestination() { return _hipLowDestination; }
  const LocCode& hipLowDestination() const { return _hipLowDestination; }

  LocCode& hipConstructedCity() { return _hipConstructedCity; }
  const LocCode& hipConstructedCity() const { return _hipConstructedCity; }

  MoneyAmount& hipAmount() { return _hipAmount; }
  const MoneyAmount& hipAmount() const { return _hipAmount; }

  STATUS_TYPE& status() { return _status; }
  const STATUS_TYPE status() const { return _status; }

  Indicator& tripType() { return _tripType; }
  const Indicator tripType() const { return _tripType; }

  Indicator& calculationIndicator() { return _calculationIndicator; }
  const Indicator calculationIndicator() const { return _calculationIndicator; }

  uint16_t& differSeqNumber() { return _differSeqNumber; }
  const uint16_t& differSeqNumber() const { return _differSeqNumber; }

  std::vector<CarrierCode>& carrier() { return _carrier; }
  const std::vector<CarrierCode>& carrier() const { return _carrier; }

  std::vector<CarrierCode>& carrierCurrent() { return _carrierCurrent; }
  const std::vector<CarrierCode>& carrierCurrent() const { return _carrierCurrent; }

  BookingCode& bookingCode() { return _bookingCode; }
  const BookingCode& bookingCode() const { return _bookingCode; }

  CarrierCode& carrierDiff() { return _carrierDiff; }
  const CarrierCode& carrierDiff() const { return _carrierDiff; }

  bool& stops() { return _stops; }
  const bool stops() const { return _stops; }

  bool& setSameCarrier() { return _sameCarrier; }
  const bool isSameCarrier() const { return _sameCarrier; }

  bool& setRebookedBkg() { return _rebooked; }
  const bool isRebookedBkg() const { return _rebooked; }

  bool& setSlideAllow() { return _slideAllow; }
  const bool isSlideAllow() const { return _slideAllow; }

  bool& setFailCat23() { return _failCat23; }
  const bool isFailCat23() const { return _failCat23; }

  std::vector<FareMarket*>& fareMarket(void) { return _fareMarket; }
  const std::vector<FareMarket*>& fareMarket(void) const { return _fareMarket; }

  std::vector<FareMarket*>& fareMarketCurrent(void) { return _fareMarketCurrent; }
  const std::vector<FareMarket*>& fareMarketCurrent(void) const { return _fareMarketCurrent; }

  bool finalizeCarrierAndFareMarketLists(const CarrierCode& cr);

  PaxTypeFare*& fareLow(void) { return _fareLow; }
  const PaxTypeFare* fareLow(void) const { return _fareLow; }

  PaxTypeFare*& fareHigh(void) { return _fareHigh; }
  const PaxTypeFare* fareHigh(void) const { return _fareHigh; }

  std::vector<TravelSeg*>& travelSeg() { return _travelSeg; }
  const std::vector<TravelSeg*>& travelSeg() const { return _travelSeg; }

  PaxTypeFare*& throughFare(void) { return _throughFare; }
  const PaxTypeFare* throughFare(void) const { return _throughFare; }

  bool& setDiffFMFalse() { return _isDiffFMFalse; }
  const bool& isDiffFMFalse() const { return _isDiffFMFalse; }

  const bool& isSetSwap() const { return _setSwap; }

  void swap();

  std::vector<DifferentialData*>& inConsolDiffData() { return _diffConsec; }
  const std::vector<DifferentialData*>& inConsolDiffData() const { return _diffConsec; }

  bool& setConsolidate() { return _consolidate; }
  const bool& isItemConsolidated() const { return _consolidate; }

  uint16_t& thruNumber() { return _thruNumber; }
  const uint16_t thruNumber() const { return _thruNumber; }

  std::vector<CarrierCode>& cleanUpVectors() { return _carrierToRemove; }
  const std::vector<CarrierCode>& cleanUpVectors() const { return _carrierToRemove; }

  FareType& intermFareType() { return _intermFareType; }
  const FareType& intermFareType() const { return _intermFareType; }

  Indicator& calcIntermIndicator() { return _calcIntermIndicator; }
  const Indicator calcIntermIndicator() const { return _calcIntermIndicator; }

  DifferentialData* clone(DataHandle& dataHandle) const;

private:
  const Loc* _origin = nullptr;
  const Loc* _destination = nullptr;
  DifferentialTag _diffTag;
  FareClassCode _fareClassHigh;
  FareClassCode _fareClassLow;
  MoneyAmount _amountFareClassHigh = 0;
  MoneyAmount _amountFareClassLow = 0;
  MoneyAmount _hipAmtFareClassHigh = 0;
  MoneyAmount _hipAmtFareClassLow = 0;
  MoneyAmount _amount = 0;
  MoneyAmount _hipAmount = 0;
  LocCode _hipHighOrigin;
  LocCode _hipHighDestination;
  LocCode _hipLowOrigin;
  LocCode _hipLowDestination;
  LocCode _hipConstructedCity;
  uint32_t _maxPermittedMileage = 0;
  std::vector<CarrierCode> _carrier;
  std::vector<CarrierCode> _carrierCurrent; // cxr's current
  std::vector<CarrierCode> _carrierToRemove; // cxr's need to be removed
  std::vector<FareMarket*> _fareMarket;
  std::vector<FareMarket*> _fareMarketCurrent; // fm's current
  std::vector<TravelSeg*> _travelSeg;
  std::vector<DifferentialData*> _diffConsec;
  PaxTypeFare* _fareLow = nullptr;
  PaxTypeFare* _fareHigh = nullptr;
  PaxTypeFare* _throughFare = nullptr;
  BookingCode _bookingCode;
  CarrierCode _carrierDiff;
  uint16_t _differSeqNumber = 0;
  uint16_t _thruNumber = 0;
  bool _stops = false;
  bool _sameCarrier = false;
  bool _isDiffFMFalse = false; // true if differentials origin/dest = FM origin/dest
  bool _setSwap = false; // swap origin/destination for Inbound FareMarket
  bool _consolidate = false; // true - consolidate diff seg, false - regular
  bool _rebooked = false; // false - Bkg is not rebooked
  bool _slideAllow = true; // true - Slide Allow
  bool _failCat23 = false;
  HipRelated _hipCabinHigh = HipRelated::NO_HIP;
  HipRelated _hipCabinLow = HipRelated::NO_HIP;
  FareTypeDesignators _cabin = FareTypeDesignators::BLANK_FTD;
  STATUS_TYPE _status = STATUS_TYPE::SC_NOT_PROCESSED_YET;
  Indicator _mileage = 0;
  Indicator _tripType = 0;
  Indicator _calculationIndicator = ' ';
  FareType  _intermFareType;
  Indicator _calcIntermIndicator = ' ';
};

using DifferentialDataPtrVec = std::vector<DifferentialData*>;
using DifferentialDataPtrVecI = std::vector<DifferentialData*>::iterator;
using DifferentialDataPtrVecIC = std::vector<DifferentialData*>::const_iterator;

} // tse
