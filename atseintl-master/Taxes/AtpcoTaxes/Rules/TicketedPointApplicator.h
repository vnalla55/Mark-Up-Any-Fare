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

namespace tax
{

class BusinessRule;
class PaymentDetail;
class GeoPath;

class TicketedPointApplicator : public BusinessRuleApplicator
{
public:
  TicketedPointApplicator(BusinessRule const* parent, GeoPath const& geoPath);
  ~TicketedPointApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  GeoPath const& _geoPath;
};
}
