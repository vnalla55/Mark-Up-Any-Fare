//-------------------------------------------------------------------
//
//  File:        FareCalcDetail.h
//  Created:     November 2, 2004
//  Design:      Mike Carroll
//  Authors:
//
//  Description: Fare Calc Detail object.
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
#include "DataModel/DifferentialDetail.h"
#include "DataModel/SegmentDetail.h"
#include "DataModel/SurchargeDetail.h"

#include <string>

namespace tse
{

class FareCalcDetail
{
public:
  FareCalcDetail();

  LocCode& departureCity() { return _departureCity; }
  const LocCode& departureCity() const { return _departureCity; }

  LocCode& departureAirport() { return _departureAirport; }
  const LocCode& departureAirport() const { return _departureAirport; }

  CarrierCode& governingCarrier() { return _governingCarrier; }
  const CarrierCode& governingCarrier() const { return _governingCarrier; }

  CarrierCode& secondaryGoverningCarrier() { return _secondaryGoverningCarrier; }
  const CarrierCode& secondaryGoverningCarrier() const { return _secondaryGoverningCarrier; }

  CarrierCode& trueGoverningCarrier() { return _trueGoverningCarrier; }
  const CarrierCode& trueGoverningCarrier() const { return _trueGoverningCarrier; }

  CurrencyCode& baseCurrencyCode() { return _baseCurrencyCode; }
  const CurrencyCode& baseCurrencyCode() const { return _baseCurrencyCode; }

  LocCode& arrivalCity() { return _arrivalCity; }
  const LocCode& arrivalCity() const { return _arrivalCity; }

  LocCode& arrivalAirport() { return _arrivalAirport; }
  const LocCode& arrivalAirport() const { return _arrivalAirport; }

  MoneyAmount& fareAmount() { return _fareAmount; }
  const MoneyAmount& fareAmount() const { return _fareAmount; }

  FareClassCode& fareBasisCode() { return _fareBasisCode; }
  const FareClassCode& fareBasisCode() const { return _fareBasisCode; }

  uint16_t& fareBasisCodeLength() { return _fareBasisCodeLength; }
  const uint16_t& fareBasisCodeLength() const { return _fareBasisCodeLength; }

  PaxTypeCode& requestedPassengerType() { return _requestedPassengerType; }
  const PaxTypeCode& requestedPassengerType() const { return _requestedPassengerType; }

  FareClassCode& differentialFareBasisCode() { return _differentialFareBasisCode; }
  const FareClassCode& differentialFareBasisCode() const { return _differentialFareBasisCode; }

  std::string& ticketDesignator() { return _ticketDesignator; }
  const std::string& ticketDesignator() const { return _ticketDesignator; }

  MoneyAmount& snapDesignatorDiscount() { return _snapDesignatorDiscount; }
  const MoneyAmount& snapDesignatorDiscount() const { return _snapDesignatorDiscount; }

  std::string& cabinCode() { return _cabinCode; }
  const std::string& cabinCode() const { return _cabinCode; }

  NationCode& departureCountry() { return _departureCountry; }
  const NationCode& departureCountry() const { return _departureCountry; }

  IATAAreaCode& departureIATA() { return _departureIATA; }
  const IATAAreaCode& departureIATA() const { return _departureIATA; }

  StateCode& departureStateOrProvince() { return _departureStateOrProvince; }
  const StateCode& departureStateOrProvince() const { return _departureStateOrProvince; }

  NationCode& arrivalCountry() { return _arrivalCountry; }
  const NationCode& arrivalCountry() const { return _arrivalCountry; }

  IATAAreaCode& arrivalIATA() { return _arrivalIATA; }
  const IATAAreaCode& arrivalIATA() const { return _arrivalIATA; }

  StateCode& arrivalStateOrProvince() { return _arrivalStateOrProvince; }
  const StateCode& arrivalStateOrProvince() const { return _arrivalStateOrProvince; }

  MoneyAmount& publishedFareAmount() { return _publishedFareAmount; }
  const MoneyAmount& publishedFareAmount() const { return _publishedFareAmount; }

  CurrencyCode& fareComponentCurrencyCode() { return _fareComponentCurrencyCode; }
  const CurrencyCode& fareComponentCurrencyCode() const { return _fareComponentCurrencyCode; }

  Indicator& roundTripFare() { return _roundTripFare; }
  const Indicator& roundTripFare() const { return _roundTripFare; }
  const bool isRoundTripFare() const { return TypeConvert::pssCharToBool(_roundTripFare); }

  Indicator& oneWayFare() { return _oneWayFare; }
  const Indicator& oneWayFare() const { return _oneWayFare; }
  const bool isOneWayFare() const { return TypeConvert::pssCharToBool(_oneWayFare); }

  Indicator& oneWayDirectionalFare() { return _oneWayDirectionalFare; }

  Indicator& iataAuthorizedCarrier() { return _iataAuthorizedCarrier; }

  Indicator& verifyGeographicRestrictions() { return _verifyGeographicRestrictions; }

  Indicator& failCat15() { return _failCat15; }

  Indicator& ticketRestricted() { return _ticketRestricted; }

  Indicator& fareByRuleSpouseHead() { return _fareByRuleSpouseHead; }

  Indicator& fareByRuleSpouseAccompany() { return _fareByRuleSpouseAccompany; }

  Indicator& fareByRuleSeamanAdult() { return _fareByRuleSeamanAdult; }

  Indicator& fareByRuleSeamanChild() { return _fareByRuleSeamanChild; }

  Indicator& fareByRuleSeamanInfant() { return _fareByRuleSeamanInfant; }

  Indicator& fareByRuleBTS() { return _fareByRuleBTS; }

  Indicator& fareByRuleNegotiated() { return _fareByRuleNegotiated; }

  Indicator& fareByRuleNegotiatedChild() { return _fareByRuleNegotiatedChild; }

  Indicator& fareByRuleNegotiatedInfant() { return _fareByRuleNegotiatedInfant; }

  DateTime& commencementDate() { return _commencementDate; }
  const DateTime& commencementDate() const { return _commencementDate; }

  std::string& typeOfFare() { return _typeOfFare; }
  const std::string& typeOfFare() const { return _typeOfFare; }

  std::vector<SegmentDetail*>& segmentDetails() { return _segmentDetails; }
  const std::vector<SegmentDetail*>& segmentDetails() const { return _segmentDetails; }

  std::vector<DifferentialDetail*>& differentialDetails() { return _differentialDetails; }
  const std::vector<DifferentialDetail*>& differentialDetails() const
  {
    return _differentialDetails;
  }

  std::string& discountCode() { return _discountCode; }
  const std::string& discountCode() const { return _discountCode; }

  uint16_t& discountPercentage() { return _discountPercentage; }
  const uint16_t& discountPercentage() const { return _discountPercentage; }

  bool& isRouting() { return _isRouting; }
  const bool& isRouting() const { return _isRouting; }

  uint16_t& mileageSurchargePctg() { return _mileageSurchargePctg; }
  const uint16_t& mileageSurchargePctg() const { return _mileageSurchargePctg; }

  std::string& hipOrigCity() { return _hipOrigCity; }
  const std::string& hipOrigCity() const { return _hipOrigCity; }

  std::string& hipDestCity() { return _hipDestCity; }
  const std::string& hipDestCity() const { return _hipDestCity; }

  std::string& constructedHipCity() { return _constructedHipCity; }
  const std::string& constructedHipCity() const { return _constructedHipCity; }

  Indicator& normalOrSpecialFare() { return _normalOrSpecialFare; }
  const Indicator& normalOrSpecialFare() const { return _normalOrSpecialFare; }

  uint16_t& pricingUnitCount() { return _pricingUnitCount; }
  const uint16_t& pricingUnitCount() const { return _pricingUnitCount; }

  std::string& pricingUnitType() { return _pricingUnitType; }
  const std::string& pricingUnitType() const { return _pricingUnitType; }

  std::string& globalIndicator() { return _globalIndicator; }
  const std::string& globalIndicator() const { return _globalIndicator; }

  std::string& directionality() { return _directionality; }
  const std::string& directionality() const { return _directionality; }

  Indicator& sideTripIncluded() { return _sideTripIncluded; }
  const bool isSideTripIncluded() const { return TypeConvert::pssCharToBool(_sideTripIncluded); }

  uint16_t& segmentsCount() { return _segmentsCount; }
  const uint16_t& segmentsCount() const { return _segmentsCount; }

private:
  // FareCalc detail Information 					// CAL
  LocCode _departureCity; // CBC
  LocCode _departureAirport; // CBA
  LocCode _arrivalCity; // CAC
  LocCode _arrivalAirport; // CAA
  CarrierCode _governingCarrier; // CGP
  CarrierCode _secondaryGoverningCarrier; // CGS
  CarrierCode _trueGoverningCarrier;
  CurrencyCode _baseCurrencyCode; // BCC
  CurrencyCode _fareComponentCurrencyCode; // COC
  PaxTypeCode _requestedPassengerType; // CFO
  MoneyAmount _fareAmount; // CFA
  MoneyAmount _snapDesignatorDiscount; // CSD
  MoneyAmount _publishedFareAmount; // PFA
  DateTime _commencementDate; // CAD

  std::string _ticketDesignator; // CTD
  std::string _cabinCode; // CCB
  std::string _typeOfFare; // NOK
  std::string _discountCode; // DCO
  std::string _hipOrigCity; // HOC
  std::string _hipDestCity; // HDC
  std::string _constructedHipCity; // CHC
  std::string _pricingUnitType; // PUT
  std::string _globalIndicator; // GLB
  std::string _directionality; // CDR

  // Segment detail records
  std::vector<SegmentDetail*> _segmentDetails;
  std::vector<DifferentialDetail*> _differentialDetails;

  FareClassCode _fareBasisCode; // CFB
  FareClassCode _differentialFareBasisCode; // FBC

  NationCode _departureCountry; // DCC
  NationCode _arrivalCountry; // ACC
  IATAAreaCode _departureIATA; // DIA
  IATAAreaCode _arrivalIATA; // AIA
  StateCode _departureStateOrProvince; // DSC
  StateCode _arrivalStateOrProvince; // DSC

  Indicator _roundTripFare; // CRT
  Indicator _oneWayFare; // COW
  Indicator _oneWayDirectionalFare; // COD
  Indicator _iataAuthorizedCarrier; // CIU

  uint16_t _discountPercentage; // DPR
  uint16_t _mileageSurchargePctg; // MSP
  uint16_t _pricingUnitCount; // QPC
  uint16_t _segmentsCount; // Q0U - side trip related

  uint16_t _fareBasisCodeLength; // FBL

  Indicator _verifyGeographicRestrictions; // VGR
  Indicator _failCat15; // C15
  Indicator _ticketRestricted; // CTR
  Indicator _fareByRuleSpouseHead; // FSH
  Indicator _fareByRuleSpouseAccompany; // FSA
  Indicator _fareByRuleSeamanAdult; // FSM
  Indicator _fareByRuleSeamanChild; // FSC
  Indicator _fareByRuleSeamanInfant; // FSI
  Indicator _fareByRuleBTS; // BTS
  Indicator _fareByRuleNegotiated; // FRN
  Indicator _fareByRuleNegotiatedChild; // FRC
  Indicator _fareByRuleNegotiatedInfant; // FRI
  Indicator _normalOrSpecialFare; // NOS
  Indicator _sideTripIncluded; // P2N

  bool _isRouting; // RTG
};

inline FareCalcDetail::FareCalcDetail()
  : _fareAmount(0),
    _snapDesignatorDiscount(0),
    _publishedFareAmount(0),
    _roundTripFare(' '),
    _oneWayFare(' '),
    _oneWayDirectionalFare(' '),
    _iataAuthorizedCarrier(' '),
    _discountPercentage(0),
    _mileageSurchargePctg(0),
    _pricingUnitCount(0),
    _segmentsCount(0),
    _fareBasisCodeLength(0),
    _verifyGeographicRestrictions(' '),
    _failCat15(' '),
    _ticketRestricted(' '),
    _fareByRuleSpouseHead(' '),
    _fareByRuleSpouseAccompany(' '),
    _fareByRuleSeamanAdult(' '),
    _fareByRuleSeamanChild(' '),
    _fareByRuleSeamanInfant(' '),
    _fareByRuleBTS(' '),
    _fareByRuleNegotiated(' '),
    _fareByRuleNegotiatedChild(' '),
    _fareByRuleNegotiatedInfant(' '),
    _normalOrSpecialFare(' '),
    _sideTripIncluded(' '),
    _isRouting(false)
{
}
} // tse namespace
