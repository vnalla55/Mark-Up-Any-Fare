
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

#ifndef TAKE_ALL_NUMBERS_H
#define TAKE_ALL_NUMBERS_H

#include "Pricing/Shopping/Swapper/IAppraiser.h"
#include "Pricing/Shopping/Swapper/BasicAppraiserScore.h"

namespace tse
{

namespace swp
{

class TakeAllNumbers : public IAppraiser<int, BasicAppraiserScore>
{
public:
  BasicAppraiserScore
  beforeItemAdded(const int& item, IMapUpdater<int, BasicAppraiserScore>& blackboard) override
  {
    return BasicAppraiserScore(BasicAppraiserScore::MUST_HAVE);
  }

  void
  beforeItemRemoved(const int& item, IMapUpdater<int, BasicAppraiserScore>& blackboard) override
  { 
  }

  bool isSatisfied() const override
  {
    // Always satisfied
    return true;
  }

  std::string toString() const override
  {
    return "Take all numbers";
  }
};

} // namespace swp

} // namespace tse

#endif // TAKE_ALL_NUMBERS_H
