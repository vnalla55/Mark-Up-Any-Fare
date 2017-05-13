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

#include "DataModel/Services/PreviousTicketTaxInfo.h"

#include <map>
#include <set>
#include <string>

namespace tax
{
class PreviousTicketService
{
public:
  virtual const std::set<PreviousTicketTaxInfo>& getTaxesForPreviousTicket() const = 0;

  virtual std::set<std::string> getParentTaxes(const std::string& sabreTaxCode) const = 0;

  virtual ~PreviousTicketService() {}
};
}
