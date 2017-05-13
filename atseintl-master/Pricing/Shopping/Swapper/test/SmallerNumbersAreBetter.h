
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

#ifndef SMALLER_NUMBERS_ARE_BETTER_H
#define SMALLER_NUMBERS_ARE_BETTER_H

#include "Pricing/Shopping/Swapper/IAppraiser.h"
#include "Pricing/Shopping/Swapper/BasicAppraiserScore.h"

namespace tse
{

namespace swp
{

// This appraiser does not have to collect anything
// it only rates the numbers to provide additional
// sorting criteria for items
class SmallerNumbersAreBetter : public IAppraiser<int, BasicAppraiserScore>
{
public:
  BasicAppraiserScore
  beforeItemAdded(const int& item, IMapUpdater<int, BasicAppraiserScore>& blackboard) override
  {
    return BasicAppraiserScore(BasicAppraiserScore::NICE_TO_HAVE, -item);
  }

  void
  beforeItemRemoved(const int& item, IMapUpdater<int, BasicAppraiserScore>& blackboard) override
  {
  }

  bool isSatisfied() const override
  {
    // Always satisfied since does not collect anything
    return true;
  }

  std::string toString() const override
  {
    return "Smaller numbers are better";
  }
};

} // namespace swp

} // namespace tse

#endif // SMALLER_NUMBERS_ARE_BETTER_H
