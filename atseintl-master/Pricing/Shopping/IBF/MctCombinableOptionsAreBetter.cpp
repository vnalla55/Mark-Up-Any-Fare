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

#include "Pricing/Shopping/IBF/MctCombinableOptionsAreBetter.h"

#include "Common/ShoppingUtil.h"
#include "DataModel/ShoppingTrx.h"

namespace tse
{

swp::BasicAppraiserScore
MctCombinableOptionsAreBetter::beforeItemAdded(const utils::SopCombination& item,
                                               ImplIScoreBlackboard& blackboard)
{
  const bool answer = ShoppingUtil::checkMinConnectionTime(_trx.getOptions(), item, _trx.legs());

  // We mark everything as NICE_TO_HAVE since not ignored
  // and not necessary for this appraiser at the same time.
  return swp::BasicAppraiserScore(swp::BasicAppraiserScore::NICE_TO_HAVE, static_cast<int>(answer));
}

} // namespace tse
