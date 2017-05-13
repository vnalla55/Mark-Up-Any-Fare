//----------------------------------------------------------------------------
//  File:        Diag905Collector.h
//  Author:      Adrienne A. Stipe
//  Created:     2004-09-08
//
//  Description: Diagnostic 905 formatter
//
//  Updates:
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

#include "DataModel/ExchangeOverrides.h"
#include "DataModel/RexShoppingTrx.h"
#include "Diagnostic/Diag200Collector.h"
#include "Diagnostic/Diag902Collector.h"
#include "Diagnostic/DiagCollector.h"

#include <string>

namespace tse
{
class Diversity;
class FlightFinderTrx;

class Diag905Collector : public Diag902Collector
{
public:
  explicit Diag905Collector(Diagnostic& root) : Diag902Collector(root) {}
  Diag905Collector() {}

  virtual Diag905Collector& operator<<(const PricingTrx& pricingTrx) override;

private:
  void outputAirSeg(const ShoppingTrx::SchedulingOption* sop,
                    int segNum,
                    const class AirSeg& seg,
                    const std::string& indentation,
                    bool displayRexShoppingData = false);
  void outputAgentData(const Agent* agent, DiagCollector& dc, bool displayRexShoppingData = false);
  void outputExchangeItinData(const RexShoppingTrx& rexShoppingTrx, DiagCollector& dc);
  void outputExchangeFareComponentData(const RexShoppingTrx& rexShoppingTrx, DiagCollector& dc);
  void outputExchangePUPData(const RexShoppingTrx& rexShoppingTrx, DiagCollector& dc);
  void outputJourneyItinSegs(const FlightFinderTrx& ffTrx, DiagCollector& dc);
  void outputRexPassengerData(const PaxType* curPT, int& count, DiagCollector& dc);
  void printSegment(const AirSeg* journeySeg, DiagCollector& dc);
  void outputAltDatesList(const PricingTrx& trx, DiagCollector& dc);
  void outputDiversityParameters(const Diversity& dvr);
};

} // namespace tse

