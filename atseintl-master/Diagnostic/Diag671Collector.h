//----------------------------------------------------------------------------
//  File:        Diag671Collector.cpp
//
//  Copyright Sabre 2012
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
class FarePathFactory;

class Diag671Collector : public DiagCollector
{
public:
  void printHeader() override;
  void displayFmp(PricingTrx& trx,
                  const uint16_t fmpOrder,
                  const FareMarketPath* fmp,
                  const bool isExpanding,
                  const bool nextFmpSameAmount,
                  const bool thruFarePricing,
                  const FarePathFactory* fpf,
                  const PaxType* paxType);

  void displayExpansion(const size_t fmpSize,
                        const size_t fpfSize,
                        const FarePathFactory* fpf,
                        const PaxType* paxType,
                        const size_t pqSize);

  void
  removeFpfDiag(const DatePair* datePair, const bool nonThruPricing, const FarePathFactory* fpf);
};
}

