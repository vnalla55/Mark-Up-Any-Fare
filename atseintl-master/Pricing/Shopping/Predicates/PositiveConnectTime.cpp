//-------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "Pricing/Shopping/Predicates/PositiveConnectTime.h"

#include "Common/Logger.h"
#include "Common/ShoppingUtil.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/Utils/ShoppingUtils.h"

namespace tse
{

namespace utils
{

namespace
{
Logger
logger("atseintl.Pricing.ShoppingUtils.PositiveConnectTime");
}

bool
PositiveConnectTime::
operator()(const SopCombination& sopIds)
{
  const bool answer = ShoppingUtil::checkPositiveConnectionTime(sopIds, _trx.legs());
  LOG4CXX_DEBUG(logger, "Returning " << answer << " for combination: " << sopIds);
  return answer;
}

} // namespace utils

} // namespace tse
