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
#include "DataModel/RequestResponse/InputTicketingOptions.h"
#include "DomainDataObjects/TicketingOptions.h"
#include "Factories/TicketingOptionsFactory.h"

namespace tax
{

TicketingOptions
TicketingOptionsFactory::createFromInput(const InputTicketingOptions& inputTicketingOptions)
{
  TicketingOptions result;
  result.paymentCurrency() = inputTicketingOptions.paymentCurrency();
  result.formOfPayment() = inputTicketingOptions.formOfPayment();
  result.ticketingPoint() = inputTicketingOptions.ticketingPoint();
  result.ticketingDate() = type::Date(inputTicketingOptions.ticketingDate());
  result.ticketingTime() = type::Time(inputTicketingOptions.ticketingTime());
  return result;
}

} // namespace tax
