
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
class Diag455Collector : public RoutingDiagCollector
{
public:
  friend class Diag455CollectorTest;

  explicit Diag455Collector(Diagnostic& root) : RoutingDiagCollector(root) {}
  Diag455Collector() {}

  void displayRouteMapAndRestrictions(PricingTrx& trx,
                                      const TravelRoute& tvlRoute,
                                      const RoutingInfos* routingInfos);

private:
  void buildHeader();
  void displayHeader2(const RoutingInfo& rtgInfo);
  void displayRtgAddonHeader(const RoutingInfo& rtgInfo);
  void displayNoRouteStrings(const RoutingInfo& rtgInfo);
  void displayLine(bool firstPass);
  bool displayRouteStrings(const RoutingMapStrings* rMapStrings);
  void breakString(const std::string& rString);
  std::string::size_type searchString(const std::string& rString, uint16_t searchStart);
};

} // namespace tse

