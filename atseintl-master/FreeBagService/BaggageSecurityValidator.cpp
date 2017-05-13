//-------------------------------------------------------------------
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "FreeBagService/BaggageSecurityValidator.h"

#include "DataModel/Agent.h"
#include "DataModel/PricingTrx.h"

namespace tse
{
BaggageSecurityValidator::BaggageSecurityValidator(
    PricingTrx& trx,
    const std::vector<TravelSeg*>::const_iterator segI,
    const std::vector<TravelSeg*>::const_iterator segIE,
    bool isCollectingT183Info)
  : ExtendedSecurityValidator(trx, segI, segIE), _isCollectingT183Info(isCollectingT183Info)
{
}

BaggageSecurityValidator::~BaggageSecurityValidator() {}

const Loc*
BaggageSecurityValidator::getLocation() const
{
  return _trx.getRequest()->ticketingAgent()->agentLocation();
}

bool
BaggageSecurityValidator::shouldCreateDiag() const
{
  return _isCollectingT183Info;
}

bool
BaggageSecurityValidator::shouldCollectDiag() const
{
  return _isCollectingT183Info;
}

} // tse
