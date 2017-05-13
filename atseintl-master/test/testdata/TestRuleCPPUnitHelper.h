#ifndef RULE_CPP_UNIT_HELPER
#define RULE_CPP_UNIT_HELPER

#include "Rules/RuleConst.h"

namespace tse
{

class CategoryRuleItemInfo;
class Itin;
class PaxTypeFare;

class TestRuleCPPUnitHelper
{

public:
  static bool buildTestItinAndRuleForIsDirectionPass(CategoryRuleItemInfo& catRuleItemInfo,
                                                     const Record3ReturnTypes desiredResult,
                                                     const char dir,
                                                     const PaxTypeFare& ptFare,
                                                     Itin& itin,
                                                     bool& isLocationSwapped);
};
}

#endif
