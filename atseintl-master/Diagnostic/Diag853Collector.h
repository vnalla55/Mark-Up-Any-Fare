//----------------------------------------------------------------------------
//  File:        Diag853Collector.h
//  Authors:
//  Created:
//
//  Description: Diagnostic 853 formatter
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
#include "DBAccess/FareCalcConfig.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class PricingTrx;
class FareCalcConfig;

class Diag853Collector : public DiagCollector
{
public:
  explicit Diag853Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag853Collector() {}

  void process(const PricingTrx& trx, const FareCalcConfig*& fareCalcConfig);

private:
  bool buildFareCalcConfigDisplay(const PricingTrx& trx, const FareCalcConfig*& fareCalcConfig);
  void buildFareCalcConfigSegDisplay(const FareCalcConfig*& fareCalcConfig);
  void writeFareCalcConfigHeader();
  void writeFareCalcConfigSegHeader();
  void writeFooter();
};

} // namespace tse

