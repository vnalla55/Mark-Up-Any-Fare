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
#include "ServiceInterfaces/Services.h"

namespace tax
{

class ExemptTagRule;
class PaymentDetail;

class ExemptTagApplicator : public BusinessRuleApplicator
{
public:
  ExemptTagApplicator(ExemptTagRule const& parent, const Services& services);

  bool apply(PaymentDetail& paymentDetail) const;

private:
  const Services& _services;
};
}
