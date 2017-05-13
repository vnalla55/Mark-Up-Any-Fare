//-------------------------------------------------------------------
//
//  File:        TaxDetail.h
//  Created:     November 2, 2004
//  Design:      Mike Carroll
//  Authors:
//
//  Description: Pricing Detail Tax object.
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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

#include <string>

namespace tse
{

class TaxDetail
{
public:
  TaxDetail() : _amount(0), _amountPublished(0) {}

  MoneyAmount& amount() { return _amount; }
  const MoneyAmount& amount() const { return _amount; }

  TaxCode& code() { return _code; }
  const TaxCode& code() const { return _code; }

  CurrencyCode& currencyCode() { return _currencyCode; }
  const CurrencyCode& currencyCode() const { return _currencyCode; }

  std::string& description() { return _description; }
  const std::string& description() const { return _description; }

  MoneyAmount& amountPublished() { return _amountPublished; }
  const MoneyAmount& amountPublished() const { return _amountPublished; }

  CurrencyCode& publishedCurrencyCode() { return _publishedCurrencyCode; }
  const CurrencyCode& publishedCurrencyCode() const { return _publishedCurrencyCode; }

  CurrencyCode& emuCurrencyCode() { return _emuCurrencyCode; }
  const CurrencyCode& emuCurrencyCode() const { return _emuCurrencyCode; }

  std::string& gst() { return _gst; }
  const std::string& gst() const { return _gst; }

  NationCode& countryCode() { return _countryCode; }
  const NationCode& countryCode() const { return _countryCode; }

  CarrierCode& airlineCode() { return _airlineCode; }
  const CarrierCode& airlineCode() const { return _airlineCode; }

  LocCode& stationCode() { return _stationCode; }
  const LocCode& stationCode() const { return _stationCode; }

private:
  // Tax detail Information 						// TAX
  std::string _description; // TDS
  std::string _gst; // GST
  MoneyAmount _amount; // TAT
  MoneyAmount _amountPublished; // TAP
  TaxCode _code; // TCD
  CurrencyCode _currencyCode; // TCC
  CurrencyCode _publishedCurrencyCode; // TPC
  CurrencyCode _emuCurrencyCode; // EMU
  NationCode _countryCode; // TCO
  CarrierCode _airlineCode; // TAL
  LocCode _stationCode; // TSC
};
} // tse namespace
