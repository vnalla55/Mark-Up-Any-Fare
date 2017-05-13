//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2014
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


#include "Pricing/Shopping/IBF/SopCountingAllSopsRepresented.h"

#include <sstream>

namespace tse
{

using namespace swp;

BasicAppraiserScore
SopCountingAllSopsRepresented::beforeItemAdded(const utils::SopCombination& item,
                                    ImplIScoreBlackboard& blackboard)
{
  // We use both tracker to have a general idea how much we covered
  // and the appraiser to calculate detailed scores for options,
  // looking if an option has any unique SOP or not.
  _tracker.combinationAdded(item);
  return _appraiser.beforeItemAdded(item, blackboard);
}

void
SopCountingAllSopsRepresented::beforeItemRemoved(const utils::SopCombination& item,
                                      ImplIScoreBlackboard& blackboard)
{
  _appraiser.beforeItemRemoved(item, blackboard);
  _tracker.combinationRemoved(item);
}

bool
SopCountingAllSopsRepresented::isSatisfied() const
{
  return _tracker.getNbrOfUnusedSops() == 0;
}

std::string
SopCountingAllSopsRepresented::toString() const
{
  std::ostringstream out;
  out << "Sop counting all SOPs represented (" << (_tracker.getNbrOfSops() - _tracker.getNbrOfUnusedSops())
      << "/" << _tracker.getNbrOfSops() << " SOPs represented)";
  return out.str();
}

} // namespace tse
