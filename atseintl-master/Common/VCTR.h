//-------------------------------------------------------------------
//
//  File:        VCTR.h
//  Created:     January 25, 2007
//  Authors:     Andrew Ahmad
//
//  Description:
//
//
//  Copyright Sabre 2007
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

namespace tse
{
class VCTR final
{
public:
  VCTR() = default;

  VCTR(const VendorCode vendor,
       const CarrierCode carrier,
       const TariffNumber tariff,
       const RuleNumber rule,
       const uint32_t sequenceNumber)
    : _vendor(vendor),
      _carrier(carrier),
      _tariff(tariff),
      _rule(rule),
      _sequenceNumber(sequenceNumber)
  {
  }

  const VendorCode vendor() const { return _vendor; }
  VendorCode& vendor() { return _vendor; }

  const CarrierCode carrier() const { return _carrier; }
  CarrierCode& carrier() { return _carrier; }

  const TariffNumber tariff() const { return _tariff; }
  TariffNumber& tariff() { return _tariff; }

  const RuleNumber rule() const { return _rule; }
  RuleNumber& rule() { return _rule; }

  const uint32_t sequenceNumber() const { return _sequenceNumber; }
  uint32_t& sequenceNumber() { return _sequenceNumber; }

  void clear();

  VCTR& operator=(const VCTR& rhs);

  bool operator==(const VCTR& rhs) const;

  bool operator!=(const VCTR& rhs) const { return !(*this == rhs); }

private:
  VendorCode _vendor;
  CarrierCode _carrier;
  TariffNumber _tariff = 0;
  RuleNumber _rule;
  uint32_t _sequenceNumber = 0;
};
} // namespace tse
