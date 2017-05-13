//-------------------------------------------------------------------
//
//  File:        PlusUpDetail.h
//  Created:     May 2, 2005
//  Design:      Gregory Graham
//  Authors:
//
//  Description: Plus Up information for WPDF
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

class PlusUpDetail
{
public:
  PlusUpDetail() : _amount(0) {}

  MoneyAmount& amount() { return _amount; }
  const MoneyAmount& amount() const { return _amount; }

  LocCode& origCity() { return _origCity; }
  const LocCode& origCity() const { return _origCity; }

  LocCode& destCity() { return _destCity; }
  const LocCode& destCity() const { return _destCity; }

  std::string& message() { return _message; }
  const std::string& message() const { return _message; }

  LocCode& viaCity() { return _viaCity; }
  const LocCode& viaCity() const { return _viaCity; }

  NationCode& countryOfPayment() { return _countryOfPayment; }
  const NationCode& countryOfPayment() const { return _countryOfPayment; }

private:
  // PlusUp detail Information                      // PUP
  MoneyAmount _amount; // PUA
  LocCode _origCity; // PUO
  LocCode _destCity; // PUD
  std::string _message; // PUM
  LocCode _viaCity; // PUV
  NationCode _countryOfPayment; // PUC
};
} // tse namespace
