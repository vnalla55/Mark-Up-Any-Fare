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

#include "Rules/JourneyLoc1AsOriginRule.h"

#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"
#include "ServiceInterfaces/Services.h"
#include "Rules/JourneyLoc1AsOriginApplicator.h"

namespace tax
{

JourneyLoc1AsOriginRule::JourneyLoc1AsOriginRule(LocZone const& zone, type::Vendor const& vendor)
  : _locZone(zone), _vendor(vendor)
{
}

JourneyLoc1AsOriginRule::ApplicatorType
JourneyLoc1AsOriginRule::createApplicator(const type::Index& itinIndex,
                                          const Request& request,
                                          Services& services,
                                          RawPayments& /*itinPayments*/) const
{
  const Itin& itin = request.getItinByIndex(itinIndex);
  const type::Index& geoPathRefId = itin.geoPathRefId();
  const Geo& startGeo = request.geoPaths()[geoPathRefId].geos()[0];

  return ApplicatorType(*this, startGeo, services.locService());
}

std::string
JourneyLoc1AsOriginRule::getDescription(Services&) const
{
  return "THE JOURNEY GEO LOC1 MUST BE ORIGIN AND RESTRICTED TO " + _locZone.toString();
}
}
