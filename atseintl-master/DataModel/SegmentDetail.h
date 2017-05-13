//-------------------------------------------------------------------
//
//  File:        SegmentDetail.h
//  Created:     November 2, 2004
//  Design:      Mike Carroll
//  Authors:
//
//  Description: Segment Detail object.
//
//  Updates:
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
#include "Common/TypeConvert.h"

#include <string>

namespace tse
{

class SurchargeDetail;

class SegmentDetail final
{
public:
  // Accessors
  LocCode& departureCity() { return _departureCity; }
  const LocCode& departureCity() const { return _departureCity; }

  LocCode& departureAirport() { return _departureAirport; }
  const LocCode& departureAirport() const { return _departureAirport; }

  LocCode& arrivalCity() { return _arrivalCity; }
  const LocCode& arrivalCity() const { return _arrivalCity; }

  LocCode& arrivalAirport() { return _arrivalAirport; }
  const LocCode& arrivalAirport() const { return _arrivalAirport; }

  FareClassCode& fareClass() { return _fareClass; }
  const FareClassCode& fareClass() const { return _fareClass; }

  MoneyAmount& cityStopoverCharge() { return _cityStopoverCharge; }
  const MoneyAmount& cityStopoverCharge() const { return _cityStopoverCharge; }

  MoneyAmount& transferCharge() { return _transferCharge; }
  const MoneyAmount& transferCharge() const { return _transferCharge; }

  uint16_t& itinSegmentNumber() { return _itinSegmentNumber; }
  const uint16_t& itinSegmentNumber() const { return _itinSegmentNumber; }

  std::string& routeTravel() { return _routeTravel; }
  const std::string& routeTravel() const { return _routeTravel; }

  DateTime& notValidBeforeDate() { return _notValidBeforeDate; }
  const DateTime& notValidBeforeDate() const { return _notValidBeforeDate; }

  DateTime& notValidAfterDate() { return _notValidAfterDate; }
  const DateTime& notValidAfterDate() const { return _notValidAfterDate; }

  uint16_t& passengerNumber() { return _passengerNumber; }
  const uint16_t& passengerNumber() const { return _passengerNumber; }

  Indicator& baggageIndicator() { return _baggageIndicator; }
  const Indicator& baggageIndicator() const { return _baggageIndicator; }

  MoneyAmount& baggageValue() { return _baggageValue; }
  const MoneyAmount& baggageValue() const { return _baggageValue; }

  Indicator& availabilityBreak() { return _availabilityBreak; }
  const Indicator& availabilityBreak() const { return _availabilityBreak; }

  Indicator& sideTripIndicator() { return _sideTripIndicator; }
  const Indicator& sideTripIndicator() const { return _sideTripIndicator; }
  const bool isSideTripIndicator() const { return TypeConvert::pssCharToBool(_sideTripIndicator); }

  uint16_t& extraMileageAllowance() { return _extraMileageAllowance; }
  const uint16_t& extraMileageAllowance() const { return _extraMileageAllowance; }

  uint16_t& mileageExclusion() { return _mileageExclusion; }
  const uint16_t& mileageExclusion() const { return _mileageExclusion; }

  uint16_t& mileageReduction() { return _mileageReduction; }
  const uint16_t& mileageReduction() const { return _mileageReduction; }

  uint16_t& mileageEqualization() { return _mileageEqualization; }
  const uint16_t& mileageEqualization() const { return _mileageEqualization; }

  Indicator& stopover() { return _stopover; }
  const Indicator& stopover() const { return _stopover; }
  const bool isStopover() const { return TypeConvert::pssCharToBool(_stopover); }

  Indicator& connection() { return _connection; }
  const Indicator& connection() const { return _connection; }
  const bool isConnection() const { return TypeConvert::pssCharToBool(_connection); }

  Indicator& fareBreakPoint() { return _fareBreakPoint; }
  const Indicator& fareBreakPoint() const { return _fareBreakPoint; }

  Indicator& turnAround() { return _turnAround; }
  const Indicator& turnAround() const { return _turnAround; }
  const bool isTurnAround() const { return TypeConvert::pssCharToBool(_turnAround); }

  Indicator& offPointPartMainComponent() { return _offPointPartMainComponent; }
  const Indicator& offPointPartMainComponent() const { return _offPointPartMainComponent; }

  Indicator& sideTripEnd() { return _sideTripEnd; }
  const Indicator& sideTripEnd() const { return _sideTripEnd; }
  const bool isSideTripEnd() const { return TypeConvert::pssCharToBool(_sideTripEnd); }

  Indicator& unchargeableSurface() { return _unchargeableSurface; }
  const Indicator& unchargeableSurface() const { return _unchargeableSurface; }

  Indicator& pureSurface() { return _pureSurface; }
  const Indicator& pureSurface() const { return _pureSurface; }

  std::vector<SurchargeDetail*>& surchargeDetails() { return _surchargeDetails; }
  const std::vector<SurchargeDetail*>& surchargeDetails() const { return _surchargeDetails; }

  Indicator& transfer() { return _transfer; }
  const Indicator& transfer() const { return _transfer; }

  CurrencyCode& transferPubCurrency() { return _transferPubCurrency; }
  const CurrencyCode& transferPubCurrency() const { return _transferPubCurrency; }

  CurrencyCode& stopoverPubCurrency() { return _stopoverPubCurrency; }
  const CurrencyCode& stopoverPubCurrency() const { return _stopoverPubCurrency; }

private:
  // Surcharge detail Information // SEG
  LocCode _departureCity; // SDC
  LocCode _departureAirport; // SDA
  LocCode _arrivalCity; // SAC
  LocCode _arrivalAirport; // SAA
  FareClassCode _fareClass; // SFC
  MoneyAmount _cityStopoverCharge = 0; // SCC
  MoneyAmount _transferCharge = 0; // STC
  uint16_t _itinSegmentNumber = 0; // SNN
  std::string _routeTravel; // SRT
  DateTime _notValidBeforeDate; // SNB
  DateTime _notValidAfterDate; // SNA
  uint16_t _passengerNumber = 0; // SPN
  Indicator _baggageIndicator = ' '; // SBI
  MoneyAmount _baggageValue = 0; // SBV
  Indicator _availabilityBreak = ' '; // SAB
  Indicator _sideTripIndicator = ' '; // SST
  uint16_t _extraMileageAllowance = 0; // SEM
  uint16_t _mileageExclusion = 0; // SME
  uint16_t _mileageReduction = 0; // SMR
  uint16_t _mileageEqualization = 0; // SEQ
  Indicator _stopover = ' '; // SSS
  Indicator _connection = ' '; // SCX
  Indicator _fareBreakPoint = ' '; // FBK
  Indicator _turnAround = ' '; // STA
  Indicator _offPointPartMainComponent = ' '; // PMC
  Indicator _sideTripEnd = ' '; // S07
  Indicator _unchargeableSurface = ' '; // S09
  Indicator _pureSurface = ' '; // S10

  // Surcharge detail records
  std::vector<SurchargeDetail*> _surchargeDetails;

  Indicator _transfer = ' ';
  CurrencyCode _transferPubCurrency;
  CurrencyCode _stopoverPubCurrency;
};
} // tse namespace

