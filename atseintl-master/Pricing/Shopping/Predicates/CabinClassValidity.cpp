

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

#include "Pricing/Shopping/Predicates/CabinClassValidity.h"

#include "Common/Logger.h"
#include "Pricing/Shopping/Utils/ShoppingUtils.h"

namespace tse
{

namespace utils
{

namespace
{
Logger
logger("atseintl.Pricing.ShoppingUtils.CabinClassValidity");
}

bool
CabinClassValidity::
operator()(const SopCombination& sopIds)
{
  const bool answer = ShoppingUtil::isCabinClassValid(_trx, sopIds);
  LOG4CXX_DEBUG(logger, "Returning " << answer << " for combination: " << sopIds);
  return answer;
}

} // namespace utils

} // namespace tse
