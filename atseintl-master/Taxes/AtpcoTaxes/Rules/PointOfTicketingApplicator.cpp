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
#include "Rules/PointOfTicketingApplicator.h"
#include "Rules/PointOfTicketingRule.h"

#include "ServiceInterfaces/LocService.h"

namespace tax
{

PointOfTicketingApplicator::PointOfTicketingApplicator(const PointOfTicketingRule& rule,
                                                       const type::AirportCode& ticketingPoint,
                                                       const LocService& locService)
  : BusinessRuleApplicator(&rule),
    _pointOfTicketingRule(rule),
    _ticketingPoint(ticketingPoint),
    _locService(locService)
{
}

PointOfTicketingApplicator::~PointOfTicketingApplicator() {}

bool
PointOfTicketingApplicator::apply(PaymentDetail& /*paymentDetail*/) const
{
  if (_ticketingPoint.empty())
    return true;
  return _locService.isInLoc(
      _ticketingPoint, _pointOfTicketingRule.getLocZone(), _pointOfTicketingRule.getVendor());
}
}
