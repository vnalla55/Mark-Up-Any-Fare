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

#include "DomainDataObjects/Request.h"
#include "Rules/ItinPayments.h"
#include "ServiceInterfaces/Services.h"

namespace tax
{
class ApplicatorFactory
{
public:
  template <typename Rule>
  static typename Rule::ApplicatorType create(const Rule& rule,
                                              const type::Index& itinIndex,
                                              const Request& request,
                                              Services& services,
                                              RawPayments& rawPayments)
  {
    return rule.createApplicator(itinIndex, request, services, rawPayments);
  }
};
}

