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
#pragma once

#include "Rules/BusinessRuleApplicator.h"
#include "DataModel/Common/Types.h"

namespace tax
{

class PointOfTicketingRule;
class PaymentDetail;
class LocService;

class PointOfTicketingApplicator : public BusinessRuleApplicator
{
public:
  PointOfTicketingApplicator(const PointOfTicketingRule& rule,
                             const type::AirportCode& ticketingPoint,
                             const LocService& locService);

  ~PointOfTicketingApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  const PointOfTicketingRule& _pointOfTicketingRule;
  const type::AirportCode& _ticketingPoint;
  const LocService& _locService;
};
}

