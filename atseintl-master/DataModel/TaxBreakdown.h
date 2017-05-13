//-------------------------------------------------------------------
//
//  File:        TaxBreakdown.h
//  Created:     November 2, 2004
//  Design:      Mike Carroll
//  Authors:
//
//  Description: Pricing Breakdown Tax object.
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

class TaxBreakdown
{
public:
  TaxBreakdown() : _amount(0), _amountPublished(0), _type(' ') {}

  TaxCode& code() { return _code; }
  const TaxCode& code() const { return _code; }

  MoneyAmount& amount() { return _amount; }
  const MoneyAmount& amount() const { return _amount; }

  CurrencyCode& currencyCode() { return _currencyCode; }
  const CurrencyCode& currencyCode() const { return _currencyCode; }

  MoneyAmount& amountPublished() { return _amountPublished; }
  const MoneyAmount& amountPublished() const { return _amountPublished; }

  CarrierCode& airlineCode() { return _airlineCode; }
  const CarrierCode& airlineCode() const { return _airlineCode; }

  LocCode& airportCode() { return _airportCode; }
  const LocCode& airportCode() const { return _airportCode; }

  NationCode& countryCode() { return _countryCode; }
  const NationCode& countryCode() const { return _countryCode; }

  std::string& description() { return _description; }
  const std::string& description() const { return _description; }

  CurrencyCode& publishedCurrencyCode() { return _publishedCurrencyCode; }
  const CurrencyCode& publishedCurrencyCode() const { return _publishedCurrencyCode; }

  Indicator& type() { return _type; }
  const Indicator& type() const { return _type; }

private:
  // Tax Breakdown Information        // TBD
  std::string _description; // TDS
  CurrencyCode _currencyCode; // TCC
  CurrencyCode _publishedCurrencyCode; // TPC
  MoneyAmount _amount; // TAT
  MoneyAmount _amountPublished; // TAP
  CarrierCode _airlineCode; // TAL
  TaxCode _code; // TCD
  NationCode _countryCode; // TCO
  LocCode _airportCode; // TPO
  Indicator _type; // A05
};
} // tse namespace
