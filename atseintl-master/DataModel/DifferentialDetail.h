//-------------------------------------------------------------------
//
//  File:        DifferentialDetail.h
//  Created:     June 15, 2005
//  Design:      Gregory Graham
//  Authors:
//
//  Description: Differential Detail object.
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

#include <string>

namespace tse
{

class DifferentialDetail
{
public:
  DifferentialDetail() : _amount(0), _mileagePctg(0), _cabinLow(0), _cabinHigh(0) {}

  LocCode& origCityHIP() { return _origCityHIP; }
  const LocCode& origCityHIP() const { return _origCityHIP; }

  LocCode& destCityHIP() { return _destCityHIP; }
  const LocCode& destCityHIP() const { return _destCityHIP; }

  LocCode& lowOrigHIP() { return _lowOrigHIP; }
  const LocCode& lowOrigHIP() const { return _lowOrigHIP; }

  LocCode& lowDestHIP() { return _lowDestHIP; }
  const LocCode& lowDestHIP() const { return _lowDestHIP; }

  LocCode& highOrigHIP() { return _highOrigHIP; }
  const LocCode& highOrigHIP() const { return _highOrigHIP; }

  LocCode& highDestHIP() { return _highDestHIP; }
  const LocCode& highDestHIP() const { return _highDestHIP; }

  FareClassCode& fareClassLow() { return _fareClassLow; }
  const FareClassCode& fareClassLow() const { return _fareClassLow; }

  FareClassCode& fareClassHigh() { return _fareClassHigh; }
  const FareClassCode& fareClassHigh() const { return _fareClassHigh; }

  Indicator& cabinLow() { return _cabinLow; }
  const Indicator& cabinLow() const { return _cabinLow; }

  Indicator& cabinHigh() { return _cabinHigh; }
  const Indicator& cabinHigh() const { return _cabinHigh; }

  MoneyAmount& amount() { return _amount; }
  const MoneyAmount& amount() const { return _amount; }

  const uint16_t mileagePctg() const { return _mileagePctg; }
  uint16_t& mileagePctg() { return _mileagePctg; }

private:
  // Differential detail Information      // HIP
  MoneyAmount _amount; // C50
  LocCode _origCityHIP; // A13
  LocCode _destCityHIP; // A14
  LocCode _lowOrigHIP; // A01
  LocCode _lowDestHIP; // A02
  LocCode _highOrigHIP; // A03
  LocCode _highDestHIP; // A04
  FareClassCode _fareClassLow; // B30
  FareClassCode _fareClassHigh; // BJ0
  uint16_t _mileagePctg; // Q48
  Indicator _cabinLow; // N00
  Indicator _cabinHigh; // N04
};
} // tse namespace
