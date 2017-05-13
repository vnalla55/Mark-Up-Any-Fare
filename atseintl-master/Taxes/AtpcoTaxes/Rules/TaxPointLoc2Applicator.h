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
#include "DomainDataObjects/GeoPath.h"
#include "Rules/BusinessRuleApplicator.h"
#include "Rules/TaxPointLoc2Rule.h"

namespace tax
{

class BusinessRule;
class LocService;
class PaymentDetail;

class TaxPointLoc2Applicator : public BusinessRuleApplicator
{
public:
  TaxPointLoc2Applicator(const TaxPointLoc2Rule& rule,
                         const LocService& locService);
  ~TaxPointLoc2Applicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  const TaxPointLoc2Rule& _rule;

  const LocService& _locService;
};

} // namespace tax
