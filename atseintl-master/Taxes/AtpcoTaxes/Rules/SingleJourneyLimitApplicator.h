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
#include "SingleJourneyLimitRule.h"
#include "DomainDataObjects/GeoPath.h"
#include "SpecialTrips.h"

namespace tax
{

class BusinessRule;
class LocService;
class PaymentDetail;

class SingleJourneyLimitApplicator : public BusinessRuleApplicator
{

public:
  SingleJourneyLimitApplicator(const BusinessRule* parent,
                               const GeoPath& geoPath,
                               const LocService& locService,
                               RawPayments& rawPayments);

  ~SingleJourneyLimitApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  const GeoPath& _geoPath;
  RawPayments& _rawPayments;

  bool applyLimit(PaymentDetail& paymentDetail, uint32_t limit) const;
};

} // namespace tax

