#pragma once

#include "Common/FallbackUtil.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"

namespace tse
{
// delete this file along with fallback
FIXEDFALLBACK_DECL(dynamicCastsFBRPTFRuleData)

namespace PTFRuleData
{
template <typename T>
inline decltype(auto)
toFBRPaxTypeFare(T paxTypeFareRuleData)
{
  using ReturnType = decltype(paxTypeFareRuleData->toFBRPaxTypeFareRuleData());
  if (fallback::fixed::dynamicCastsFBRPTFRuleData())
    return dynamic_cast<ReturnType>(paxTypeFareRuleData);

  return paxTypeFareRuleData->toFBRPaxTypeFareRuleData();
}
} // PTFRuleData
} // tse
