//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"

namespace tse
{

class RuleValidationContext;
class RuleValidationMonitor;
class RuleValidationPolicy;

namespace flexFares
{
class ValidationStatus;
}

class RuleValidationObserver
{
public:
  typedef RuleValidationMonitor Monitor;

  RuleValidationObserver(Monitor& monitor) : _monitor(&monitor) {}

  virtual void update(const RuleValidationContext& rvc,
                      flexFares::ValidationStatusPtr validationStatus,
                      const uint16_t& catNumber,
                      const Record3ReturnTypes& ret) {};

  virtual void update(const uint16_t catNumber, const RuleValidationPolicy& policy) {}

  virtual void update(const RuleValidationContext& rvc,
                      const std::string& acctCodeOrCorpIdString,
                      flexFares::ValidationStatusPtr validationStatus,
                      const bool isAccountCode)
  {
  }

  virtual ~RuleValidationObserver() {}

protected:
  RuleValidationObserver() : _monitor(nullptr) {}

  Monitor* _monitor;
}; // RuleValidationObserver

} // tse

