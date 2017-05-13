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

#include "Rules/RulesGroupApplyFunctor.h"

#include "Rules/OptionalServicePointOfDeliveryRule.h"
#include "Rules/OptionalServiceTagsRule.h"

namespace tax
{

struct OptionalServiceGroup
{
  boost::optional<OptionalServiceTagsRule> _optionalServiceTagsRule;
  boost::optional<OptionalServicePointOfDeliveryRule> _optionalServicePointOfDeliveryRule;

  template <template <class> class Functor, class ...Args>
  bool foreach(Args&&... args) const
  {
    return apply<OptionalServiceTagsRule, Functor>(_optionalServiceTagsRule,
                                                   std::forward<Args>(args)...) &&
           apply<OptionalServicePointOfDeliveryRule, Functor>(_optionalServicePointOfDeliveryRule,
                                                              std::forward<Args>(args)...);
  }
};
}


