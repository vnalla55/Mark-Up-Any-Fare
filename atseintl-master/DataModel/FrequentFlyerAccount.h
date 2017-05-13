//-------------------------------------------------------------------
//
//  File:       FrequentFlyerAccount.h
//  Created:    August 11, 2005
//  Authors:    Andrea Yang
//
//  Description: Frequent Flyer Account Information for Traveler
//
//  Updates:
//
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
#include <vector>

namespace tse
{
class FrequentFlyerAccount
{
  Indicator _vipType = 0; // FREQ Flyer VIP-tye
  CarrierCode _carrier; // FREQ Flyer Carrier Code
  std::string _accountNumber; // FREQ Flyer Number
  std::vector<CarrierCode> _partner; // FREQ Flyer Partner
  std::string _tierLevel; // FREQ Flyer Tier Level

public:
  Indicator& vipType() { return _vipType; }
  const Indicator& vipType() const { return _vipType; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  std::string& accountNumber() { return _accountNumber; }
  const std::string& accountNumber() const { return _accountNumber; }

  std::vector<CarrierCode>& partner() { return _partner; }
  const std::vector<CarrierCode>& partner() const { return _partner; }

  std::string& tierLevel() { return _tierLevel; }
  const std::string& tierLevel() const { return _tierLevel; }
};
} // tse namespace
