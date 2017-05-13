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
#include "DataModel/RequestResponse/InputFlight.h"
#include "DomainDataObjects/Flight.h"
#include "Factories/FlightFactory.h"

namespace tax
{

Flight
FlightFactory::createFromInput(const InputFlight& inputFlight)
{
  Flight result;
  result.departureTime() = inputFlight._departureTime;
  result.arrivalTime() = inputFlight._arrivalTime;
  result.arrivalDateShift() = inputFlight._arrivalDateShift;
  result.marketingCarrierFlightNumber() = inputFlight._marketingCarrierFlightNumber;
  result.marketingCarrier() = inputFlight._marketingCarrier;
  result.operatingCarrier() = inputFlight._operatingCarrier;
  result.equipment() = inputFlight._equipment;
  result.cabinCode() = inputFlight._cabinCode;
  result.reservationDesignatorCode() = inputFlight._reservationDesignator;
  return result;
}

} // namespace tax
