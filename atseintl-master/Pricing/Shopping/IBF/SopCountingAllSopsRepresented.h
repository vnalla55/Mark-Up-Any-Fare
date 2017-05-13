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

#pragma once

#include "Pricing/Shopping/IBF/ISopUsageTracker.h"
#include "Pricing/Shopping/IBF/SopCountingAllSopsRepresentedRatingPolicy.h"
#include "Pricing/Shopping/IBF/SopCountingAppraiser.h"
#include "Pricing/Shopping/Swapper/BasicAppraiserScore.h"
#include "Pricing/Shopping/Swapper/IAppraiser.h"
#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h"

#include <boost/utility.hpp>

#include <sstream>

namespace tse
{

class SopCountingAllSopsRepresented : public swp::IAppraiser<utils::SopCombination, swp::BasicAppraiserScore>,
                              boost::noncopyable
{
public:

  SopCountingAllSopsRepresented(ISopUsageTracker& tracker): _tracker(tracker), _appraiser(_policy){}

  swp::BasicAppraiserScore
  beforeItemAdded(const utils::SopCombination& item, ImplIScoreBlackboard& blackboard) override;

  void beforeItemRemoved(const utils::SopCombination& item,
                         ImplIScoreBlackboard& blackboard) override;

  bool isSatisfied() const override;

  std::string toString() const override;

private:

  ISopUsageTracker& _tracker;
  SopCountingAllSopsRepresentedRatingPolicy _policy;
  SopCountingAppraiser<SopCountingAllSopsRepresentedRatingPolicy> _appraiser;
};



} // namespace tse

