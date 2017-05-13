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

#pragma once

#include "Rules/BusinessRuleApplicator.h"
#include "Rules/BusinessRule.h"

namespace tax
{
class Geo;
class PaymentDetail;
class TaxName;

class PointOverlappingForYqYrApplicator : public BusinessRuleApplicator
{
  friend class PointOverlappingApplicatorTest;
public:
  PointOverlappingForYqYrApplicator(const BusinessRule*, RawPayments& /*itinPayments*/);

  bool apply(PaymentDetail&) const;
private:
  void applyOnYqYrs(const TaxName&, const Geo&, TaxableYqYrs&) const;
  RawPayments& _payments;
};

} /* namespace tax */

