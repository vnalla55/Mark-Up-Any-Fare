//-----------------------------------------------------------------------------
//
//  File:     Diag327Collector.h
//
//  Author :  Konrad Koch
//
//  Copyright Sabre 2009
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the agreement/contract
//          under which the program(s) have been supplied.
//
//-----------------------------------------------------------------------------

#pragma once

#include "DBAccess/Record2Types.h"
#include "Diagnostic/DiagCollector.h"
#include "Rules/RuleItem.h"

#include <string>

namespace tse
{
class PaxTypeFare;
class Tours;

class Diag327Collector : public DiagCollector
{
  friend class Diag327CollectorTest;

public:
  explicit Diag327Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag327Collector() {}

  void collect(RuleItem::ProcessingPhase phase,
               const PaxTypeFare& paxTypeFare,
               const CategoryRuleInfo& ruleInfo,
               const Tours& tours);
  void displayStatus(Record3ReturnTypes result);

private:
  void displayHeader(RuleItem::ProcessingPhase phase,
                     const PaxTypeFare& paxTypeFare,
                     const CategoryRuleInfo& catRuleInfo);
  void displayRecord2Info(const CategoryRuleInfo& catRuleInfo);
  void displayRecord2SegmentsInfo(const CategoryRuleInfo& catRuleItemInfo);
  void displayRecord2SegmentInfo(const CategoryRuleItemInfo& catRuleItemInfo);
  void displayRecord3Info(const Tours& tours);
};

} // namespace tse

