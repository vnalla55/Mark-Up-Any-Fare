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

#include "Rules/PointOverlappingForYqYrApplicator.h"
#include "Rules/PointOverlappingForYqYrRule.h"

namespace tax
{
PointOverlappingForYqYrRule::PointOverlappingForYqYrRule()
{
}

PointOverlappingForYqYrRule::~PointOverlappingForYqYrRule()
{
}

PointOverlappingForYqYrRule::ApplicatorType
PointOverlappingForYqYrRule::createApplicator(type::Index const& /*itinIndex*/,
                                              const Request& /*request*/,
                                              Services& /*services*/,
                                              RawPayments& itinPayments) const
{
  return ApplicatorType(this, itinPayments);
}

std::string
PointOverlappingForYqYrRule::getDescription(Services&) const
{
  return "POINT OVERLAPPING";
}
}

