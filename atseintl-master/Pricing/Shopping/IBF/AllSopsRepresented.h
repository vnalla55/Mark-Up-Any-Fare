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

#include "Pricing/Shopping/IBF/ISopUsageTracker.h"
#include "Pricing/Shopping/Swapper/BasicAppraiserScore.h"
#include "Pricing/Shopping/Swapper/IAppraiser.h"
#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h"

#include <boost/utility.hpp>

#include <sstream>

namespace tse
{

class AllSopsRepresented : public swp::IAppraiser<utils::SopCombination, swp::BasicAppraiserScore>,
                           boost::noncopyable
{
public:

  AllSopsRepresented(ISopUsageTracker& tracker): _tracker(tracker){}

  swp::BasicAppraiserScore
  beforeItemAdded(const utils::SopCombination& item, ImplIScoreBlackboard& blackboard) override;

  void beforeItemRemoved(const utils::SopCombination& item,
                         ImplIScoreBlackboard& blackboard) override;

  bool isSatisfied() const override;

  std::string toString() const override;


protected:

  ISopUsageTracker& _tracker;
};



} // namespace tse

