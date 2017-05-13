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

namespace tax
{

class BusinessRule;
class PaymentDetail;
class Geo;
class GeoPath;
class LocService;
class JourneyIncludesRule;

class JourneyIncludesApplicator : public BusinessRuleApplicator
{
public:
  JourneyIncludesApplicator(const JourneyIncludesRule& rule,
                            const GeoPath& geoPath,
                            const LocService& locService);
  ~JourneyIncludesApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  bool doesJourneyTicketedPointTagMatch(const Geo& geo) const;
  bool doesJourneyInclude(const Geo& geo) const;
  const JourneyIncludesRule& _journeyIncludesRule;

  const GeoPath& _geoPath;
  const LocService& _locService;
};

} // namespace tax
