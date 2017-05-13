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

#include "DataModel/Common/Types.h"
#include "Rules/BusinessRuleApplicator.h"

namespace tax
{

class OptionalServiceTagsRule;
class PaymentDetail;

class OptionalServiceTagsApplicator : public BusinessRuleApplicator
{
public:
  OptionalServiceTagsApplicator(OptionalServiceTagsRule const& parent);

  ~OptionalServiceTagsApplicator();

  bool apply(PaymentDetail& paymentDetail) const;

private:
  bool serviceMatchesTaxableUnitTag(type::OptionalServiceTag const& optionalServiceTag) const;

  OptionalServiceTagsRule const& _optionalServiceTagsRule;
};
}

