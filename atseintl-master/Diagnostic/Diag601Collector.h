//----------------------------------------------------------------------------
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
class PaxTypeFare;

class Diag601Collector : public DiagCollector
{
public:
  Diag601Collector& operator<<(const PaxTypeFare& fare) override;
  Diag601Collector& operator<<(const FareUsage& fu) override;

  virtual void printHeader() override;

private:
  void displayCat10RuleData(const CombinabilityRuleInfo* rec2Cat10, const PaxTypeFare& fare);
  void displayFareInfo(const PaxTypeFare& fare);
};

} /* end tse namespace */

