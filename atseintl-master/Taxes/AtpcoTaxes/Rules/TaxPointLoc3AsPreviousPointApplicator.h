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

#include "DataModel/Common/Types.h"
#include "Rules/BusinessRuleApplicator.h"

namespace tax
{

class LocZone;
class LocService;
class PaymentDetail;
class TaxPointLoc3AsPreviousPointRule;

class TaxPointLoc3AsPreviousPointApplicator : public BusinessRuleApplicator
{
public:
  TaxPointLoc3AsPreviousPointApplicator(const TaxPointLoc3AsPreviousPointRule& rule,
                                        const LocZone& locZone,
                                        const type::Vendor& vendor,
                                        const LocService& locService);
  ~TaxPointLoc3AsPreviousPointApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  const LocZone& _locZone;
  const type::Vendor& _vendor;
  const LocService& _locService;
  const TaxPointLoc3AsPreviousPointRule& _taxPointLoc3AsPreviousPointRule;
};

} // namespace tax
