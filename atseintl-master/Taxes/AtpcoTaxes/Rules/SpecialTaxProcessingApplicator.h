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
#include "DataModel/Common/Types.h"

namespace tax
{
class PaymentDetail;

class SpecialTaxProcessingApplicator : public BusinessRuleApplicator
{
public:
  SpecialTaxProcessingApplicator(const BusinessRule& parent, bool tch)
    : BusinessRuleApplicator(&parent), _tch(tch)
  {
  }

  bool apply(PaymentDetail&) const { return _tch; }

private:
  bool _tch;
};

} /* namespace tax */

