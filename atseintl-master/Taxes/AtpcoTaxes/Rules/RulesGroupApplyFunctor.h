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
#include "Rules/RawPayments.h"

#include <boost/optional.hpp>

#include <utility>

namespace tax
{

template <typename Rule, template <class> class Functor, class ...Args>
bool
apply(const boost::optional<Rule>& rule, Args&&... args)
{
  if (rule)
    return Functor<Rule>::apply(*rule, std::forward<Args>(args)...);

  return true;
}
} // namespace tax

