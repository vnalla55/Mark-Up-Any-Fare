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
#include "DataModel/RequestResponse/InputPassengerStatus.h"
#include "DomainDataObjects/PassengerStatus.h"
#include "Factories/PassengerStatusFactory.h"

namespace tax
{

PassengerStatus
PassengerStatusFactory::createFromInput(const InputPassengerStatus& inputPassengerStatus)
{
  PassengerStatus result;
  result._nationality = inputPassengerStatus._nationality;
  result._residency = inputPassengerStatus._residency;
  result._employment = inputPassengerStatus._employment;
  return result;
}

} // namespace tax
