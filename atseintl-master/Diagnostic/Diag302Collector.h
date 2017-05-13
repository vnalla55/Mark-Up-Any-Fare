//----------------------------------------------------------------------------
//  File:        Diag302Collector.h
//  Authors:
//  Created:
//
//  Description: Diagnostic 302 formatter
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

#include "Common/TseStringTypes.h"
#include "DBAccess/Record2Types.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class PaxTypeFare;
class DayTimeAppInfo;

class Diag302Collector : public DiagCollector
{
public:
  //@TODO will be removed, once the transition is done
  explicit Diag302Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag302Collector() {}

  void diag302Collector(const PaxTypeFare& paxTypeFare,
                        const PricingUnit& pu,
                        const Record3ReturnTypes status,
                        const std::string& phase,
                        const CategoryRuleInfo& cri,
                        const DayTimeAppInfo& ruleInfo);

private:
  std::string getOrdinal(int input) const;
};

} // namespace tse

