//----------------------------------------------------------------------------
//  File:        Diag550Collector.h
//  Authors:
//  Created:
//
//  Description: Diagnostic 550 formatter
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
class FootNoteCtrlInfo;
class GeneralFareRuleInfo;

class Diag550Collector : public DiagCollector
{
public:
  //@TODO will be removed, once the transition is done
  explicit Diag550Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag550Collector() = default;

  void printHeader() override;
  void diag550Collector(const PaxTypeFare& paxFare,
                        const GeneralFareRuleInfo* rule,
                        const bool fareRule = true);
  void diag550Collector(const PaxTypeFare& paxFare, const FootNoteCtrlInfo* rule);
  void diag550Collector(Record3ReturnTypes statusRule);
  void displayRelation(const PaxTypeFare& paxFare,
                       const CategoryRuleItemInfo* rule,
                       Record3ReturnTypes statusRule);

private:
  enum SeparatorType
  {
    RULE_HEADER = 1
  };

  void writeSeparator(SeparatorType);
};

} // namespace tse

