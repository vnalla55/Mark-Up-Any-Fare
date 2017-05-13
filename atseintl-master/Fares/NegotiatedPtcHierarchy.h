//-------------------------------------------------------------------
//
//  File:        NegotiatedPtcHierarchy.h
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

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"

#include <vector>

namespace tse
{
class PaxTypeFare;
class NegPaxTypeFareRuleData;
class NegFareRest;

class NegotiatedPtcHierarchy
{
  friend class NegotiatedPtcHierarchyTest;

public:
  static bool isNegGroup(const PaxTypeCode& paxTypeCode);
  static bool isJcbGroup(const PaxTypeCode& paxTypeCode);
  static bool isPfaGroup(const PaxTypeCode& paxTypeCode);
  static bool ifPtcsBelongToTheSameGroup(const PaxTypeFare& ptFare, const PaxTypeCode& paxTypeCode);

  static bool loadPtcHierarchy();
  static void loadNegPtcHierarchy();
  static void loadJcbPtcHierarchy();
  static void loadPfaPtcHierarchy();

  static bool findLowerHierarchy(const NegPaxTypeFareRuleData& ruleData,
                                 const NegFareRest& negFareRest,
                                 const PaxTypeFare& ptFare);

  static bool matchLowerHierarchy(const PaxTypeFare& ptFare,
                                  const PaxTypeCode& paxTypeCode1,
                                  const PaxTypeCode& paxTypeCode2);

private:
  NegotiatedPtcHierarchy() {}

  static std::vector<PaxTypeCode> negPtcHierarchy;
  static std::vector<PaxTypeCode> jcbPtcHierarchy;
  static std::vector<PaxTypeCode> pfaPtcHierarchy;
};

} // namespace tse

