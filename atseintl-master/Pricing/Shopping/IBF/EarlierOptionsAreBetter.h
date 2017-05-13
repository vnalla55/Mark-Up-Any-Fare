
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

#include "Common/Assert.h"
#include "Pricing/Shopping/Swapper/BasicAppraiserScore.h"
#include "Pricing/Shopping/Swapper/IAppraiser.h"

#include <boost/utility.hpp>

namespace tse
{

// Since priority queue generates cheaper options first,
// we promote earlier options
class EarlierOptionsAreBetter
    : public swp::IAppraiser<utils::SopCombination, swp::BasicAppraiserScore>,
      boost::noncopyable
{
public:
  swp::BasicAppraiserScore
  beforeItemAdded(const utils::SopCombination& item, ImplIScoreBlackboard& blackboard) override
  {
    const int previous = _optionsCounter;
    _optionsCounter += 1;
    // int overflow blocker :P
    TSE_ASSERT(_optionsCounter > previous);

    // The later an option came, the less is the minor rank
    // We mark them everything as NICE_TO_HAVE since not ignored
    // and not necessary for this appraiser at the same time
    return swp::BasicAppraiserScore(swp::BasicAppraiserScore::NICE_TO_HAVE, -_optionsCounter);
  }

  void
  beforeItemRemoved(const utils::SopCombination& item, ImplIScoreBlackboard& blackboard) override
  {
  }

  bool isSatisfied() const override
  {
    // Always satisfied - does not collect, only rates
    return true;
  }

  std::string toString() const override
  {
    return "Earlier options preferred";
  }

private:
  // Each time a new option is generated by the IS queue, this counter
  // is incremented
  int _optionsCounter = 0;
};
} // namespace tse
