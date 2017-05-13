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

#include "Common/TsePrimitiveTypes.h"
#include "Pricing/Shopping/IBF/SopCountingAppraiserTypes.h"
#include "Pricing/Shopping/Swapper/BasicAppraiserScore.h"
#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h"

namespace tse
{

class SopUniquenessPredicate
{
public:
  bool operator()(unsigned int newCount, bool increased)
  {
    if (increased)
    {
      // We want to handle events:
      // 1. when the first item arrives to mark uniqueness
      // 2. when the second item arrives to mark non-uniqueness
      if (newCount <= 2)
      {
        return true;
      }
      return false;
    }

    // When the last item remains, we want to emit an event
    if (newCount == 1)
    {
      return true;
    }
    return false;
  }
};



class SopCountingAllSopsRepresentedRatingPolicy
{
public:
  typedef SopUniquenessPredicate PredicateType;

  SopUniquenessPredicate& getPredicate()
  {
    return _predicate;
  }

  swp::BasicAppraiserScore rate(const utils::SopCombination& item,
      const ISopCombinationsTracker& tracker) const
  {
    size_t unique_sops_in_item = 0;
    for (size_t i = 0; i < item.size(); ++i)
    {
      const size_t sopCount =
          tracker.getItemsContainingPart(utils::SopEntry(static_cast<LegId>(i), item[i])).size();
      if (sopCount == 1)
      {
        unique_sops_in_item += 1;
      }
    }

    if (unique_sops_in_item > 0)
    {
      return swp::BasicAppraiserScore(swp::BasicAppraiserScore::MUST_HAVE,
          static_cast<int>(unique_sops_in_item));
    }
    return swp::BasicAppraiserScore(swp::BasicAppraiserScore::IGNORE);
  }

  bool isScoreAcceptable(const swp::BasicAppraiserScore& score) const
  {
    // The SopCountingAllSopsRepresented appraiser uses its own version
    // of isSatisfied(): safe to just return true here.
    return true;
  }

private:
  SopUniquenessPredicate _predicate;
};

} // namespace tse

