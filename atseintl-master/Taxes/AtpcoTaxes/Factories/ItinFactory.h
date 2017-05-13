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
#pragma once

#include "DomainDataObjects/TicketingFee.h"

namespace tax
{
class Itin;
class InputItin;
struct InputFlightPath;

class ItinFactory
{
public:
  static Itin createFromInput(const InputItin& inputItin,
                              const InputFlightPath& flightPath,
                              std::vector<TicketingFee>& ticketingFees);
};

} // namespace tax
