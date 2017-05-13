//----------------------------------------------------------------------------
//  File:        Diag502Collector.h
//  Created:
//  Authors:
//
//  Description: Diagnostic 502 formatter
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Diagnostic/DiagCollector.h"

namespace tse
{
class CategoryRuleItemInfo;
class CombinabilityRuleInfo;
class FootNoteCtrlInfo;
class GeneralFareRuleInfo;

class Diag502Collector : public DiagCollector
{
public:
  static const std::string SHORT_OUTPUT;
  static const std::string PREVALIDATION;
  static const std::string NORMAL_OR_FARECOMP_VALIDATION;
  static const std::string REVALIDATION;
  static const std::string DYNAMIC_VALIDATION;
  static const std::string SPECIFIC_CATEGORY;

  enum ValidationPhase
  {
    PRV = 0,
    NFV,
    REV,
    DYV,
    ANY = -1,
  };

  void printRulePhase(const CategoryPhase phase);
  void diag502Collector(const PaxTypeFare& paxFare, const GeneralFareRuleInfo& rule);
  void diag502Collector(const PaxTypeFare& paxFare, const FootNoteCtrlInfo& rule);
  void diag502Collector(const PaxTypeFare& paxFare, const CombinabilityRuleInfo& rule);

  static bool isDiagNeeded(PricingTrx& trx,
                           const PaxTypeFare& paxFare,
                           const std::vector<CategoryRuleItemInfoSet*>& ruleSet);
};

} // namespace tse

