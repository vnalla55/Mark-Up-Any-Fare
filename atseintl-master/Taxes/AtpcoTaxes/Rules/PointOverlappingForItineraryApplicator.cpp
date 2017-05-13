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


#include "Rules/PointOverlappingForItineraryApplicator.h"

namespace tax
{
PointOverlappingForItineraryApplicator::PointOverlappingForItineraryApplicator(const BusinessRule* rule,
                                                       RawPayments& payments)
  : BusinessRuleApplicator(rule), _payments(payments)
{
}

bool
PointOverlappingForItineraryApplicator::apply(PaymentDetail& paymentDetail) const
{
  const TaxName& taxName = paymentDetail.taxName();
  const Geo& taxPointBegin = paymentDetail.getTaxPointBegin();

  applyOnItinerary(taxName, taxPointBegin, paymentDetail.getMutableItineraryDetail());
  return !paymentDetail.isFailed();
}

void
PointOverlappingForItineraryApplicator::applyOnItinerary(const TaxName& taxName,
                                             const Geo& taxPoint,
                                             TaxableItinerary& taxableItinerary) const
{
  const type::Index taxPointId = taxPoint.id();
  for (type::Index i = _payments.size() - 1; i > 0; --i)
  {
    const RawPayments::value_type& rawPayment = _payments[i - 1];

    if (!rawPayment.detail.isFailedRule() &&
        taxPointId == rawPayment.detail.getTaxPointBegin().id() &&
        *rawPayment.taxName == taxName)
    {
      taxableItinerary.setFailedRule(getBusinessRule());
      return;
    }

    if (*rawPayment.taxName != taxName)
      break;
  }
}

} /* namespace tax */

