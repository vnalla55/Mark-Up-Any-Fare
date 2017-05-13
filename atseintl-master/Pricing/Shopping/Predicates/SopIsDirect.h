//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Pricing/Shopping/FiltersAndPipes/IPredicate.h"
#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h"

#include <boost/utility.hpp>

namespace tse
{

namespace utils
{

// Checks the requirement "cabin class validity"
// for a single SOP
class SopIsDirect : public IPredicate<SopCandidate>, boost::noncopyable
{
public:
  bool operator()(const SopCandidate& candid) override
  {
    return candid.isFlightDirect;
  }
};

} // namespace utils

} // namespace tse

