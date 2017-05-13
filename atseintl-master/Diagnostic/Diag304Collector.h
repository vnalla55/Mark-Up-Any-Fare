//----------------------------------------------------------------------------
//  File:        Diag304Collector.h
//  Authors:
//  Created:
//
//  Description: Diagnostic 304 formatter
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
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class FlightAppRule;
class PricingRequest;

class Diag304Collector : public DiagCollector
{
public:
  //@TODO will be removed, once the transition is done
  explicit Diag304Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag304Collector() {}

  void diag304Collector(const PaxTypeFare& paxFare,
                        const FlightAppRule& flightAppRule,
                        const PricingRequest& request);
};

} // namespace tse

