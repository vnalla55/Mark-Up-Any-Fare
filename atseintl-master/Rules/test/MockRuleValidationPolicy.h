//------------------------------------------------------------------
//
//  File: MockRuleValidationPolicy.h
//
//  Copyright Sabre 2014
//
//    The copyright to the computer program(s) herein
//    is the property of Sabre.
//    The program(s) may be used and/or copied only with
//    the written permission of Sabre or in accordance
//    with the terms and conditions stipAulated in the
//    agreement/contract under which the program(s)
//    have been supplied.
//
//-------------------------------------------------------------------

#ifndef MOCKRULEVALIDATIONPOLICY_H_
#define MOCKRULEVALIDATIONPOLICY_H_

#include <gmock/gmock.h>
#include "Rules/RuleValidationPolicy.h"

namespace tse
{

class MockRuleValidationPolicy : public RuleValidationPolicy
{
public:
  MOCK_METHOD1(shouldPerform, bool(const RuleValidationContext& context));
  MOCK_METHOD0(shouldReturn, bool());
  ~MockRuleValidationPolicy() {}
};

} // namespace tse
#endif /* MOCKRULEVALIDATIONPOLICY_H_ */
