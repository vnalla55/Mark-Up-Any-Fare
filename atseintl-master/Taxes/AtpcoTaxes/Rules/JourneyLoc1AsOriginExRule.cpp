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

#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"
#include "Rules/JourneyLoc1AsOriginExRule.h"
#include "ServiceInterfaces/Services.h"

#include "Rules/JourneyLoc1AsOriginApplicator.h"

namespace tax
{
JourneyLoc1AsOriginExRule::JourneyLoc1AsOriginExRule(LocZone const& zone, type::Vendor const& vendor)
  : JourneyLoc1AsOriginRule(zone, vendor)
{
}

JourneyLoc1AsOriginExRule::ApplicatorType
JourneyLoc1AsOriginExRule::createApplicator(const type::Index& itinIndex,
                                            const Request& request,
                                            Services& services,
                                            RawPayments& /*itinPayments*/) const
{
  const Itin& itin = request.getItinByIndex(itinIndex);
  const type::Index& geoPathRefId = itin.geoPathRefId();

  const Geo& startGeo =
    request.prevTicketGeoPath().geos().size() ?
      request.prevTicketGeoPath().geos()[0] : request.geoPaths()[geoPathRefId].geos()[0];

  ApplicatorType applicator(*this, startGeo, services.locService());
  applicator.setSkipExempt(true);
  return applicator;
}

}
