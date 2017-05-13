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
#include "Rules/BusinessRuleApplicator.h"
#include "DomainDataObjects/GeoPath.h"

namespace tax
{

class TaxPointLoc2CompareRule;
class PaymentDetail;

class TaxPointLoc2CompareApplicator : public BusinessRuleApplicator
{
public:
  TaxPointLoc2CompareApplicator(const TaxPointLoc2CompareRule& rule);
  ~TaxPointLoc2CompareApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  const TaxPointLoc2CompareRule& _taxPointLoc2CompareRule;
};

} // namespace tax
