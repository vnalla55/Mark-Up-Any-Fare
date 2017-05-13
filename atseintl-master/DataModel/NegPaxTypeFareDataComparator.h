// -------------------------------------------------------------------
//
//  Copyright (C) Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
// -------------------------------------------------------------------

#pragma once

#include "DataModel/NegPaxTypeFareData.h"
#include "Rules/RuleConst.h"

namespace tse
{

struct NegPaxTypeFareDataComparator
{

  bool isSameDirectionality(const NegPaxTypeFareData& left, const NegPaxTypeFareData& right) const
  {
    if (!left.isDirectionalityApplied || !right.isDirectionalityApplied)
      return false;

    return (left.isDir3 && right.isDir3) || (left.isDir4 && right.isDir4) ||
           (left.inOutInd != RuleConst::ALWAYS_APPLIES &&
            right.inOutInd != RuleConst::ALWAYS_APPLIES && left.inOutInd == right.inOutInd);
  }

  bool areEquivalent(const NegPaxTypeFareData& left, const NegPaxTypeFareData& right) const
  {
    if (!(left.isPricingOption || left.isFqTrx) || !(right.isPricingOption || right.isFqTrx))
      return false;
    if (left.psgType != right.psgType)
      return false;

    if (isSameDirectionality(left, right))
    {
      if (left.r2SubSetNum == right.r2SubSetNum)
        return true;
      if (!left.isQualifierPresent && !right.isQualifierPresent)
        return true;
    }

    if (left.isQualifierPresent || right.isQualifierPresent)
      return false;
    if (left.isDirectionalityApplied || right.isDirectionalityApplied)
      return false;

    return true;
  }

  bool areSameButValCxrNot(const NegPaxTypeFareData& left, const NegPaxTypeFareData& right) const
  {
    if (UNLIKELY(!(left.isPricingOption || left.isFqTrx) || !(right.isPricingOption || right.isFqTrx)))
      return false;
    if (left.psgType != right.psgType)
      return false;
    if (UNLIKELY(left.validatingCxr.empty() || right.validatingCxr.empty()))
      return false;

    if (left.validatingCxr == right.validatingCxr)
      return false;

    if(left.catRuleItemInfo != right.catRuleItemInfo)
      return false;
    if(left.r2SubSetNum != right.r2SubSetNum)
      return false;

    return true;
  }

};

} // tse

