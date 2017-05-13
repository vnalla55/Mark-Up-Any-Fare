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

#pragma once

#include "Pricing/Shopping/Swapper/BasicAppraiserScore.h"
#include "Pricing/Shopping/Swapper/IAppraiser.h"
#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h"

#include <boost/utility.hpp>

namespace tse
{
class ShoppingTrx;

// We only want options that don't respect minimum connect time
// as a last resort in All SOPs Represented FOSes.
class MctCombinableOptionsAreBetter
    : public swp::IAppraiser<utils::SopCombination, swp::BasicAppraiserScore>,
      boost::noncopyable
{
public:
  MctCombinableOptionsAreBetter(const ShoppingTrx& trx) : _trx(trx) {}

  swp::BasicAppraiserScore
  beforeItemAdded(const utils::SopCombination& item, ImplIScoreBlackboard& blackboard) override;

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
    return "MCT options preferred";
  }

private:
  const ShoppingTrx& _trx;
};
} // namespace tse
