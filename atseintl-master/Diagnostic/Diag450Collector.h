//
//----------------------------------------------------------------------------
//  File:   Diag450Collector.h
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

#include "Diagnostic/RoutingDiagCollector.h"
#include "Routing/RoutingInfo.h"

namespace tse
{
class Routing;
class TravelRoute;

class Diag450Collector : public RoutingDiagCollector
{
public:
  friend class Diag450CollectorTest;

  explicit Diag450Collector(Diagnostic& root) : RoutingDiagCollector(root) {}
  Diag450Collector() {}

  void displayRoutingValidationResults(PricingTrx& trx,
                                       const TravelRoute& tvlRoute,
                                       const RoutingInfos* routingInfos);

private:
  void buildHeader();
};

} // namespace tse

