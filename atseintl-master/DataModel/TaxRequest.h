//-------------------------------------------------------------------
//
//  File:         TaxRequest.h
//  Created:
//  Authors:
//
//  Description:
//
//  Updates:
//          08/25/07 - Dean Van Decker
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

#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

#include "DataModel/PricingRequest.h"

#include <string>
#include <vector>

namespace tse
{

class TaxRequest : public PricingRequest
{
public:
  TaxRequest();

  static const uint32_t EMPTY_SEQUENCE = 0xFFFFFFFF;

  //--------------------------------------------------------------------------
  // Accessors
  //--------------------------------------------------------------------------

  NationCode& nation() { return _nation; }
  const NationCode& nation() const { return _nation; }

  TaxDescription& nationDescription() { return _nationDescription; }
  const TaxDescription& nationDescription() const { return _nationDescription; }

  std::string& nationName() { return _nationName; }
  const std::string& nationName() const { return _nationName; }

  LocCode& airportCode() { return _airportCode; }
  const LocCode& airportCode() const { return _airportCode; }

  TaxCode& taxCode() { return _taxCode; }
  const TaxCode& taxCode() const { return _taxCode; }

  TaxType& taxType() { return _taxType; }
  const TaxType& taxType() const { return _taxType; }

  CarrierCode& carrierCode() { return _carrierCode; }
  const CarrierCode& carrierCode() const { return _carrierCode; }

  std::string& carrierCodes() { return _carrierCodes; }
  const std::string& carrierCodes() const { return _carrierCodes; }

  DateTime& effectiveDate() { return _effectiveDate; }
  const DateTime& effectiveDate() const { return _effectiveDate; }

  DateTime& travelDate() { return _travelDate; }
  const DateTime& travelDate() const { return _travelDate; }

  DateTime& requestDate() { return _requestDate; }
  const DateTime& requestDate() const { return _requestDate; }

  uint32_t& sequenceNumber() { return _sequenceNumber; }
  const uint32_t& sequenceNumber() const { return _sequenceNumber; }

  uint32_t& sequenceNumber2() { return _sequenceNumber2; }
  const uint32_t& sequenceNumber2() const { return _sequenceNumber2; }

  const std::vector<uint32_t>& categoryVec() const { return _categoryVec; }
  std::vector<uint32_t>& categoryVec() { return _categoryVec; }

  const std::string& categories() const { return _categories; }
  std::string& categories() { return _categories; }

  bool& menu() { return _menu; }
  void setReissue(bool reissue) { _reissue = reissue; }
  bool getReissue() const { return _reissue; }
  void setHelp(bool value) { _help = value; }
  bool help() const { return _help; }
  bool& helpTaxUS() { return _helpTaxUS; }

  Indicator& amtType() { return _amtType; }
  const Indicator& amtType() const { return _amtType; }

  Indicator& tripType() { return _tripType; }
  const Indicator& tripType() const { return _tripType; }

  MoneyAmount& fareAmt() { return _fareAmt; }
  const MoneyAmount& fareAmt() const { return _fareAmt; }

  LocCode& loc2() { return _loc2; }
  const LocCode& loc2() const { return _loc2; }

  LocCode& loc1() { return _loc1; }
  const LocCode& loc1() const { return _loc1; }

  std::string& txEntryCmd() { return _txEntryCmd; }
  const std::string& txEntryCmd() const { return _txEntryCmd; }

  std::string& txEntryType() { return _txEntryType; }
  const std::string& txEntryType() const { return _txEntryType; }

  std::string& txEntryDetailLevels() { return _txEntryDetailLevels; }
  const std::string& txEntryDetailLevels() const { return _txEntryDetailLevels; }

private:
  TaxRequest(const TaxRequest&);
  TaxRequest& operator=(const TaxRequest&);

  NationCode _nation;
  TaxDescription _nationDescription;
  std::string _nationName; // for Atpco TaxDisplay
  LocCode _airportCode; // for Atpco TaxDisplay
  TaxCode _taxCode;
  TaxType _taxType; // for Atpco TaxDisplay
  CarrierCode _carrierCode;
  std::string _carrierCodes; // for Atpco TaxDisplay, codes in pattern "CA|LO|AA..."
  DateTime _effectiveDate; //
  DateTime _travelDate; //
  DateTime _requestDate; // for Atpco TaxDisplay
  uint32_t _sequenceNumber; //
  uint32_t _sequenceNumber2; //
  std::vector<uint32_t> _categoryVec; //
  std::string _categories; // for Atpco TaxDisplay, categories in pattern "2|4|6..."
  bool _menu; //
  bool _reissue; //
  bool _help; // Default Help Indicator
  bool _helpTaxUS; // Special Help Indicator
  Indicator _amtType;
  Indicator _tripType;
  MoneyAmount _fareAmt;
  LocCode _loc1;
  LocCode _loc2;
  std::string _txEntryCmd; // for Atpco TaxDisplay
  std::string _txEntryType; // for Atpco TaxDisplay
  std::string _txEntryDetailLevels; // for Atpco TaxDisplay, levels in pattern "10|4|2..."
};

inline TaxRequest::TaxRequest()
  : _effectiveDate(DateTime::emptyDate()),
    _travelDate(DateTime::emptyDate()),
    _requestDate(DateTime::localTime()),
    _sequenceNumber(EMPTY_SEQUENCE),
    _sequenceNumber2(EMPTY_SEQUENCE),
    _menu(false),
    _reissue(false),
    _help(false),
    _helpTaxUS(false),
    _amtType(0),
    _tripType(0),
    _fareAmt(0)
{
}

} // tse namespace

