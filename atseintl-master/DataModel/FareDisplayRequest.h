//-------------------------------------------------------------------
//
//  File:         FareDisplayRequest.h
//  Created:
//  Authors:
//
//  Description:
//
//  Updates:
//          02/15/05 - Mike Carroll - file created.
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

#include "Common/DateTime.h"
#include "Common/TseConsts.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/PricingRequest.h"

#include <set>
#include <string>
#include <vector>

namespace tse
{
using FDRequestType = std::string;
extern const FDRequestType FARE_DISPLAY_REQUEST;
extern const FDRequestType FARE_RULES_REQUEST;
extern const FDRequestType FARE_TAX_REQUEST;
extern const FDRequestType FARE_MILEAGE_REQUEST;
extern const FDRequestType FARE_BOOKINGCODE_REQUEST;
extern const FDRequestType TAX_CODE_REQUEST;
extern const FDRequestType PFC_REQUEST;
extern const FDRequestType ENHANCED_RD_REQUEST;

using FDOutputType = std::string;
extern const FDOutputType FD_OUTPUT_SDS;

class FareDisplayRequest : public PricingRequest
{
public:
  FareDisplayRequest() = default;

enum CabinInclusionCode : uint8_t
{ PB_PREM_FIRST = 1,
  FB_FIRST,
  JB_PREM_BUSINESS,
  BB_BUSINESS,
  SB_PREM_ECONOMY,
  YB_ECONOMY,
  AB_ALL };

  CurrencyCode& displayCurrency() { return _displayCurrency; }
  const CurrencyCode& displayCurrency() const { return _displayCurrency; }

  CurrencyCode& alternateDisplayCurrency() { return _alternateDisplayCurrency; }
  const CurrencyCode& alternateDisplayCurrency() const { return _alternateDisplayCurrency; }

  DateTime& returnDate() { return _returnDate; }
  const DateTime& returnDate() const { return _returnDate; }

  DateTime& dateRangeLower() { return _dateRangeLower; }
  const DateTime& dateRangeLower() const { return _dateRangeLower; }

  DateTime& dateRangeUpper() { return _dateRangeUpper; }
  const DateTime& dateRangeUpper() const { return _dateRangeUpper; }

  DateTime& preferredTravelDate() { return _preferredTravelDate; }
  const DateTime& preferredTravelDate() const { return _preferredTravelDate; }

  std::set<PaxTypeCode>& passengerTypes() { return _passengerTypes; }
  const std::set<PaxTypeCode>& passengerTypes() const { return _passengerTypes; }

  BookingCode& bookingCode() { return _bookingCode; }
  const BookingCode& bookingCode() const { return _bookingCode; }

  TktDesignator& ticketDesignator() { return _ticketDesignator; }
  const TktDesignator& ticketDesignator() const { return _ticketDesignator; }

  FareClassCode& fareBasisCode() { return _fareBasisCode; }
  const FareClassCode& fareBasisCode() const { return _fareBasisCode; }

  uint32_t& numberOfFareLevels() { return _numberOfFareLevels; }
  const uint32_t& numberOfFareLevels() const { return _numberOfFareLevels; }

  InclusionCode& inclusionCode() { return _inclusionCode; }
  const InclusionCode& inclusionCode() const { return _inclusionCode; }

  InclusionCode& requestedInclusionCode() { return _requestedInclusionCode; }
  const InclusionCode& requestedInclusionCode() const { return _requestedInclusionCode; }

  FDRequestType& requestType() { return _requestType; }
  const FDRequestType& requestType() const { return _requestType; }

  Indicator& carrierNotEntered() { return _carrierNotEntered; }
  const Indicator& carrierNotEntered() const { return _carrierNotEntered; }

  std::vector<LocCode>& secondaryCity1() { return _secondaryCity1; }
  const std::vector<LocCode>& secondaryCity1() const { return _secondaryCity1; }

  std::vector<LocCode>& secondaryCity2() { return _secondaryCity2; }
  const std::vector<LocCode>& secondaryCity2() const { return _secondaryCity2; }

  std::vector<CarrierCode>& secondaryCarrier() { return _secondaryCarrier; }
  const std::vector<CarrierCode>& secondaryCarrier() const { return _secondaryCarrier; }

  int16_t& addSubLineNumber() { return _addSubLineNumber; }
  const int16_t& addSubLineNumber() const { return _addSubLineNumber; }

  int16_t& addSubPercentage() { return _addSubPercentage; }
  const int16_t& addSubPercentage() const { return _addSubPercentage; }

  std::vector<PaxTypeCode>& displayPassengerTypes() { return _displayPassengerTypes; }
  const std::vector<PaxTypeCode>& displayPassengerTypes() const { return _displayPassengerTypes; }
  std::vector<PaxTypeCode>& inputPassengerTypes() { return _inputPassengerTypes; }
  const std::vector<PaxTypeCode>& inputPassengerTypes() const { return _inputPassengerTypes; }

  std::set<PaxTypeCode>& rec8PassengerTypes() { return _rec8PassengerTypes; }
  const std::set<PaxTypeCode>& rec8PassengerTypes() const { return _rec8PassengerTypes; }

  bool isPaxTypeRequested();

  MoneyAmount& calcFareAmount() { return _calcFareAmount; }
  const MoneyAmount calcFareAmount() const { return _calcFareAmount; }

  FDOutputType& outputType() { return _outputType; }
  const FDOutputType& outputType() const { return _outputType; }

  LocCode& fareOrigin() { return _fareOrigin; }
  const LocCode& fareOrigin() const { return _fareOrigin; }

  LocCode& fareDestination() { return _fareDestination; }
  const LocCode& fareDestination() const { return _fareDestination; }

  bool& displayAccCode() { return _displayAccCode; }
  const bool& displayAccCode() const { return _displayAccCode; }

  bool& multiInclusionCodes() { return _multipleInclusionCodes; }
  const bool& multiInclusionCodes() const { return _multipleInclusionCodes; }

  FareClassCode& uniqueFareBasisCode() { return _uniqueFareBasisCode; }
  const FareClassCode& uniqueFareBasisCode() const { return _uniqueFareBasisCode; }

  typedef std::map<uint8_t, std::pair<std::set<PaxTypeCode>, std::set<PaxTypeCode> > > PaxTypesPerInclCodeMap;

  PaxTypesPerInclCodeMap& paxTypesPerInclCodeMap() { return _paxTypesPerInclCodeMap; }
  const PaxTypesPerInclCodeMap& paxTypesPerInclCodeMap() const { return _paxTypesPerInclCodeMap; }

  uint8_t inclusionNumber(InclusionCode& inclCode) const;
  std::string inclusionVerbiage(uint8_t inclusionNumber);

private:
  FareDisplayRequest(const FareDisplayRequest&) = delete;
  FareDisplayRequest& operator=(const FareDisplayRequest&) = delete;

  CurrencyCode _displayCurrency; // C40
  CurrencyCode _alternateDisplayCurrency; // C41
  DateTime _returnDate = DateTime::emptyDate(); // D02
  DateTime _dateRangeLower = DateTime::emptyDate(); // D05
  DateTime _dateRangeUpper = DateTime::emptyDate(); // D06
  DateTime _preferredTravelDate = DateTime::emptyDate(); // D00
  std::set<PaxTypeCode> _passengerTypes;
  BookingCode _bookingCode; // B30
  TktDesignator _ticketDesignator; // BE0
  FareClassCode _fareBasisCode; // B50
  MoneyAmount _calcFareAmount = 0.0; // C5A
  uint32_t _numberOfFareLevels = 0; // Q17
  InclusionCode _inclusionCode; // BI0 : ModelMap May update this field
  InclusionCode _requestedInclusionCode; // BI0 : Actual Inclusion Code entered by user Model Map
                                         // will never update this field.

  FDRequestType _requestType = FARE_DISPLAY_REQUEST; // S58
  Indicator _carrierNotEntered = 0; // PBB

  std::vector<LocCode> _secondaryCity1; // A10
  std::vector<LocCode> _secondaryCity2; // A11
  std::vector<CarrierCode> _secondaryCarrier; // B03

  int16_t _addSubLineNumber = 0; // Q4N
  int16_t _addSubPercentage = 0; // Q17

  std::vector<PaxTypeCode> _displayPassengerTypes; // B70 for Inclusion code
  std::vector<PaxTypeCode> _inputPassengerTypes; // B70 for Fare rule display
  std::set<PaxTypeCode> _rec8PassengerTypes; // Aggregate

  FDOutputType _outputType; // S82
  LocCode _fareOrigin; // AK0
  LocCode _fareDestination; // AL0

  bool _displayAccCode = false; // ??
  bool _multipleInclusionCodes = false; // FQ with multiple consecutive Cabin inclusion codes

  FareClassCode _uniqueFareBasisCode; // B50 for ERD only
  // price by Cabin
  PaxTypesPerInclCodeMap _paxTypesPerInclCodeMap;
};
} // tse namespace
