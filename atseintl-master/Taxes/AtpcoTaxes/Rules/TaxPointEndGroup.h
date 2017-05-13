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

#include <cassert>

#include "Rules/RulesGroupApplyFunctor.h"

#include "Rules/TaxPointLoc2CompareRule.h"
#include "Rules/TaxPointLoc2InternationalDomesticRule.h"
#include "Rules/TaxPointLoc2Rule.h"
#include "Rules/TaxPointLoc2StopoverTagRule.h"
#include "Rules/TaxPointLoc3AsNextStopoverRule.h"
#include "Rules/TaxPointLoc3AsPreviousPointRule.h"

namespace tax
{

struct TaxPointEndGroup
{
  boost::optional<TaxPointLoc2StopoverTagRule> _taxPointLoc2StopoverTagRule;
  boost::optional<TaxPointLoc2Rule> _taxPointLoc2Rule;
  boost::optional<TaxPointLoc2InternationalDomesticRule> _taxPointLoc2InternationalDomesticRule;
  boost::optional<TaxPointLoc2CompareRule> _taxPointLoc2CompareRule;
  boost::optional<TaxPointLoc3AsNextStopoverRule> _taxPointLoc3AsNextStopoverRule;
  boost::optional<TaxPointLoc3AsPreviousPointRule> _taxPointLoc3AsPreviousPointRule;

  template <template <class> class Functor, class ...Args>
  bool foreach(Args&&... args) const
  {
    // There can be only one...
    assert(!_taxPointLoc2StopoverTagRule || !_taxPointLoc3AsNextStopoverRule);

    return apply<TaxPointLoc2StopoverTagRule, Functor>(
               _taxPointLoc2StopoverTagRule, std::forward<Args>(args)...) &&
           apply<TaxPointLoc3AsNextStopoverRule, Functor>(_taxPointLoc3AsNextStopoverRule,
                                                          std::forward<Args>(args)...) &&
           apply<TaxPointLoc3AsPreviousPointRule, Functor>(_taxPointLoc3AsPreviousPointRule,
                                                           std::forward<Args>(args)...) &&
           apply<TaxPointLoc2Rule, Functor>(
               _taxPointLoc2Rule, std::forward<Args>(args)...) &&
           apply<TaxPointLoc2InternationalDomesticRule, Functor>(
               _taxPointLoc2InternationalDomesticRule,
               std::forward<Args>(args)...) &&
           apply<TaxPointLoc2CompareRule, Functor>(
               _taxPointLoc2CompareRule, std::forward<Args>(args)...);
  }
};
}

