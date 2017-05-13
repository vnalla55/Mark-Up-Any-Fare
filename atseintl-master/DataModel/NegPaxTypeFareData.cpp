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

#include "DataModel/NegPaxTypeFareData.h"

namespace tse
{

NegPaxTypeFareData::NegPaxTypeFareData()
  : isQualifierPresent(false),
    isDirectionalityApplied(false),
    isDir3(false),
    isDir4(false),
    isFqTrx(false),
    isPricingOption(false),
    itemNo(0),
    inOutInd('0'),
    psgType(""),
    ptf(nullptr),
    catRuleItemInfo(nullptr),
    r2SubSetNum(0)
{
}

void
NegPaxTypeFareData::reset()
{
  isQualifierPresent = false;
  isDirectionalityApplied = false;
  isDir3 = false;
  isDir4 = false;
  isFqTrx = false;
  isPricingOption = false;
  itemNo = 0;
  inOutInd = '0';
  psgType = "", ptf = nullptr;
  catRuleItemInfo = nullptr;
  r2SubSetNum = 0;
  validatingCxr = "";
}

bool
NegPaxTypeFareData::
operator==(const NegPaxTypeFareData& rhs) const
{
  return isQualifierPresent == rhs.isQualifierPresent &&
         isDirectionalityApplied == rhs.isDirectionalityApplied && isDir3 == rhs.isDir3 &&
         isDir4 == rhs.isDir4 && isFqTrx == rhs.isFqTrx && isPricingOption == isPricingOption &&
         itemNo == rhs.itemNo && inOutInd == rhs.inOutInd && psgType == rhs.psgType &&
         ptf == rhs.ptf && catRuleItemInfo == rhs.catRuleItemInfo && r2SubSetNum == rhs.r2SubSetNum &&
         validatingCxr == rhs.validatingCxr;
};

} // tse
