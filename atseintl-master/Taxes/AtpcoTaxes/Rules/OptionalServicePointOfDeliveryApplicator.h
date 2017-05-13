// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include "DataModel/Common/Types.h"
#include "Rules/BusinessRuleApplicator.h"
#include "DomainDataObjects/OptionalService.h"
#include "DomainDataObjects/OptionalServicePath.h"
#include "DomainDataObjects/GeoPathMapping.h"

namespace tax
{

class OptionalServicePointOfDeliveryRule;
class PaymentDetail;
class LocService;

class OptionalServicePointOfDeliveryApplicator : public BusinessRuleApplicator
{
public:
  OptionalServicePointOfDeliveryApplicator(OptionalServicePointOfDeliveryRule const& parent,
                                           LocService const& locService);

  ~OptionalServicePointOfDeliveryApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  OptionalServicePointOfDeliveryRule const& _optionalServicePointOfDeliveryRule;

  LocService const& _locService;
};
}
