//-------------------------------------------------------------------
//
//  File:        OneProgramOneBrand.h
//  Created:     March 2013
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2013
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

#include <boost/noncopyable.hpp>

#include <vector>

namespace tse
{
class OneProgramOneBrand : boost::noncopyable
{
public:
  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  ProgramCode& programCode() { return _programCode; }
  const ProgramCode& programCode() const { return _programCode; }

  ProgramName& programName() { return _programName; }
  const ProgramName& programName() const { return _programName; }

  BrandCode& brandCode() { return _brandCode; }
  const BrandCode& brandCode() const { return _brandCode; }

  BrandNameS8& brandName() { return _brandName; }
  const BrandNameS8& brandName() const { return _brandName; }

  TierNumber& tier() { return _tier; }
  TierNumber tier() const { return _tier; }

  std::vector<PaxTypeCode>& passengerType() { return _paxTypeCode; }
  const std::vector<PaxTypeCode>& passengerType() const { return _paxTypeCode; }

  SystemCode& systemCode() { return _systemCode; }
  const SystemCode& systemCode() const { return _systemCode; }

private:
  CarrierCode _carrier;
  ProgramCode _programCode;
  ProgramName _programName;
  BrandCode _brandCode;
  BrandNameS8 _brandName;
  TierNumber _tier = 0;
  std::vector<PaxTypeCode> _paxTypeCode;
  SystemCode _systemCode = ' ';
};
} // tse

