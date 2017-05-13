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

#include "Rules/JourneyLoc1AsOriginApplicator.h"
#include "DomainDataObjects/Geo.h"
#include "ServiceInterfaces/LocService.h"
#include "Rules/JourneyLoc1AsOriginRule.h"
#include "Rules/PaymentDetail.h"

namespace tax
{

JourneyLoc1AsOriginApplicator::JourneyLoc1AsOriginApplicator(const JourneyLoc1AsOriginRule& rule,
                                                             const Geo& startGeo,
                                                             const LocService& locService)
  : BusinessRuleApplicator(&rule),
    _loc1AsOriginRule(rule),
    _startGeo(startGeo),
    _locService(locService),
    _isSkipExempt(false)
{
}

bool
JourneyLoc1AsOriginApplicator::apply(PaymentDetail& paymentDetail) const
{
  paymentDetail.setSkipExempt(_isSkipExempt);
  paymentDetail.setJourneyLoc1(&_startGeo);
  return _locService.isInLoc(
      _startGeo.loc().code(), _loc1AsOriginRule.getLocZone(), _loc1AsOriginRule.getVendor());
}

} // namespace tax
