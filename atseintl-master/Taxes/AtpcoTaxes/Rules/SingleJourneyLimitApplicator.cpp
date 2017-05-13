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
#include <vector>


#include "SingleJourneyLimitApplicator.h"
#include "DomainDataObjects/GeoPath.h"
#include "ServiceInterfaces/LocService.h"
#include "PaymentDetail.h"

namespace tax
{

SingleJourneyLimitApplicator::SingleJourneyLimitApplicator(const BusinessRule* parent,
                                                           const GeoPath& geoPath,
                                                           const LocService& /*locService*/,
                                                           RawPayments& rawPayments)
  : BusinessRuleApplicator(parent), _geoPath(geoPath), _rawPayments(rawPayments)
{
}

SingleJourneyLimitApplicator::~SingleJourneyLimitApplicator() {}

bool
SingleJourneyLimitApplicator::apply(PaymentDetail& paymentDetail) const
{
  paymentDetail.taxApplicationLimit() = type::TaxApplicationLimit::OncePerSingleJourney;
  return applyLimit(paymentDetail, 1);
}

bool
SingleJourneyLimitApplicator::applyLimit(PaymentDetail& paymentDetail, uint32_t limit) const
{
  std::vector<Trip> trips;
  SpecialTrips::findSingleJourneys(paymentDetail, _geoPath, trips);

  uint32_t collectedTaxCount = 0;
  PaymentDetail* paymentDetailFound = nullptr;
  bool onePerItinLimitFound = false;

  for(RawPayments::value_type& rawPayment : _rawPayments)
  {
    if (rawPayment.detail.isCalculated() && !rawPayment.detail.getItineraryDetail().isFailedRule() &&
        paymentDetail.taxName() == *rawPayment.taxName &&
        SpecialTrips::isGeoInTrip(
            trips, rawPayment.detail.getTaxPointBegin(), paymentDetail.getTaxPointBegin()))
    {
      ++collectedTaxCount;
      paymentDetailFound = &rawPayment.detail;
      if (rawPayment.detail.taxApplicationLimit() == type::TaxApplicationLimit::OnceForItin)
      {
        onePerItinLimitFound = true;
      }
    }
  }

  if (!onePerItinLimitFound)
  {
    return collectedTaxCount < limit;
  }

  if (paymentDetailFound->taxAmt() >= paymentDetail.taxAmt())
  {
    return false;
  }
  else
  {
    paymentDetailFound->getMutableItineraryDetail().setFailedRule(getBusinessRule());
    return true;
  }
}
}
