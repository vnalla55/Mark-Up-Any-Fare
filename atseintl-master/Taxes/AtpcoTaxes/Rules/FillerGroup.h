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

#include "Rules/ApplicationTag01Rule.h"
#include "Rules/ConnectionsTagsRule.h"
#include "Rules/FillTimeStopoversRule.h"
#include "Rules/TaxPointLoc1Rule.h"
#include "Rules/TicketedPointRule.h"

#include <boost/optional.hpp>

#include <utility>

namespace tax
{

struct FillerGroup
{
  boost::optional<ApplicationTag01Rule> _applicationTagRule;
  boost::optional<TaxPointLoc1Rule> _taxPointLoc1Rule;
  boost::optional<FillTimeStopoversRule> _fillTimeStopoversRule;
  boost::optional<TicketedPointRule> _ticketedPointRule;
  boost::optional<ConnectionsTagsRule> _connectionsTagsRule;

  template <template <class> class Functor, class ...Args>
  bool foreach(Args&&... args) const
  {
    return apply<TaxPointLoc1Rule, Functor>(
               _taxPointLoc1Rule, std::forward<Args>(args)...) &&
           apply<TicketedPointRule, Functor>(
               _ticketedPointRule, std::forward<Args>(args)...) &&
           apply<FillTimeStopoversRule, Functor>(
               _fillTimeStopoversRule, std::forward<Args>(args)...) &&
           apply<ApplicationTag01Rule, Functor>(
               _applicationTagRule, std::forward<Args>(args)...) &&
           apply<ConnectionsTagsRule, Functor>(
               _connectionsTagsRule, std::forward<Args>(args)...);
  }
};
}

