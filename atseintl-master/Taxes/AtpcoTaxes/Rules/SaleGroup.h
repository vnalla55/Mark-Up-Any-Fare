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
#include "Rules/PointOfSaleRule.h"

#include <utility>

namespace tax
{

struct SaleGroup
{
  boost::optional<PointOfSaleRule> _pointOfSaleRule;

  template <template <class> class Functor, class ...Args>
  bool foreach(Args&&... args) const
  {
    return apply<PointOfSaleRule, Functor>(_pointOfSaleRule, std::forward<Args>(args)...);
  }
};
}

