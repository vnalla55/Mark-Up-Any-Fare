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
#include "Rules/OptionalServicePointOfDeliveryApplicator.h"
#include "Rules/OptionalServicePointOfDeliveryRule.h"
#include "Rules/PaymentDetail.h"
#include "ServiceInterfaces/LocService.h"
#include "DataModel/Common/Types.h"

namespace tax
{

OptionalServicePointOfDeliveryApplicator::OptionalServicePointOfDeliveryApplicator(
    OptionalServicePointOfDeliveryRule const& parent, LocService const& locService)
  : BusinessRuleApplicator(&parent),
    _optionalServicePointOfDeliveryRule(parent),
    _locService(locService)
{
}

OptionalServicePointOfDeliveryApplicator::~OptionalServicePointOfDeliveryApplicator() {}

bool
OptionalServicePointOfDeliveryApplicator::apply(PaymentDetail& paymentDetail) const
{
  for(OptionalService & optionalService : paymentDetail.optionalServiceItems())
  {
    if (!optionalService.isFailed() &&
        !_locService.isInLoc(optionalService.pointOfDeliveryLoc(),
                           _optionalServicePointOfDeliveryRule.getLocZone(),
                           _optionalServicePointOfDeliveryRule.getVendor()))
    {
      optionalService.setFailedRule(&_optionalServicePointOfDeliveryRule);
    }
  }

  return true;
}
}
