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

#include "Rules/ReturnToOriginRule.h"

#include "DomainDataObjects/Itin.h"
#include "DomainDataObjects/Request.h"
#include "Rules/ReturnToOriginApplicator.h"

namespace tax
{

ReturnToOriginRule::ReturnToOriginRule(type::RtnToOrig const& rtnToOrig) : _rtnToOrig(rtnToOrig) {}

ReturnToOriginRule::~ReturnToOriginRule() {}

ReturnToOriginRule::ApplicatorType
ReturnToOriginRule::createApplicator(type::Index const& itinIndex,
                                     const Request& request,
                                     Services& /* services */,
                                     RawPayments& /*itinPayments*/) const
{
  Itin const& itin = request.getItinByIndex(itinIndex);
  type::Index const& geoPathRefId = itin.geoPathRefId();
  return ApplicatorType(this, request.geoPaths()[geoPathRefId], _rtnToOrig);
}

std::string
ReturnToOriginRule::getDescription(Services&) const
{
  if (_rtnToOrig == type::RtnToOrig::ReturnToOrigin)
    return "THE JOURNEY HAS TO RETURN TO ORIGIN";
  else //(_rtnToOrig == type::RtnToOrig::NotReturnToOrigin)
    return "THE JOURNEY CANNOT RETURN TO ORIGIN";
}
}
