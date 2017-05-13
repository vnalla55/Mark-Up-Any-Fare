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

#include "Rules/ContinuousJourneyRule.h"

#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"
#include "Rules/ContinuousJourneyApplicator.h"

namespace tax
{

ContinuousJourneyRule::ContinuousJourneyRule(type::RtnToOrig const& rtnToOrig)
  : _rtnToOrig(rtnToOrig)
{
}

ContinuousJourneyRule::~ContinuousJourneyRule() {}

ContinuousJourneyRule::ApplicatorType
ContinuousJourneyRule::createApplicator(type::Index const& itinIndex,
                                        const Request& request,
                                        Services& /*services*/,
                                        RawPayments& /*itinPayments*/) const
{
  Itin const& itin = request.getItinByIndex(itinIndex);
  type::Index const& geoPathRefId = itin.geoPathRefId();
  return ApplicatorType(*this, request.geoPaths()[geoPathRefId]);
}

std::string
ContinuousJourneyRule::getDescription(Services&) const
{
  return std::string("LOC1 HAS TO BE ") +
         (_rtnToOrig == type::RtnToOrig::ContinuousJourney1stPoint ? "1ST" : "2ND") +
         " POINT IN CONTINOUS JOURNEY";
}
}
