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
#ifndef MOCK_RULE_VALIDATION_MONITOR_H
#define MOCK_RULE_VALIDATION_MONITOR_H

#include "Rules/RuleValidationMonitor.h"

#include "gmock/gmock.h"

namespace tse
{

class MockRuleValidationMonitor : public RuleValidationMonitor
{
public:
  MOCK_METHOD4(notify,
               void(RuleValidationMonitor::Event,
                    const RuleValidationContext&,
                    const uint16_t,
                    Record3ReturnTypes&));

  MOCK_METHOD4(notify,
               void(RuleValidationMonitor::Event,
                    const uint16_t,
                    const Record3ReturnTypes&,
                    const RuleValidationPolicy&));

  MOCK_METHOD2(subscribe, void(Observer*, const std::vector<Event>&));
};

} // namespace tse

#endif
