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
#ifndef MOCK_PRICING_UNIT_RULE_CONTROLLER_H
#define MOCK_PRICING_UNIT_RULE_CONTROLLER_H

#include "Rules/PricingUnitRuleController.h"

#include "gmock/gmock.h"

namespace tse
{

class MockPricingUnitRuleController : public PricingUnitRuleController
{
public:
  MOCK_METHOD5(doCategoryPostProcessing,
               Record3ReturnTypes(PricingTrx&,
                                  RuleControllerDataAccess&,
                                  const uint16_t&,
                                  RuleProcessingData&,
                                  const Record3ReturnTypes));

  MOCK_METHOD8(callCategoryRuleItemSet,
               Record3ReturnTypes(CategoryRuleItemSet&,
                                  const CategoryRuleInfo&,
                                  const std::vector<CategoryRuleItemInfoSet*>&,
                                  RuleControllerDataAccess&,
                                  RuleProcessingData&,
                                  bool,
                                  bool,
                                  bool));
};

} // namespace tse

#endif
