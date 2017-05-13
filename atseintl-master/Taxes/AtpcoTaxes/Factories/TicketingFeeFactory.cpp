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
#include "DataModel/RequestResponse/InputTicketingFee.h"
#include "DomainDataObjects/TicketingFee.h"
#include "Factories/TicketingFeeFactory.h"

namespace tax
{

TicketingFee
TicketingFeeFactory::createFromInput(const InputTicketingFee& inputTicketingFee)
{
  return TicketingFee(inputTicketingFee._id,
                      inputTicketingFee._amount,
                      inputTicketingFee._taxAmount,
                      inputTicketingFee._subCode);
}

} // namespace tax
