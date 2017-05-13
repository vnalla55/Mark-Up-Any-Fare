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

#include "Rules/TaxPointLoc1InternationalDomesticRule.h"
#include "Rules/TaxPointLoc1StopoverTagRule.h"
#include "Rules/TaxPointLoc1TransferTypeRule.h"

#include <utility>

namespace tax
{

struct TaxPointBeginGroup
{
  boost::optional<TaxPointLoc1TransferTypeRule> _taxPointLoc1TransferTypeRule;
  boost::optional<TaxPointLoc1StopoverTagRule> _taxPointLoc1StopoverTagRule;
  boost::optional<TaxPointLoc1InternationalDomesticRule> _taxPointLoc1InternationalDomesticRule;

  template <template <class> class Functor, class ...Args>
  bool foreach(Args&&... args) const
  {
    return apply<TaxPointLoc1TransferTypeRule, Functor>(_taxPointLoc1TransferTypeRule,
                                                        std::forward<Args>(args)...) &&
           apply<TaxPointLoc1StopoverTagRule, Functor>(_taxPointLoc1StopoverTagRule,
                                                       std::forward<Args>(args)...) &&
           apply<TaxPointLoc1InternationalDomesticRule, Functor>(
               _taxPointLoc1InternationalDomesticRule,
               std::forward<Args>(args)...);
  }
};
}

