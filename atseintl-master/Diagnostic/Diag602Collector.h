//----------------------------------------------------------------------------
//  Copyright Sabre 2007
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

#include "DataModel/FareCompInfo.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class PaxTypeFare;
class ProcessTagInfo;

class Diag602Collector : public DiagCollector
{
private:
  bool _filterPassed;

public:
  explicit Diag602Collector(Diagnostic& root) : DiagCollector(root), _filterPassed(true) {}
  Diag602Collector() : _filterPassed(true) {}

  virtual Diag602Collector& operator<<(const PaxTypeFare& ptf) override;
  Diag602Collector& operator<<(RepriceFareValidationResult r);
  void printPtiInfo(RepriceFareValidationResult r, const ProcessTagInfo& pti);
  void initializeFilter(PricingTrx& trx, const PaxTypeFare& ptf);
  void displayErrorMessage(const MoneyAmount& excAmount, const MoneyAmount& newAmount);
};

} // namespace tse

