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
#include "BusinessRuleApplicator.h"
#include "ContinuousJourneyLimitRule.h"
#include "DomainDataObjects/GeoPath.h"

#include <vector>

namespace tax
{

class ContinuousJourneyLimitRule;
class LocService;
class PaymentDetail;

class ContinuousJourneyLimitApplicator : public BusinessRuleApplicator
{
  typedef std::pair<type::Index, type::Index> Trip;

public:
  ContinuousJourneyLimitApplicator(ContinuousJourneyLimitRule const& parent,
                                   const GeoPath& geoPath,
                                   const LocService& locService,
                                   const RawPayments& rawPayments);

  ~ContinuousJourneyLimitApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  const GeoPath& _geoPath;
  const RawPayments& _rawPayments;

  bool applyLimit(PaymentDetail& paymentDetail, uint32_t limit) const;
  bool isGeoInTrip(const std::vector<Trip>& trips, const Geo& tripGeo, const Geo& currentGeo) const;
};

} // namespace tax

