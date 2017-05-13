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

class TaxPointLoc2InternationalDomesticRule;
class PaymentDetail;

class TaxPointLoc2InternationalDomesticApplicator : public BusinessRuleApplicator
{
public:
  TaxPointLoc2InternationalDomesticApplicator(const TaxPointLoc2InternationalDomesticRule& rule);
  ~TaxPointLoc2InternationalDomesticApplicator();

  bool matches(const type::Nation& nation, const Geo& geo) const;
  bool apply(PaymentDetail& paymentDetail) const;

private:
  const TaxPointLoc2InternationalDomesticRule& _rule;
};

} // namespace tax
