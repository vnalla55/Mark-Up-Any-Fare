#include "DataModel/PaxTypeFareRuleData.h"

#include "DBAccess/DataHandle.h"

namespace tse
{

PaxTypeFareRuleData*
PaxTypeFareRuleData::clone(DataHandle& dataHandle) const
{
  PaxTypeFareRuleData* cloneObj = nullptr;
  dataHandle.get(cloneObj);

  copyTo(*cloneObj);

  return cloneObj;
}

void
PaxTypeFareRuleData::copyTo(PaxTypeFareRuleData& cloneObj) const
{
  cloneObj._baseFare = _baseFare;
  cloneObj._categoryRuleInfo = _categoryRuleInfo;
  cloneObj._isLocationSwapped = _isLocationSwapped;
  cloneObj._categoryRuleItemInfoSet = _categoryRuleItemInfoSet;
  cloneObj._categoryRuleItemInfoVec = _categoryRuleItemInfoVec;
  cloneObj._categoryRuleItemInfo = _categoryRuleItemInfo;
  cloneObj._ruleItemInfo = _ruleItemInfo;
  cloneObj._ruleStatus = _ruleStatus;
  cloneObj._validatingCarriers = _validatingCarriers;
}

} // tse namespace
