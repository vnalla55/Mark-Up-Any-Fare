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

#include "Rules/ServiceFeeSecurityApplicatorFacade.h"

#include "DataModel/Common/CodeIO.h"
#include "Rules/DummyApplicator.h"
#include "Rules/ServiceFeeSecurityApplicator.h"
#include "Rules/ServiceFeeSecurityRule.h"
#include "ServiceInterfaces/LocService.h"
#include "ServiceInterfaces/ServiceFeeSecurityService.h"

namespace tax
{
ServiceFeeSecurityApplicatorFacade::ServiceFeeSecurityApplicatorFacade(
    const ServiceFeeSecurityRule* rule,
    const PointOfSale& pointOfSale,
    const LocService& locService,
    const std::shared_ptr<const ServiceFeeSecurityItems>& sfs)
  : BusinessRuleApplicator(rule),
    _serviceFeeSecurityRule(rule),
    _pointOfSale(pointOfSale),
    _locService(locService),
    _serviceFeeSecurity(sfs)
{
}

bool
ServiceFeeSecurityApplicatorFacade::apply(PaymentDetail& paymentDetail) const
{
  if (_serviceFeeSecurity->empty())
  {
    std::ostringstream buf;
    buf << "Service fee security: vendor " << _serviceFeeSecurityRule->getVendor() << ", item "
        << _serviceFeeSecurityRule->getItemNo() << " not found in services!";
    return DummyApplicator(*_serviceFeeSecurityRule, false, buf.str()).apply(paymentDetail);
  }

  return ServiceFeeSecurityApplicator(
             _serviceFeeSecurityRule, _pointOfSale, _locService, _serviceFeeSecurity)
      .apply(paymentDetail);
}
}

