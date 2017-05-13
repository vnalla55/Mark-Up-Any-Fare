// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#pragma once
#include "DomainDataObjects/FlightUsage.h"

namespace tax
{
class FlightUsageBuilder
{
  FlightUsage* _flightUsage;

public:
  FlightUsageBuilder() : _flightUsage(new FlightUsage) {}

  FlightUsageBuilder& setFlight(Flight* flight)
  {
    _flightUsage->flight() = flight;
    return *this;
  }

  FlightUsageBuilder& setConnectionDateShift(int16_t shift)
  {
    _flightUsage->connectionDateShift() = shift;
    return *this;
  }

  FlightUsageBuilder& setOpen(type::OpenSegmentIndicator ind)
  {
    _flightUsage->openSegmentIndicator() = ind;
    return *this;
  }

  FlightUsageBuilder& setForcedConnection(type::ForcedConnection ind)
  {
    _flightUsage->forcedConnection() = ind;
    return *this;
  }

  FlightUsage* build() { return _flightUsage; }
};
}

