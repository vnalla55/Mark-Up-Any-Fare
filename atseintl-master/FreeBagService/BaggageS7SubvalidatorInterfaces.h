//-------------------------------------------------------------------
//  Copyright Sabre 2015
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

#include "Common/TseEnums.h"
#include "Common/TsePrimitiveTypes.h"

namespace tse
{
class BaggageCharge;
class OptionalServicesInfo;
class OCFees;

class INonBtaFareSubvalidator
{
public:
  virtual ~INonBtaFareSubvalidator() {}
  virtual StatusS7Validation validate(const OptionalServicesInfo& s7, OCFees& ocFees) = 0;
};

class IBtaSubvalidator
{
public:
  static constexpr Indicator BTA_A = 'A';
  static constexpr Indicator BTA_S = 'S';
  static constexpr Indicator BTA_J = 'J';
  static constexpr Indicator BTA_M = 'M';
  static constexpr Indicator BTA_EMPTY = ' ';

  virtual ~IBtaSubvalidator() {}
  virtual StatusS7Validation validate(const OptionalServicesInfo& s7, OCFees& ocFees) = 0;
};

class IAllowanceCollector
{
public:
  virtual ~IAllowanceCollector() {}
  virtual bool collect(OCFees& ocFees) = 0; // return true if no more are needed
};

class IChargeCollector
{
public:
  virtual ~IChargeCollector() {}
  virtual bool needAny() const = 0;
  virtual bool needThis(const BaggageCharge& bc) const = 0;
  virtual bool collect(BaggageCharge& bc) = 0; // return true if no more are needed
};
}
