//-------------------------------------------------------------------
//
//  File:        CurrencyDetail.h
//  Created:     April 29, 2005
//  Design:      Gregory Graham
//  Authors:
//
//  Description: Currency conversion information for WPDF
//
//  Updates:
//
//  Copyright Sabre 2005
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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

#include <string>

namespace tse
{

class CurrencyDetail
{
public:
  CurrencyDetail();

  // Accessors
  CurrencyCode& from() { return _from; }
  const CurrencyCode& from() const { return _from; }

  CurrencyCode& to() { return _to; }
  const CurrencyCode& to() const { return _to; }

  CurrencyCode& intermediateCurrency() { return _intermediateCurrency; }
  const CurrencyCode& intermediateCurrency() const { return _intermediateCurrency; }

  MoneyAmount& amount() { return _amount; }
  const MoneyAmount& amount() const { return _amount; }

  uint16_t& decimalPlaces() { return _decimalPlaces; }
  const uint16_t& decimalPlaces() const { return _decimalPlaces; }

  MoneyAmount& convertedAmount() { return _convertedAmount; }
  const MoneyAmount& convertedAmount() const { return _convertedAmount; }

  uint16_t& decimalPlacesConvertedAmount() { return _decimalPlacesConvertedAmount; }
  const uint16_t& decimalPlacesConvertedAmount() const { return _decimalPlacesConvertedAmount; }

  ExchRate& exchangeRateOne() { return _exchangeRateOne; }
  const ExchRate& exchangeRateOne() const { return _exchangeRateOne; }

  uint16_t& decimalPlacesExchangeRateOne() { return _decimalPlacesExchangeRateOne; }
  const uint16_t& decimalPlacesExchangeRateOne() const { return _decimalPlacesExchangeRateOne; }

  ExchRate& exchangeRateTwo() { return _exchangeRateTwo; }
  const ExchRate& exchangeRateTwo() const { return _exchangeRateTwo; }

  uint16_t& decimalPlacesExchangeRateTwo() { return _decimalPlacesExchangeRateTwo; }
  const uint16_t& decimalPlacesExchangeRateTwo() const { return _decimalPlacesExchangeRateTwo; }

  CarrierCode& currencyConversionCarrierCode() { return _currencyConversionCarrierCode; }
  const CarrierCode& currencyConversionCarrierCode() const
  {
    return _currencyConversionCarrierCode;
  }

  NationCode& countryCode() { return _countryCode; }
  const NationCode& countryCode() const { return _countryCode; }

  std::string& travelDate() { return _travelDate; }
  const std::string& travelDate() const { return _travelDate; }

  std::string& currencyCoversionFareBasisCode() { return _currencyCoversionFareBasisCode; }
  const std::string& currencyCoversionFareBasisCode() const
  {
    return _currencyCoversionFareBasisCode;
  }

  std::string& conversionType() { return _conversionType; }
  const std::string& conversionType() const { return _conversionType; }

  std::string& applicationType() { return _applicationType; }
  const std::string& applicationType() const { return _applicationType; }

  std::string& effectiveDate() { return _effectiveDate; }
  const std::string& effectiveDate() const { return _effectiveDate; }

  std::string& discontinueDate() { return _discontinueDate; }
  const std::string& discontinueDate() const { return _discontinueDate; }

private:
  // Currency detail Information                    // CCD
  MoneyAmount _amount; // ATC
  MoneyAmount _convertedAmount; // CAT
  ExchRate _exchangeRateOne; // EX1
  ExchRate _exchangeRateTwo; // EX2
  CurrencyCode _from; // FCR
  CurrencyCode _to; // TCR
  CurrencyCode _intermediateCurrency; // ICR
  CarrierCode _currencyConversionCarrierCode; // CCO
  NationCode _countryCode; // CNC
  uint16_t _decimalPlaces; // NDF
  uint16_t _decimalPlacesConvertedAmount; // NDC
  uint16_t _decimalPlacesExchangeRateOne; // ND1
  uint16_t _decimalPlacesExchangeRateTwo; // ND2
  std::string _travelDate; // TDT
  std::string _currencyCoversionFareBasisCode; // FCO
  std::string _conversionType; // COT
  std::string _applicationType; // ATY
  std::string _effectiveDate; // CED
  std::string _discontinueDate; // CDD
};

inline CurrencyDetail::CurrencyDetail()
  : _amount(0),
    _convertedAmount(0),
    _exchangeRateOne(0),
    _exchangeRateTwo(0),
    _decimalPlaces(0),
    _decimalPlacesConvertedAmount(0),
    _decimalPlacesExchangeRateOne(0),
    _decimalPlacesExchangeRateTwo(0)
{
}

} // tse namespace

