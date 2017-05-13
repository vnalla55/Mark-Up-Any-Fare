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

#include "Rules/PointOverlappingForItineraryApplicator.h"
#include "Rules/PointOverlappingForItineraryRule.h"

namespace tax
{
PointOverlappingForItineraryRule::PointOverlappingForItineraryRule()
{
}

PointOverlappingForItineraryRule::~PointOverlappingForItineraryRule()
{
}

PointOverlappingForItineraryRule::ApplicatorType
PointOverlappingForItineraryRule::createApplicator(type::Index const& /*itinIndex*/,
                                                   const Request& /*request*/,
                                                   Services& /*services*/,
                                                   RawPayments& itinPayments) const
{
  return ApplicatorType(this, itinPayments);
}

std::string
PointOverlappingForItineraryRule::getDescription(Services&) const
{
  return "POINT OVERLAPPING FOR ITINERARY";
}
}

