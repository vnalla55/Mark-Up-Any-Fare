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


#include "Rules/PointOverlappingForYqYrApplicator.h"

namespace tax
{
PointOverlappingForYqYrApplicator::PointOverlappingForYqYrApplicator(const BusinessRule* rule,
                                                       RawPayments& payments)
  : BusinessRuleApplicator(rule), _payments(payments)
{
}

bool
PointOverlappingForYqYrApplicator::apply(PaymentDetail& paymentDetail) const
{
  const TaxName& taxName = paymentDetail.taxName();
  const Geo& taxPointBegin = paymentDetail.getTaxPointBegin();

  applyOnYqYrs(taxName, taxPointBegin, paymentDetail.getMutableYqYrDetails());
  return !paymentDetail.isFailed();
}

void
PointOverlappingForYqYrApplicator::applyOnYqYrs(const TaxName& taxName,
                                                const Geo& taxPoint,
                                                TaxableYqYrs& taxableYqYrs) const
{
  if (!taxableYqYrs.areAllFailed())
  {
    const type::Index taxPointId = taxPoint.id();
    for (type::Index i = _payments.size() - 1; i > 0; --i)
    {
      const RawPayments::value_type& rawPayment = _payments[i - 1];
      if (*rawPayment.taxName != taxName)
        break;

      if (taxPointId != rawPayment.detail.getTaxPointBegin().id())
        continue;

      const TaxableYqYrs& refYqYrs = rawPayment.detail.getYqYrDetails();
      type::Index refId = 0;
      for (type::Index i = 0; i < taxableYqYrs._subject.size(); ++i)
      {
        while (refId < refYqYrs._ids.size() && refYqYrs._ids[refId] < taxableYqYrs._ids[i])
          ++refId;

        if (refId == refYqYrs._ids.size())
          break;

        if (taxableYqYrs.isFailedRule(i) || refYqYrs.isFailedRule(refId) ||
            refYqYrs._ids[refId] > taxableYqYrs._ids[i])
          continue;

        // at this point refYqYrs._ids[refId] == taxableYqYrs._ids[i],
        // so taxableYqYr under refId in refYqYrs and i in taxableYqYrs
        // reffer to the same YqYr usage in this itin
        taxableYqYrs.setFailedRule(i, *getBusinessRule());
      }
    }
  }
}

} /* namespace tax */

