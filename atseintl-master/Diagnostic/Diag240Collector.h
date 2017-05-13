//----------------------------------------------------------------------------
//  File:        Diag240Collector.h
//  Authors:
//  Created:
//
//  Description: Diagnostic 240 formatter
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2006
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

#include "Common/TseStringTypes.h"
#include "Diagnostic/DiagCollector.h"

#include <vector>

namespace tse
{
class FareFocusRuleInfo;
class FareMarket;
class PricingTrx;

class Diag240Collector : public DiagCollector
{
public:
  explicit Diag240Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag240Collector() = default;

  void printFareMarketHeaderFFR(PricingTrx& trx, const FareMarket& fm);
  void printPaxTypeFare(PricingTrx& trx, const PaxTypeFare& paxTypeFare);
  void printDiagSecurityHShakeNotFound();
  void printDiagFareFocusRulesNotFound();
  void printFareFocusLookup(const uint64_t fareFocusRuleId, StatusFFRuleValidation rc);
  void printFareFocusLookup(StatusFFRuleValidation rc);
  void printFareFocusRuleNoneFound();
  void displayStatus(StatusFFRuleValidation status);
  void printFareFocusRuleStatus(StatusFFRuleValidation rc);
  void printFareFocusRuleInfo(const FareFocusRuleInfo* ffri);
};

} // namespace tse

