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

class TaxPointLoc1Rule;
class LocService;
class PaymentDetail;

class TaxPointLoc1Applicator : public BusinessRuleApplicator
{
public:
  TaxPointLoc1Applicator(TaxPointLoc1Rule const& rule, LocService const& locService);
  ~TaxPointLoc1Applicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  LocService const& _locService;
  TaxPointLoc1Rule const& _taxPointLoc1Rule;
};

} // namespace tax
