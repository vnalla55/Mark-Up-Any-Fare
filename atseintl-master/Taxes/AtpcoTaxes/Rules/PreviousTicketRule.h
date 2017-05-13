// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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
#include "Rules/BusinessRule.h"
#include "Rules/PreviousTicketApplicator.h"

namespace tax
{
class Request;
class Services;
class RawPayments;

class PreviousTicketRule : public BusinessRule
{
  type::Percent _taxPercentage;

public:
  PreviousTicketRule(const type::Percent& taxPercentage);

  typedef PreviousTicketApplicator ApplicatorType;

  PreviousTicketApplicator createApplicator(const type::Index& /*itinIndex*/,
                                            const Request& /*request*/,
                                            Services& services,
                                            RawPayments& /*rawPayments*/) const;

  std::string getDescription(Services& services) const override;
};
}
