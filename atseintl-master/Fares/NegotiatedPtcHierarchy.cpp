//-------------------------------------------------------------------
//
//  File:        NegotiatedPtcHierarchy.cpp
//  Created:     Jun 26, 2009
//  Authors:     Slawek Machowicz
//
//  Description: Code moved from NegotiatedFareController class
//
//  Updates:
//
//  Copyright Sabre 2009
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------
#include "Fares/NegotiatedPtcHierarchy.h"

#include "Common/PaxTypeUtil.h"
#include "Common/TseConsts.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/NegFareRest.h"

namespace tse
{
std::vector<PaxTypeCode> NegotiatedPtcHierarchy::negPtcHierarchy;
std::vector<PaxTypeCode> NegotiatedPtcHierarchy::jcbPtcHierarchy;
std::vector<PaxTypeCode> NegotiatedPtcHierarchy::pfaPtcHierarchy;

bool
NegotiatedPtcHierarchy::isNegGroup(const PaxTypeCode& paxTypeCode)
{
  return (paxTypeCode == CHILD || paxTypeCode == CNE || paxTypeCode == INFANT ||
          paxTypeCode == INE);
}

bool
NegotiatedPtcHierarchy::isJcbGroup(const PaxTypeCode& paxTypeCode)
{
  return (paxTypeCode == JCB || paxTypeCode == JNN || paxTypeCode == JNF);
}

bool
NegotiatedPtcHierarchy::isPfaGroup(const PaxTypeCode& paxTypeCode)
{
  return (paxTypeCode == CBC || paxTypeCode == CBI);
}

bool
NegotiatedPtcHierarchy::ifPtcsBelongToTheSameGroup(const PaxTypeFare& ptFare,
                                                   const PaxTypeCode& paxTypeCode)
{
  // checks if fare's ptc and specified ptc belong to the same group
  return
      // NEG
      (isNegGroup(ptFare.fcasPaxType()) && (paxTypeCode == NEG || isNegGroup(paxTypeCode))) ||
      // JCB
      (isJcbGroup(ptFare.fcasPaxType()) && (paxTypeCode == JCB || isJcbGroup(paxTypeCode))) ||
      // PFA
      (isPfaGroup(ptFare.fcasPaxType()) && (paxTypeCode == PFA || isPfaGroup(paxTypeCode)));
}

bool
NegotiatedPtcHierarchy::loadPtcHierarchy()
{
  loadNegPtcHierarchy();
  loadJcbPtcHierarchy();
  loadPfaPtcHierarchy();
  return true;
}

// The hierarchy for NEG/CNN/CNE/INF/INE group is blank-NEG-CNN-CNE-INF-INE
void
NegotiatedPtcHierarchy::loadNegPtcHierarchy()
{
  negPtcHierarchy.push_back(INE);
  negPtcHierarchy.push_back(INFANT);
  negPtcHierarchy.push_back(CNE);
  negPtcHierarchy.push_back(CHILD);
  negPtcHierarchy.push_back(NEG);
  negPtcHierarchy.push_back(ADULT);
}

// The hierarchy for JCB/JNN/JNF group is blank-JCB-JNN-JNF
void
NegotiatedPtcHierarchy::loadJcbPtcHierarchy()
{
  jcbPtcHierarchy.push_back(JNF);
  jcbPtcHierarchy.push_back(JNN);
  jcbPtcHierarchy.push_back(JCB);
}

// The hierarchy for PFA/CBC/CBI group is blank-PFA-CBC-CBI
void
NegotiatedPtcHierarchy::loadPfaPtcHierarchy()
{
  pfaPtcHierarchy.push_back(CBI);
  pfaPtcHierarchy.push_back(CBC);
  pfaPtcHierarchy.push_back(PFA);
}

bool
NegotiatedPtcHierarchy::findLowerHierarchy(const NegPaxTypeFareRuleData& ruleData,
                                           const NegFareRest& negFareRest,
                                           const PaxTypeFare& ptFare)
{
  const NegFareRest* previousNegFareRest =
      dynamic_cast<const NegFareRest*>(ruleData.ruleItemInfo());

  if (UNLIKELY(previousNegFareRest == nullptr))
    return false;

  const PaxTypeCode previousPtc = previousNegFareRest->psgType();
  const PaxTypeCode currentPtc = negFareRest.psgType();

  return matchLowerHierarchy(ptFare, previousPtc, currentPtc);
}

bool
NegotiatedPtcHierarchy::matchLowerHierarchy(const PaxTypeFare& ptFare,
                                            const PaxTypeCode& ptc1,
                                            const PaxTypeCode& ptc2)
{
  bool ret = false;
  std::vector<PaxTypeCode>* group = nullptr;

  // choose correct group
  if (isNegGroup(ptFare.fcasPaxType()))
    group = &negPtcHierarchy;
  else if (isJcbGroup(ptFare.fcasPaxType()))
    group = &jcbPtcHierarchy;
  else if (isPfaGroup(ptFare.fcasPaxType()))
    group = &pfaPtcHierarchy;

  // if group was found
  if (LIKELY(group))
  {
    std::vector<PaxTypeCode>::iterator pos1;
    std::vector<PaxTypeCode>::iterator pos2;

    pos1 = std::find(group->begin(), group->end(), ptc1); // find pos ptc1
    pos2 = std::find(group->begin(), group->end(), ptc2); // find pos ptc2

    if (pos2 < pos1)
    {
      ret = true; // matched
    }
  }

  return ret;
}

} // namespace
