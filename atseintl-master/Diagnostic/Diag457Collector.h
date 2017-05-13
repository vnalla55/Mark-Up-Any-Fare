//
//----------------------------------------------------------------------------
//  File:   Diag457Collector.h
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
#include "Routing/RtgKey.h"

namespace tse
{
class Routing;

class Diag457Collector : public RoutingDiagCollector
{
public:
  explicit Diag457Collector(Diagnostic& root) : RoutingDiagCollector(root) {}
  Diag457Collector() {}

  void buildHeader();
  void displayRoutingListItem(const Routing* routing, int16_t listItemNumber);
  void displayRoutingMapItem(std::map<RtgKey, bool>::const_iterator rMap, int16_t mapItemNumber);
};

} // namespace tse

