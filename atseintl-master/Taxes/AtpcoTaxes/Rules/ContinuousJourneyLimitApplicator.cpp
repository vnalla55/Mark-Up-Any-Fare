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
#include "ContinuousJourneyApplicator.h"
#include "ContinuousJourneyLimitApplicator.h"
#include "DomainDataObjects/GeoPath.h"
#include "ServiceInterfaces/LocService.h"
#include "PaymentDetail.h"
#include "SpecialTrips.h"


namespace tax
{

ContinuousJourneyLimitApplicator::ContinuousJourneyLimitApplicator(
    const ContinuousJourneyLimitRule& parent,
    const GeoPath& geoPath,
    const LocService& /*locService*/,
    const RawPayments& rawPayments)
  : BusinessRuleApplicator(&parent), _geoPath(geoPath), _rawPayments(rawPayments)
{
}

ContinuousJourneyLimitApplicator::~ContinuousJourneyLimitApplicator() {}

bool
ContinuousJourneyLimitApplicator::apply(PaymentDetail& paymentDetail) const
{
  paymentDetail.taxApplicationLimit() = type::TaxApplicationLimit::FirstTwoPerContinuousJourney;
  return applyLimit(paymentDetail, 2);
}

bool
ContinuousJourneyLimitApplicator::applyLimit(PaymentDetail& paymentDetail, uint32_t limit) const
{
  std::vector<Trip> journeys;
  SpecialTrips::findContinuousJourneys(paymentDetail, _geoPath, journeys);
  uint32_t collectedTaxCount = 0;

  for(const RawPayments::value_type& rawPayment : _rawPayments)
  {
    if (rawPayment.detail.isCalculated() && !rawPayment.detail.getItineraryDetail().isFailedRule() &&
        paymentDetail.taxName() == *rawPayment.taxName &&
        SpecialTrips::isGeoInTrip(
            journeys, rawPayment.detail.getTaxPointBegin(), paymentDetail.getTaxPointBegin()))
    {
      ++collectedTaxCount;
    }
  }

  return collectedTaxCount < limit;
}
}
