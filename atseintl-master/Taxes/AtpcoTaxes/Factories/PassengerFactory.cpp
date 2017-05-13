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
#include "DataModel/RequestResponse/InputPassenger.h"
#include "DomainDataObjects/Passenger.h"
#include "Factories/PassengerStatusFactory.h"
#include "Factories/PassengerFactory.h"

namespace tax
{

Passenger
PassengerFactory::createFromInput(const InputPassenger& inputPassenger)
{
  Passenger result;
  result._code = inputPassenger._code;
  result._birthDate = type::Date(inputPassenger._birthDate);
  result._stateCode = inputPassenger._stateCode;
  result._passengerStatus = PassengerStatusFactory::createFromInput(inputPassenger._passengerStatus);
  return result;
}

} // namespace tax
