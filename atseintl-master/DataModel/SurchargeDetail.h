//-------------------------------------------------------------------
//
//  File:        SurchargeDetail.h
//  Created:     November 2, 2004
//  Design:      Mike Carroll
//  Authors:
//
//  Description: Surcharge Detail object.
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

class SurchargeDetail
{
public:
  SurchargeDetail() : _amount(0) {}

  std::string& type() { return _type; }
  const std::string& type() const { return _type; }

  MoneyAmount& amount() { return _amount; }
  const MoneyAmount& amount() const { return _amount; }

  CurrencyCode& currencyCode() { return _currencyCode; }
  const CurrencyCode& currencyCode() const { return _currencyCode; }

  CurrencyCode& publishedCurrency() { return _publishedCurrency; }
  const CurrencyCode& publishedCurrency() const { return _publishedCurrency; }

  std::string& description() { return _description; }
  const std::string& description() const { return _description; }

  LocCode& origin() { return _origin; }
  const LocCode& origin() const { return _origin; }

  LocCode& destination() { return _destination; }
  const LocCode& destination() const { return _destination; }

private:
  // Surcharge detail Information 				// SUR
  std::string _type; // CST
  std::string _description; // SCD
  CurrencyCode _currencyCode; // SCU
  CurrencyCode _publishedCurrency; // SCU
  MoneyAmount _amount; // CSH
  LocCode _origin; // A11
  LocCode _destination; // A12
};
} // tse namespace
