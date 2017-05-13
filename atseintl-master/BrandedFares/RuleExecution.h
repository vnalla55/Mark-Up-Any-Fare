//-------------------------------------------------------------------
//
//  File:        RuleExecution.h
//  Created:     Oct 2013
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
#include "Common/TseStringTypes.h"

#include <boost/noncopyable.hpp>

#include <vector>

// This class represents a single S8 Branded Program data

namespace tse
{
class MarketRule;
class S8BrandingSecurity;
struct PatternInfo;
struct MarketRuleInfo;

class RuleExecution : boost::noncopyable
{
public:
  ProgramCode& programCode() { return _programCode; }
  const ProgramCode& programCode() const { return _programCode; }

  ProgramID& ruleID() { return _ruleID; }
  const ProgramID& ruleID() const { return _ruleID; }

  std::vector<PatternInfo*>& passengerType() { return _passengerType; }
  const std::vector<PatternInfo*>& passengerType() const { return _passengerType; }

  std::vector<PatternInfo*>& accountCodes() { return _accountCodes; }
  const std::vector<PatternInfo*>& accountCodes() const { return _accountCodes; }

  std::vector<MarketRuleInfo*>& marketRules() { return _marketRules; }
  const std::vector<MarketRuleInfo*>& marketRules() const { return _marketRules; }

  std::vector<PatternInfo*>& pseudoCityCodes() { return _pseudoCityCodes; }
  const std::vector<PatternInfo*>& pseudoCityCodes() const { return _pseudoCityCodes; }

  std::vector<PatternInfo*>& governingCarrier() { return _governingCarrier; }
  const std::vector<PatternInfo*>& governingCarrier() const { return _governingCarrier; }

  DateTime& salesDateStart() { return _salesDateStart; }
  const DateTime& salesDateStart() const { return _salesDateStart; }

  DateTime& salesDateEnd() { return _salesDateEnd; }
  const DateTime& salesDateEnd() const { return _salesDateEnd; }

  DateTime& travelDateStart() { return _travelDateStart; }
  const DateTime& travelDateStart() const { return _travelDateStart; }

  DateTime& travelDateEnd() { return _travelDateEnd; }
  const DateTime& travelDateEnd() const { return _travelDateEnd; }

  S8BrandingSecurity*& s8BrandingSecurity() { return _s8BrandingSecurity; }
  const S8BrandingSecurity* s8BrandingSecurity() const { return _s8BrandingSecurity; }

  StatusS8& status() { return _status; }
  const StatusS8& status() const { return _status; }

  VendorCode& vendorCode() { return _vendorCode; }
  const VendorCode& vendorCode() const { return _vendorCode; }

private:
  ProgramCode _programCode;
  ProgramID _ruleID;
  std::vector<PatternInfo*> _passengerType;
  std::vector<PatternInfo*> _accountCodes;
  std::vector<MarketRuleInfo*> _marketRules;
  std::vector<PatternInfo*> _pseudoCityCodes;
  std::vector<PatternInfo*> _governingCarrier;
  DateTime _salesDateStart;
  DateTime _salesDateEnd;
  DateTime _travelDateStart;
  DateTime _travelDateEnd;
  S8BrandingSecurity* _s8BrandingSecurity = nullptr;
  StatusS8 _status = StatusS8::PASS_S8;
  VendorCode _vendorCode;
};
} // tse

