
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

#ifndef ALL_EVEN_NUMBERS_REPRESENTED_H
#define ALL_EVEN_NUMBERS_REPRESENTED_H

#include "Pricing/Shopping/Swapper/IAppraiser.h"
#include "Pricing/Shopping/Swapper/BasicAppraiserScore.h"

namespace tse
{

namespace swp
{

class AllEvenNumbersRepresented : public IAppraiser<int, BasicAppraiserScore>
{
public:
  BasicAppraiserScore
  beforeItemAdded(const int& item, IMapUpdater<int, BasicAppraiserScore>& blackboard) override
  {
    if ((item % 2) == 0)
    {
      return BasicAppraiserScore(BasicAppraiserScore::MUST_HAVE);
    }
    return BasicAppraiserScore(BasicAppraiserScore::IGNORE);
  }

  void
  beforeItemRemoved(const int& item, IMapUpdater<int, BasicAppraiserScore>& blackboard) override
  {
  }

  bool isSatisfied() const override
  {
    // Always satisfied
    // Marks even numbers as important (sets priority for them) but
    // cannot tell if "all" have been collected
    // - just runs as long as options are collected
    return true;
  }

  std::string toString() const
  {
    return "All even numbers represented";
  }
};

} // namespace swp

} // namespace tse

#endif // ALL_EVEN_NUMBERS_REPRESENTED_H
