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

#include "DataModel/Common/Types.h"

namespace tax
{

class InputTicketingFeeUsage
{
public:
  type::Index& index() { return _ticketingFeeRefId; }
  const type::Index& index() const { return _ticketingFeeRefId; }

  type::Index _ticketingFeeRefId;
};

} // namespace tax
