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
#include "Rules/ExemptTagApplicator.h"
#include "Rules/ExemptTagRule.h"
#include "Rules/PaymentDetail.h"
#include "ServiceInterfaces/FallbackService.h"

namespace tse
{
class Trx;
ATPCO_FALLBACK_DECL(markupAnyFareOptimization)
}

namespace tax
{

ExemptTagApplicator::ExemptTagApplicator(ExemptTagRule const& parent, const Services& services)
  : BusinessRuleApplicator(&parent),
    _services(services)
{
}

bool
ExemptTagApplicator::apply(PaymentDetail& paymentDetail) const
{
  paymentDetail.taxEquivalentAmount() = 0;

  if (!_services.fallbackService().isSet(tse::fallback::markupAnyFareOptimization))
  {
    paymentDetail.taxEquivalentWithMarkupAmount() = 0;
  }

  paymentDetail.setExempt();
  return true;
}
}
