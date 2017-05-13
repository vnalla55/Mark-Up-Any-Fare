// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "Rules/JourneyLoc1AsOriginRule.h"

namespace tax
{
class Request;

class JourneyLoc1AsOriginExRule : public JourneyLoc1AsOriginRule
{
public:
  JourneyLoc1AsOriginExRule(LocZone const& zone, type::Vendor const& vendor);

  ApplicatorType createApplicator(type::Index const& itinIndex,
                                  const Request& request,
                                  Services& services,
                                  RawPayments& /*itinPayments*/) const override;
};
}
