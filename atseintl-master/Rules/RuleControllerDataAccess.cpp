#include "Rules/RuleControllerDataAccess.h"

#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "Rules/RuleConst.h"

#include <iostream>
#include <set>
#include <string>
#include <vector>

#include <stdlib.h>

namespace tse
{
PaxTypeFare&
RuleControllerDataAccess::getBaseOrPaxTypeFare(PaxTypeFare& ptFare) const
{
  if (!_retrieveBaseFare)
    return ptFare;

  // no base fare available if not Fare By Rule
  if (!ptFare.isFareByRule())
    return ptFare;

  FBRPaxTypeFareRuleData* fbr = ptFare.getFbrRuleData(RuleConst::FARE_BY_RULE);
  if (UNLIKELY(fbr == nullptr))
    return ptFare;

  return *fbr->baseFare();
}
}
