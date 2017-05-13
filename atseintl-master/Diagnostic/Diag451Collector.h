//----------------------------------------------------------------------------
//  File:        Diag451Collector.h
//  Authors:     Rahib Roy
//  Created:     March 2005
//
//  Description: Diagnostic 451 formatter
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
#include "DBAccess/RoutingKeyInfo.h"
#include "Diagnostic/RoutingDiagCollector.h"
#include "Routing/RoutingInfo.h"

#include <vector>

namespace tse
{
class Routing;
class TravelRoute;

class Diag451Collector : public RoutingDiagCollector
{
public:
  friend class Diag451CollectorTest;

  explicit Diag451Collector(Diagnostic& root) : RoutingDiagCollector(root) {}
  Diag451Collector() {}

  void displayRoutingValidationResults(TravelRoute& tvlRoute,
                                       const std::vector<RoutingKeyInfo*>& rtgKeyInfoV);

private:
  void buildHeader();
};

} // namespace tse

