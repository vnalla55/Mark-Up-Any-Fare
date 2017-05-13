//----------------------------------------------------------------------------
//  File:        Diag316Collector.h
//  Created:
//
//  Description: Diagnostic 316 formatter
//
//  Updates:
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

#include "DBAccess/Record2Types.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{

class PaxTypeFare;
class PenaltyInfo;
class PricingTrx;
class RuleValidationContext;

class Diag316Collector : public DiagCollector
{
public:
  explicit Diag316Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag316Collector() = default;

  void writeHeader(PaxTypeFare& paxTypeFare,
                   const CategoryRuleInfo& cri,
                   const PenaltyInfo& penaltyInfo,
                   PricingTrx& trx,
                   bool farePhase);
  void printFlexFareNoPenalties(const flexFares::ValidationStatusPtr& validationStatus,
                                const Record3ReturnTypes& validationResult,
                                const RuleValidationContext& context);

  static constexpr Indicator NO_APPLICATION = ' ';
};

} // namespace tse

