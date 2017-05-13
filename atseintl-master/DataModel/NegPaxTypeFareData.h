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

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

namespace tse
{

class CategoryRuleItemInfo;
class PaxTypeFare;

/*
 * Temporary hold processing data related to new negotiated fare so
 * that we can make decision whether to accept or reject a
 * fare based on
 * 1. It has IF qualifier.
 * 2. It has 3/4 directionality.
 * 3. It is first match without qualifier or directionality.
 * 4. it is not first match but hierarchy applied.
 */
struct NegPaxTypeFareData
{
  NegPaxTypeFareData();

  void reset();

  bool operator==(const NegPaxTypeFareData& rhs) const;

  bool isQualifierPresent;
  bool isDirectionalityApplied;
  bool isDir3;
  bool isDir4;
  bool isFqTrx;
  bool isPricingOption; /* Is fare create from pricingRule? */
  uint32_t itemNo;
  Indicator inOutInd;
  PaxTypeCode psgType;
  PaxTypeFare* ptf;
  const CategoryRuleItemInfo* catRuleItemInfo; // Record 3 for diag purpose
  uint16_t r2SubSetNum;
  CarrierCode validatingCxr;
};

} // tse

