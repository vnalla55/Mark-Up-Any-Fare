//----------------------------------------------------------------------------
//  File:        OcFeeGroupConfig.h
//  Created:     2010-04-12
//
//  Description: Class used to keep summary groups configuration of OC Fees
//
//  Copyright Sabre 2010
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"

#include <vector>

namespace tse
{

class OcFeeGroupConfig
{
public:
  OcFeeGroupConfig()
    : _groupCode(""), _startRange(-1), _endRange(-1), _commandName(""), _applyTo('I')
  {
  }

  ServiceGroup& groupCode() { return _groupCode; }
  const ServiceGroup& groupCode() const { return _groupCode; }

  int& startRange() { return _startRange; }
  const int& startRange() const { return _startRange; }

  int& endRange() { return _endRange; }
  const int& endRange() const { return _endRange; }

  char& applyTo() { return _applyTo; }
  const char& applyTo() const { return _applyTo; }

  std::string& commandName() { return _commandName; }
  const std::string& commandName() const { return _commandName; }

  std::vector<ServiceSubTypeCode>& subTypeCodes() { return _subTypeCodes; }
  const std::vector<ServiceSubTypeCode>& subTypeCodes() const { return _subTypeCodes; }

  static std::string parseOCFeesSummaryConfiguration(const std::string& inputString,
                                                     std::vector<OcFeeGroupConfig>& outputVec);

private:
  static std::string parseOcFeeGroup(const std::string& inputString, OcFeeGroupConfig& outputGroup);

  static std::string parseQuantity(const std::string& inputString, OcFeeGroupConfig& outputGroup);

  static std::string
  parseSortingRules(const std::string& inputString, OcFeeGroupConfig& outputGroup);

  static std::string
  parseSubTypeCodes(const std::string& inputString, OcFeeGroupConfig& outputGroup);

  ServiceGroup _groupCode;
  int _startRange;
  int _endRange;
  std::string _commandName;
  char _applyTo;
  std::vector<ServiceSubTypeCode> _subTypeCodes;
};

} // end namespace tse

