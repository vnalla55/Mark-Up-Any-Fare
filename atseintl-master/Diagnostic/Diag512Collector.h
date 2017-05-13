//----------------------------------------------------------------------------
//  File:        Diag512Collector.h
//  Authors:
//  Created:
//
//  Description: Diagnostic 512 formatter
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
class FarePath;
class FareUsage;
class PricingTrx;
class PricingUnit;
class SurchargeData;

class Diag512Collector : public DiagCollector
{
public:
  //@TODO will be removed, once the transition is done
  explicit Diag512Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag512Collector() = default;

  void diag512Collector(MoneyAmount& amt, FarePath& fp, const PricingTrx& trx);

private:
  void writeFPHeader(MoneyAmount& amt, const FarePath& fp);
  void writePUHeader(const PricingUnit& pu);
  void writeFUHeader(const FareUsage& fu);
  void displaySurchargeData(const SurchargeData& sd, const PricingTrx& trx);
};

} // namespace tse

