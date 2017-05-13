//----------------------------------------------------------------------------
//  File:        Diag500Collector.h
//  Authors:
//  Created:
//
//  Description: Diagnostic 500 formatter
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
class FareByRuleCtrlInfo;
class FootNoteCtrlInfo;
class GeneralFareRuleInfo;

class Diag500Collector : public DiagCollector
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

  //@TODO will be removed, once the transition is done
  explicit Diag500Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag500Collector() = default;

  void printHeader() override;
  void printRulePhase(const CategoryPhase phase);
  void diag500Collector(const PaxTypeFare& paxFare,
                        const GeneralFareRuleInfo* rule,
                        const bool fareRule = true);
  void diag500Collector(const PaxTypeFare& paxFare, const FootNoteCtrlInfo* rule);
  void diag500Collector(const PaxTypeFare& paxFare, const FareByRuleCtrlInfo* rule);
  void diag500Collector(Record3ReturnTypes statusRule);
  void displayRelation(const PaxTypeFare& paxFare,
                       const CategoryRuleItemInfo* rule,
                       Record3ReturnTypes statusRule);
  void displayDates(const PaxTypeFare& paxFare);

  static ValidationPhase& givenValidationPhase() { return _givenValidationPhase; }
  static ValidationPhase& validationPhase() { return _validationPhase; }

  static uint16_t& categoryNumber() { return _categoryNumber; }

private:
  bool shouldDisplay(const PaxTypeFare& paxFare, const CategoryRuleItemInfo* rule);

  enum SeparatorType
  {
    RULE_HEADER = 1
  };

  void writeSeparator(SeparatorType);
  static ValidationPhase _givenValidationPhase;
  static ValidationPhase _validationPhase;
  static uint16_t _categoryNumber;
};

} // namespace tse

