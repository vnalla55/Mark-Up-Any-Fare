// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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

#include "Rules/ApplicationLimitApplicator.h"
#include "Rules/ApplicationLimitRule.h"
#include "Rules/SpecialTrips.h"

namespace tax
{
ApplicationLimitApplicator::ApplicationLimitApplicator(const ApplicationLimitRule& rule,
                                                       const GeoPath& geoPath,
                                                       const std::vector<FlightUsage>& flightUsages)
  : BusinessRuleApplicator(&rule), _rule(rule), _geoPath(geoPath), _flightUsages(flightUsages)
{
}

bool
ApplicationLimitApplicator::apply(PaymentDetail& paymentDetail) const
{
  std::vector<Trip> journeys;
  if (_rule.taxApplicationLimit() == type::TaxApplicationLimit::FirstTwoPerUSRoundTrip)
  {
    SpecialTrips::findRoundTrips(paymentDetail, _geoPath, _flightUsages, journeys);
    if (journeys.empty())
      return false;
  }

  return true;
}

} /* namespace tax */

