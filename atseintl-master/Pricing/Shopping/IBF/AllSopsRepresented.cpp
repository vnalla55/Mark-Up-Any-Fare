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


#include "Pricing/Shopping/IBF/AllSopsRepresented.h"

#include <sstream>

namespace tse
{

using namespace swp;

BasicAppraiserScore
AllSopsRepresented::beforeItemAdded(const utils::SopCombination& item,
                                    ImplIScoreBlackboard& blackboard)
{
  BasicAppraiserScore::CATEGORY cat = BasicAppraiserScore::IGNORE;
  const bool isNewSopCovered = _tracker.combinationAdded(item);
  if (isNewSopCovered)
  {
    cat = BasicAppraiserScore::MUST_HAVE;
  }
  return BasicAppraiserScore(cat);
}

void
AllSopsRepresented::beforeItemRemoved(const utils::SopCombination& item,
                                      ImplIScoreBlackboard& blackboard)
{
  _tracker.combinationRemoved(item);
}

bool
AllSopsRepresented::isSatisfied() const
{
  return _tracker.getNbrOfUnusedSops() == 0;
}

std::string AllSopsRepresented::toString() const
{
  std::ostringstream out;
  out << "All SOPs represented (" << (_tracker.getNbrOfSops() - _tracker.getNbrOfUnusedSops())
      << "/" << _tracker.getNbrOfSops() << " SOPs represented)";
  return out.str();
}

} // namespace tse
