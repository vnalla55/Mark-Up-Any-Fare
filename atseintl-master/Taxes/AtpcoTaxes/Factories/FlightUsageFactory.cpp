// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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
#include "DataModel/RequestResponse/InputFlightUsage.h"
#include "DomainDataObjects/FlightUsage.h"
#include "Factories/FlightUsageFactory.h"

namespace tax
{

FlightUsage
FlightUsageFactory::createFromInput(const InputFlightUsage& inputFlightUsage)
{
  FlightUsage result;
  result.flightRefId() = inputFlightUsage._flightRefId;
  result.connectionDateShift() = inputFlightUsage._connectionDateShift;
  result.openSegmentIndicator() = inputFlightUsage._openSegmentIndicator;
  result.flight() = nullptr;
  result.forcedConnection() = inputFlightUsage._forcedConnection;
  return result;
}

} // namespace tax
