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
#include "Rules/ContinuousJourneyRule.h"
#include "SpecialTrips.h"

namespace tax
{

class GeoPath;
class PaymentDetail;

class ContinuousJourneyApplicator : public BusinessRuleApplicator
{
public:
  ContinuousJourneyApplicator(ContinuousJourneyRule const& parent, GeoPath const& geoPath);

  ~ContinuousJourneyApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  GeoPath const& _geoPath;
  ContinuousJourneyRule const& _continuousJourneyRule;
};

} // namespace tax
