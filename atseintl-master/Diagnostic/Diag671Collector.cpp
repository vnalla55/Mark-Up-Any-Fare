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

#include "Diagnostic/Diag671Collector.h"

#include "Diagnostic/DiagnosticUtil.h"
#include "Pricing/FareMarketPath.h"
#include "Pricing/FarePathFactory.h"

namespace tse
{
void
Diag671Collector::printHeader()
{
  if (_active)
    *this << "\n*** DIAG 671 FOR FARE MARKET PATH PROCESSING ***\n";
}

void
Diag671Collector::displayFmp(PricingTrx& trx,
                             const uint16_t fmpOrder,
                             const FareMarketPath* fmp,
                             const bool isExpanding,
                             const bool nextFmpSameAmount,
                             const bool thruFarePricing,
                             const FarePathFactory* fpf,
                             const PaxType* paxType)
{
  if (!_active)
    return;

  *this << (isExpanding ? "EXPANDING " : "SKIPPING ") << paxType->paxType() << " FMP " << fmpOrder
        << " (" << fmp->firstFareAmt(paxType) << " NUC) SINCE ";

  if (isExpanding)
  {
    if (nextFmpSameAmount)
      *this << "FMP IS SAME AMOUNT AS PREVIOUS\n";
    else if (!fpf)
      *this << "FPF PQ IS EMPTY\n";
    else
      *this << "FMP IS SAME/LOWER THAN LOWEST FPF (" << fpf->lowerBoundFPAmount() << " NUC)\n";
  }
  else // skipping FMP
  {
    if (thruFarePricing)
      *this << "FMP DOES NOT HAVE THRU FARES\n";
    else if ((trx.altDateCutOffNucThreshold() > 0) &&
             (fmp->firstFareAmt(paxType) - trx.altDateCutOffNucThreshold() > EPSILON))
      *this << "CUTOFF NUC AMOUNT " << trx.altDateCutOffNucThreshold() << " IS REACHED\n";
    else
      *this << "FMP DATE PAIR IS ALREADY PRICED\n";
  }
}

void
Diag671Collector::displayExpansion(const size_t fmpSize,
                                   const size_t fpfSize,
                                   const FarePathFactory* fpf,
                                   const PaxType* paxType,
                                   const size_t pqSize)
{
  if (!_active)
    return;

  *this << fmpSize << " " << paxType->paxType() << " FMPS EXPANDED, " << fpfSize << " FPFS ADDED";
  if (fpf)
    *this << " (" << fpf->lowerBoundFPAmount() << " NUC)";

  *this << ", PQSIZE=" << pqSize << "\n";
}

void
Diag671Collector::removeFpfDiag(const DatePair* datePair,
                                const bool nonThruPricing,
                                const FarePathFactory* fpf)
{
  if (!_active)
    return;

  *this << "REMOVING FPF (" << fpf->lowerBoundFPAmount() << " NUC) ";

  if (datePair)
    *this << "SINCE DATE PAIR IS ALREADY PRICED\n";
  else if (nonThruPricing)
    *this << "SINCE IT DOES NOT HAVE THRU FARE\n";
  else
    *this << "FOR ACCOMPANIED TRAVEL\n";
}
}
