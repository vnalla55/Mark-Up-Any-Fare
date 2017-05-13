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

#include <vector>

#include "Common/ProrateCalculator.h"
#include "DataModel/Common/Types.h"
#include "Rules/BusinessRuleApplicator.h"

namespace tax
{
class MileageGetter;
class PaymentDetail;
class GeoPath;
class ServiceBaggage;
class YqYrAmountRule;

class YqYrAmountApplicator : public BusinessRuleApplicator
{
public:
  YqYrAmountApplicator(const YqYrAmountRule& rule,
                       const MileageGetter& mileageGetter,
                       const GeoPath& geoPath);

  bool apply(PaymentDetail& paymentDetail) const;

private:
  const YqYrAmountRule& _rule;

  const GeoPath& _geoPath;
  ProrateCalculator _prorateCalculator;
};

} // namespace tax
