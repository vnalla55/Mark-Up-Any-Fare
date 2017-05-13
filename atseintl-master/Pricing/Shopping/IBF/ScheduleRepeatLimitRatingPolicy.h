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
#include "Pricing/Shopping/Utils/CountingMultimap.h"
#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h"

namespace tse
{

class ScheduleRepeatLimitRatingPolicy
{
public:
  typedef utils::CountLimitPredicate PredicateType;
  typedef std::set<unsigned int> Legs;

  ScheduleRepeatLimitRatingPolicy(unsigned int limit): _limitPredicate(limit) {}

  utils::CountLimitPredicate& getPredicate()
  {
    return _limitPredicate;
  }

  unsigned int getLimit() const { return _limitPredicate.getLimit(); }

  void addIgnoredLeg(unsigned int legId)
  {
    _ignoredLegs.insert(legId);
  }

  const Legs& getIgnoredLegs() const
  {
    return _ignoredLegs;
  }

  swp::BasicAppraiserScore rate(const utils::SopCombination& item,
        const ISopCombinationsTracker& tracker) const
  {
    int rank = 0;
    bool anySrlBreakingLeg = false;
    for (unsigned int i = 0; i < item.size(); ++i)
    {
      const unsigned int sopCount = static_cast<unsigned int>(
          tracker.getItemsContainingPart(utils::SopEntry(static_cast<LegId>(i),
                                                         item[i])).size());
      const unsigned int limit = getLimit();
      if (sopCount > limit)
      {
        rank = rank - static_cast<int>(sopCount - limit);
        if (_ignoredLegs.find(i) == _ignoredLegs.end())
        {
          // This leg is not ignored <=> SRL is feasible here.
          // Mark this option as intended to remove since it is breaking SRL.
          anySrlBreakingLeg = true;
        }
      }
    }

    if (rank == 0)
    {
      return swp::BasicAppraiserScore(swp::BasicAppraiserScore::IGNORE);
    }

    if (anySrlBreakingLeg)
    {
      return swp::BasicAppraiserScore(swp::BasicAppraiserScore::WANT_TO_REMOVE, rank);
    }

    return swp::BasicAppraiserScore(swp::BasicAppraiserScore::NICE_TO_HAVE, rank);
  }

  bool isScoreAcceptable(const swp::BasicAppraiserScore& score) const
  {
    return score.getCategory() != swp::BasicAppraiserScore::WANT_TO_REMOVE;
  }

private:

  utils::CountLimitPredicate _limitPredicate;
  // Give a rank but do not mark SOPs on this leg as breaking the SRL
  // (WANT_TO_REMOVE).
  Legs _ignoredLegs;
};

} // namespace tse


