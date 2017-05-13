//-------------------------------------------------------------------
//
//  File:           ERDFareComp.h
//  Created:        16 September 2008
//  Authors:        Konrad Koch
//
//  Description:    Wrapper for pricing CAL section for ERD request.
//
//  Copyright Sabre 2008
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
#include "DataModel/AddOnAttributes.h"
#include "DataModel/ConstructedAttributes.h"
#include "DataModel/ERDFltSeg.h"
#include "DataModel/NonPublishedValues.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"

#include <string>

namespace tse
{
class FareDisplayTrx;
class AirSeg;
class FareDisplayRequest;
class FareDisplayOptions;

struct HPUSection
{
  std::string markupType;
  PseudoCityCode sourcePCC;
  MoneyAmount totalFareAmount;
};

typedef std::vector<ERDFltSeg*> ERDFltSegList;
typedef std::vector<HPUSection> HPUInfoList;

class ERDFareComp
{
public:
  ERDFareComp();

  class MatchFareBasis : public std::unary_function<const ERDFareComp*, bool>
  {
  public:
    MatchFareBasis(const FareClassCode& fareBasis) { _fareBasis = fareBasis; }

    bool operator()(const ERDFareComp* erdFareComp) const;

  private:
    FareClassCode _fareBasis;
  };

  class MatchConditionalFareBasis : public std::unary_function<const ERDFareComp*, bool>
  {
  public:
    MatchConditionalFareBasis(const FareClassCode& fareBasis) { _fareBasis = fareBasis; }

    bool operator()(const ERDFareComp* erdFareComp) const;

  private:
    FareClassCode _fareBasis;
  };

  class MatchSegmentNumbers : public std::unary_function<const ERDFareComp*, bool>
  {
  public:
    MatchSegmentNumbers(const std::vector<uint16_t>& segmentNumbers)
      : _segmentNumbers(segmentNumbers)
    {
    }

    MatchSegmentNumbers(uint16_t segmentNumber) { _segmentNumbers.push_back(segmentNumber); }

    bool operator()(const ERDFareComp* erdFareComp) const;

  private:
    std::vector<uint16_t> _segmentNumbers;
  };

  class MatchPaxTypeCode : public std::unary_function<const ERDFareComp*, bool>
  {
  public:
    MatchPaxTypeCode(const PaxTypeCode& paxType) { _paxType = paxType; }

    bool operator()(const ERDFareComp* erdFareComp) const;

  private:
    PaxTypeCode _paxType;
  };

  class MatchPaxTypeNumber : public std::unary_function<const ERDFareComp*, bool>
  {
  public:
    MatchPaxTypeNumber(uint16_t paxTypeNumber) { _paxTypeNumber = paxTypeNumber; }

    bool operator()(const ERDFareComp* erdFareComp) const
    {
      return _paxTypeNumber == erdFareComp->paxTypeNumber();
    }

  private:
    uint16_t _paxTypeNumber;
  };

  ERDFltSegList& segments() { return _segments; }
  const ERDFltSegList& segments() const { return _segments; }

  HPUInfoList& hpuInfo() { return _hpuInfo; }
  const HPUInfoList& hpuInfo() const { return _hpuInfo; }

  PaxTypeCode& paxTypeCode() { return _paxTypeCode; }
  const PaxTypeCode& paxTypeCode() const { return _paxTypeCode; }

  uint16_t& paxTypeNumber() { return _paxTypeNumber; }
  const uint16_t paxTypeNumber() const { return _paxTypeNumber; }

  PaxTypeCode& actualPaxTypeCode() { return _actualPaxTypeCode; }
  const PaxTypeCode& actualPaxTypeCode() const { return _actualPaxTypeCode; }

  PaxTypeCode& requestedPaxTypeCode() { return _requestedPaxTypeCode; }
  const PaxTypeCode& requestedPaxTypeCode() const { return _requestedPaxTypeCode; }

  CarrierCode& trueGoverningCarrier() { return _trueGoverningCarrier; }
  const CarrierCode& trueGoverningCarrier() const { return _trueGoverningCarrier; }

  MoneyAmount& fareAmount() { return _fareAmount; }
  const MoneyAmount& fareAmount() const { return _fareAmount; }

  CurrencyCode& fareCurrency() { return _fareCurrency; }
  const CurrencyCode& fareCurrency() const { return _fareCurrency; }

  MoneyAmount& nucFareAmount() { return _nucFareAmount; }
  const MoneyAmount& nucFareAmount() const { return _nucFareAmount; }

  FareClassCode& fareBasis() { return _fareBasis; }
  const FareClassCode& fareBasis() const { return _fareBasis; }

  uint16_t& pricingUnitNumber() { return _pricingUnitNumber; }
  const uint16_t pricingUnitNumber() const { return _pricingUnitNumber; }

  LocCode& departureCity() { return _departureCity; }
  const LocCode& departureCity() const { return _departureCity; }

  LocCode& arrivalCity() { return _arrivalCity; }
  const LocCode& arrivalCity() const { return _arrivalCity; }

  LocCode& departureAirport() { return _departureAirport; }
  const LocCode& departureAirport() const { return _departureAirport; }

  LocCode& arrivalAirport() { return _arrivalAirport; }
  const LocCode& arrivalAirport() const { return _arrivalAirport; }

  char& oneWayFare() { return _oneWayFare; }
  const char& oneWayFare() const { return _oneWayFare; }

  char& roundTripFare() { return _roundTripFare; }
  const char& roundTripFare() const { return _roundTripFare; }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  CarrierCode& governingCarrier() { return _governingCarrier; }
  const CarrierCode& governingCarrier() const { return _governingCarrier; }

  TariffNumber& fareTariff() { return _fareTariff; }
  const TariffNumber& fareTariff() const { return _fareTariff; }

  RuleNumber& ruleNumber() { return _ruleNumber; }
  const RuleNumber& ruleNumber() const { return _ruleNumber; }

  char& fareBreak() { return _fareBreak; }
  const char& fareBreak() const { return _fareBreak; }

  DateTime& departureDT() { return _departureDT; }
  const DateTime& departureDT() const { return _departureDT; }

  std::string& directionality() { return _directionality; }
  const std::string& directionality() const { return _directionality; }

  AccountCode& accountCode() { return _accountCode; }
  const AccountCode& accountCode() const { return _accountCode; }

  GlobalDirection& globalIndicator() { return _globalIndicator; }
  const GlobalDirection& globalIndicator() const { return _globalIndicator; }

  LinkNumber& linkNumber() { return _linkNumber; }
  const LinkNumber& linkNumber() const { return _linkNumber; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  std::string& createTime() { return _createTime; }
  const std::string& createTime() const { return _createTime; }

  SequenceNumber& sequenceNumber() { return _sequenceNumber; }
  const SequenceNumber& sequenceNumber() const { return _sequenceNumber; }

  FareType& fareType() { return _fareType; }
  const FareType& fareType() const { return _fareType; }

  FareClassCode& fareClass() { return _fareClass; }
  const FareClassCode& fareClass() const { return _fareClass; }

  std::string& cat35Type() { return _cat35Type; }
  const std::string& cat35Type() const { return _cat35Type; }

  NonPublishedValues& cat25Values() { return _cat25Values; }
  const NonPublishedValues& cat25Values() const { return _cat25Values; }

  NonPublishedValues& cat35Values() { return _cat35Values; }
  const NonPublishedValues& cat35Values() const { return _cat35Values; }

  NonPublishedValues& discountedValues() { return _discountedValues; }
  const NonPublishedValues& discountedValues() const { return _discountedValues; }

  BookingCode& bookingCode() { return _bookingCode; }
  const BookingCode& bookingCode() const { return _bookingCode; }

  std::string& ticketDesignator() { return _ticketDesignator; }
  const std::string& ticketDesignator() const { return _ticketDesignator; }

  ConstructedAttributes& constructedAttributes() { return _constructedAttributes; }
  const ConstructedAttributes& constructedAttributes() const { return _constructedAttributes; }

  AddOnAttributes& origAttributes() { return _origAddonAttributes; }
  const AddOnAttributes& origAttributes() const { return _origAddonAttributes; }

  AddOnAttributes& destAttributes() { return _destAddonAttributes; }
  const AddOnAttributes& destAttributes() const { return _destAddonAttributes; }

  CarrierCode& validatingCarrier() { return _validatingCarrier; }
  const CarrierCode& validatingCarrier() const { return _validatingCarrier; }

  uint16_t& fareClassLength() { return _fareClassLength; }
  const uint16_t& fareClassLength() const { return _fareClassLength; }

  bool& wasCommandPricing() { return _wasCommandPricing; }
  const bool& wasCommandPricing() const { return _wasCommandPricing; }

  FareClassCode& uniqueFareBasis() { return _uniqueFareBasis; }
  const FareClassCode& uniqueFareBasis() const { return _uniqueFareBasis; }

  FareClassCode& cat35FareBasis() { return _cat35FareBasis; }
  const FareClassCode& cat35FareBasis() const { return _cat35FareBasis; }

  //--------------------------------------------------------------------------
  // @function ERDFareComp::select
  //
  // Description: Method to transform collected ERD data into FD trx object
  //
  // @param trx - FD transaction object to write data into
  //--------------------------------------------------------------------------
  void select(FareDisplayTrx& trx);

  //--------------------------------------------------------------------------
  // @function ERDFareComp::completeFareBreak
  //
  // Description: Checks if second segment contains for the same fare
  //      and if yes copies vendor, tariff & rule from second segment
  //
  // @param second - possible data source
  //
  // @returns true if data was copied, otherwise false
  //--------------------------------------------------------------------------
  bool completeFareBreak(const ERDFareComp& second);

private:
  ERDFareComp(const ERDFareComp&);
  ERDFareComp& operator=(const ERDFareComp&);

  //--------------------------------------------------------------------------
  // @function ERDFareComp::setMultiAirportCities
  //
  // Description: Sets multi airport cities to given air segment
  //
  // @param airSeg - pointer to air segment
  //--------------------------------------------------------------------------
  void setMultiAirportCities(FareDisplayTrx& trx,
                             AirSeg* airSeg,
                             const LocCode& origAirport,
                             const LocCode& destAirport);

  //--------------------------------------------------------------------------
  // @function ERDFareComp::setTravelDates
  //
  // Description: Sets travel dates to given air segment
  //
  // @param airSeg - pointer to air segment
  //--------------------------------------------------------------------------
  void setTravelDates(AirSeg* airSeg);

  //--------------------------------------------------------------------------
  // @function ERDFareComp::setAddonAttributes
  //
  // Description: Sets addon attributes
  //
  // @param destAttrs - destination attributes
  // @param srcAttrs  - source attributes
  //--------------------------------------------------------------------------
  void setAddonAttributes(AddOnAttributes& destAttrs, const AddOnAttributes& srcAttrs) const;

  //--------------------------------------------------------------------------
  // @function ERDFareComp::storePassengers
  //
  // Description: Stores passengers into FareDisplayRequest
  //
  // @param request - request, where passengers are stored.
  //--------------------------------------------------------------------------
  void storePassengers(FareDisplayRequest& request) const;

  //--------------------------------------------------------------------------
  // @function ERDFareComp::storeNonPublishedValues
  //
  // Description: Stores non published values into FareDisplayOptions
  //
  // @param source - source of date.
  // @param destination - destination of data.
  // @param options - options, where values are stored.
  //--------------------------------------------------------------------------
  void storeNonPublishedValues(const NonPublishedValues& source,
                               NonPublishedValues& destination,
                               FareDisplayOptions& options) const;
  //--------------------------------------------------------------------------
  // @function ERDFareComp::storeConstructedAttributes
  //
  // Description: Stores store constructed attributes into FareDisplayOptions
  //
  // @param options - options, where constructed attributes are stored.
  //--------------------------------------------------------------------------
  void storeConstructedAttributes(FareDisplayOptions& options) const;

  void updateFRRInfo(FareDisplayTrx& trx, FareDisplayOptions& options, FareDisplayRequest& request);

  ERDFltSegList _segments;
  HPUInfoList _hpuInfo;
  PaxTypeCode _paxTypeCode; // PXI:B70
  PaxTypeCode _actualPaxTypeCode; // ERD:B70
  PaxTypeCode _requestedPaxTypeCode; // PXI:B71
  CarrierCode _trueGoverningCarrier; // CAL:B08
  LocCode _departureAirport; // CAL:A01
  LocCode _arrivalAirport; // CAL:A02
  LocCode _departureCity; // CAL:A11
  LocCode _arrivalCity; // CAL:A12
  MoneyAmount _fareAmount; // CAL:C5A
  MoneyAmount _nucFareAmount; // CAL:C50
  CurrencyCode _fareCurrency; // CAL:C40
  CarrierCode _governingCarrier; // SEG:B02
  FareClassCode _fareBasis; // CAL:B50
  FareClassCode _uniqueFareBasis; // CAL:B50 if ERD:B50
  FareClassCode _cat35FareBasis; // CAL:B50 if ERD:B50 & ERD:N1P
  FareClassCode _fareClass; // ERD:BJ0
  uint16_t _paxTypeNumber; // PXI:Q4P
  uint16_t _pricingUnitNumber; // CAL:Q4J
  uint16_t _fareClassLength; // ERD:Q04
  char _oneWayFare; // CAL:P04
  char _roundTripFare; // CAL:P05
  VendorCode _vendor; // SEG:S37
  LinkNumber _linkNumber; // ERD:Q46
  TariffNumber _fareTariff; // SEG:S89
  RuleNumber _ruleNumber; // SEG:S90
  DateTime _departureDT; // ERD:D08
  DateTime _createDate; // ERD:D12
  AccountCode _accountCode; // CAL:AC0
  std::string _directionality; // CAL:S70
  std::string _createTime; // ERD:D55
  std::string _cat35Type; // ERD:N1P
  std::string _ticketDesignator; // ERD:BE0
  FareType _fareType; // ERD:S53
  SequenceNumber _sequenceNumber; // ERD:Q1K
  BookingCode _bookingCode; // ERD:P72
  NonPublishedValues _cat25Values; // C25
  NonPublishedValues _cat35Values; // C35
  NonPublishedValues _discountedValues; // DFI
  ConstructedAttributes _constructedAttributes; // ERD
  AddOnAttributes _origAddonAttributes; // ERD:OAO
  AddOnAttributes _destAddonAttributes; // ERD:DAO
  CarrierCode _validatingCarrier; // ERD:B00
  GlobalDirection _globalIndicator; // CAL:A60
  bool _wasCommandPricing; // ERD:CP0
  char _fareBreak; // SEG:P2F
};

inline ERDFareComp::ERDFareComp()
  : _fareAmount(0),
    _nucFareAmount(0),
    _paxTypeNumber(0),
    _pricingUnitNumber(0),
    _fareClassLength(0),
    _oneWayFare(0),
    _roundTripFare(0),
    _linkNumber(0),
    _fareTariff(0),
    _directionality("FR"),
    _sequenceNumber(0),
    _globalIndicator(GlobalDirection::NO_DIR),
    _wasCommandPricing(false),
    _fareBreak(TRUE_INDICATOR)
{
}
}
