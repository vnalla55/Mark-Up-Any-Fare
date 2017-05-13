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

#include <boost/ptr_container/ptr_vector.hpp>

#include "DataModel/RequestResponse/InputTicketingFeeUsage.h"

namespace tax
{

class InputTicketingFeePath
{
public:
  boost::ptr_vector<InputTicketingFeeUsage>& ticketingFeeUsages() { return _ticketingFeeUsages; }
  const boost::ptr_vector<InputTicketingFeeUsage>& ticketingFeeUsages() const
  {
    return _ticketingFeeUsages;
  }

  type::Index& index() { return _id; }
  const type::Index& index() const { return _id; }

  type::Index _id;
  boost::ptr_vector<InputTicketingFeeUsage> _ticketingFeeUsages;
};

} // namespace tax
