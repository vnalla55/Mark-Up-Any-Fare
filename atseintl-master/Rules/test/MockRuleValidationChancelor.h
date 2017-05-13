// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#ifndef MOCK_RULE_VALIDATION_CHANCELOR_H
#define MOCK_RULE_VALIDATION_CHANCELOR_H

#include "Rules/RuleValidationChancelor.h"

#include "gmock/gmock.h"

namespace tse
{

class MockRuleValidationChancelor : public RuleValidationChancelor
{
public:
  MOCK_METHOD1(hasPolicy, bool(const uint16_t));
  MOCK_CONST_METHOD1(getPolicy, RuleValidationPolicy&(const uint16_t));
  MOCK_CONST_METHOD0(getMonitor, const RuleValidationMonitor&());
  MOCK_METHOD1(updateContext, void(const RuleValidationContext::ContextType&));
  MOCK_METHOD1(updateContext, void(RuleControllerDataAccess&));
  MOCK_CONST_METHOD0(getContext, RuleValidationContext&());
  MOCK_METHOD0(getMutableMonitor, RuleValidationMonitor&());
};

} // namespace tse

#endif
