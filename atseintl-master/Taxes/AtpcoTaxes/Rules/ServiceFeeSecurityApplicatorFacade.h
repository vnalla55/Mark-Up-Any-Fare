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

#include "DataModel/Services/ServiceFeeSecurity.h"
#include "Rules/BusinessRuleApplicator.h"

#include <memory>

namespace tax
{
class LocService;
class PointOfSale;
class PaymentDetail;
class ServiceFeeSecurityRule;

class ServiceFeeSecurityApplicatorFacade : public BusinessRuleApplicator
{
public:
  ServiceFeeSecurityApplicatorFacade(const ServiceFeeSecurityRule*,
                                     const PointOfSale&,
                                     const LocService&,
                                     const std::shared_ptr<const ServiceFeeSecurityItems>&);

  bool apply(PaymentDetail& paymentDetail) const;

private:
  const ServiceFeeSecurityRule* _serviceFeeSecurityRule;
  const PointOfSale& _pointOfSale;
  const LocService& _locService;
  std::shared_ptr<const ServiceFeeSecurityItems> _serviceFeeSecurity;
};
}

