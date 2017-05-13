//-------------------------------------------------------------------
//
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Diagnostic/DiagCollector.h"
#include "Pricing/PUPath.h"

namespace tse
{
class Agent;
class FareMarketPath;
class PricingTrx;

class Diag666Collector : public DiagCollector
{
public:
  Diag666Collector& operator<<(const Agent& agent);
  void printPassengerValidation(LocCode residency);
  void printItinValidation(const PricingTrx& trx);
  void
  printSpanishResidentAmount(PricingTrx& trx, PUPath& puPath);
  void printSolutionPatternInfo(PricingTrx& trx,
                                PUPath& puPath,
                                const Itin& itin);
  void printFarePathSolutions(const FarePath& farePath,
                              const PricingTrx& trx,
                              PUPath& puPath);
};
}
