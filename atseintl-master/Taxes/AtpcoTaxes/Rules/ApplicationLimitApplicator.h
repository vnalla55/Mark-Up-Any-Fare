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

#pragma once

#include "DomainDataObjects/FlightUsage.h"
#include "Rules/BusinessRuleApplicator.h"

namespace tax
{
class ApplicationLimitRule;
class BusinessRule;
class GeoPath;
class PaymentDetail;

class ApplicationLimitApplicator : public BusinessRuleApplicator
{
public:
  ApplicationLimitApplicator(const ApplicationLimitRule&, const GeoPath&,
                             const std::vector<FlightUsage>&);

  bool apply(PaymentDetail&) const;
private:
  const ApplicationLimitRule& _rule;
  const GeoPath& _geoPath;
  const std::vector<FlightUsage>& _flightUsages;
};

} /* namespace tax */

