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
class PricingTrx;
class Fare;
class FareMarket;

class Diag400Collector : public DiagCollector
{
public:
  explicit Diag400Collector(Diagnostic& root)
    : DiagCollector(root), _lineCount(0), _highCabin4(false), _highCabin7(false)
  {
  }
  Diag400Collector() : _lineCount(0), _highCabin4(false), _highCabin7(false) {}

  Diag400Collector& operator<<(const PaxTypeFare& paxTfare) override;
  Diag400Collector& operator<<(const FareMarket& mkt) override;
  Diag400Collector& operator<<(const std::pair<PricingTrx*, PaxTypeFare*>& output) override;

  bool finalDiagTest(const PricingTrx& trx, const FareMarket& mkt);

private:
  int _lineCount; // total number of lines if necessary
  bool _highCabin4; // Premium Business Cabin RBD
  bool _highCabin7; // Premium Economy Cabin RBD

  void travelSegmentHeader(); // Display header for the fares in the travel segment

  void mixClassMessage(PricingTrx& trx, const PaxTypeFare& paxTfare);
  bool failedSegInEconomy(const FareMarket& mkt, const PaxTypeFare& paxTfare);
  bool tryRule2(PricingTrx& trx, const FareMarket& mkt);

  bool& highCabin4() { return _highCabin4; }
  const bool isHighCabin4() { return _highCabin4; }

  bool& highCabin7() { return _highCabin7; }
  const bool isHighCabin7() { return _highCabin7; }

  void collectHighCabin(CabinType cabin);
  std::string addVendorLine(const PaxTypeFare& ptf) const;
};
} // namespace tse

