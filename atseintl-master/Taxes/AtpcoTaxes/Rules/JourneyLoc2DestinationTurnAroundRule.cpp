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

#include "Rules/JourneyLoc2DestinationTurnAroundRule.h"

#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"
#include "ServiceInterfaces/Services.h"
#include "Rules/JourneyLoc2DestinationTurnAroundApplicator.h"

namespace tax
{

JourneyLoc2DestinationTurnAroundRule::JourneyLoc2DestinationTurnAroundRule(
    const type::JrnyInd& jrnyInd,
    const LocZone& zone,
    const type::Vendor& vendor,
    bool alternateTurnaroundDeterminationLogic)
  : _jrnyInd(jrnyInd),
    _locZone(zone),
    _vendor(vendor),
    _alternateTurnaroundDeterminationLogic(alternateTurnaroundDeterminationLogic)
{
}

JourneyLoc2DestinationTurnAroundRule::~JourneyLoc2DestinationTurnAroundRule() {}

JourneyLoc2DestinationTurnAroundRule::ApplicatorType
JourneyLoc2DestinationTurnAroundRule::createApplicator(const type::Index& itinIndex,
                                                       const Request& request,
                                                       Services& services,
                                                       RawPayments& /*itinPayments*/) const
{
  const Itin& itin = request.getItinByIndex(itinIndex);
  return ApplicatorType(*this,
                        itin,
                        services.locService(),
                        services.mileageService());
}

std::string
JourneyLoc2DestinationTurnAroundRule::getDescription(Services&) const
{
  if (_jrnyInd == type::JrnyInd::JnyLoc2DestPointForOWOrTurnAroundForRTOrOJ)
    return "THE JOURNEY GEO LOC2 DESTINATION FOR ONE WAY OR TURNAROUND POINT FOR "
           "ROUND TRIP/OPEN JAW RESTRICTED TO " + _locZone.toString();
  else
    return "THE JOURNEY GEO LOC2 DESTINATION POINT RESTRICTED TO " + _locZone.toString();
}

} // namespace tax
