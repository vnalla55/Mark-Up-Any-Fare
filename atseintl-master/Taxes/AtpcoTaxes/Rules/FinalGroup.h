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

#include "Rules/TaxCodeConversionRule.h"

#include <utility>

namespace tax
{

struct FinalGroup
{
  boost::optional<TaxCodeConversionRule> _taxCodeConversionRule;

  template <template <class> class Functor, class ...Args>
  bool foreach(Args&&... args) const
  {
    return apply<TaxCodeConversionRule, Functor>(
        _taxCodeConversionRule, std::forward<Args>(args)...);
  }
};
}

