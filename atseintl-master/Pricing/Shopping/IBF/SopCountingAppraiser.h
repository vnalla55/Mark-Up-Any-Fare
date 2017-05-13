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

#include "Common/Assert.h"
#include "Pricing/Shopping/Swapper/BasicAppraiserScore.h"
#include "Pricing/Shopping/Swapper/IAppraiser.h"
#include "Pricing/Shopping/Utils/PartUsageTracker.h"
#include "Pricing/Shopping/Utils/ShoppingUtils.h" // hash_value for SopEntry
#include "Pricing/Shopping/Utils/ShoppingUtilTypes.h"
#include "Pricing/Shopping/Utils/SopCombinationDeconstructor.h"

#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>

namespace tse
{

template <typename RatingPolicyType>
class SopCountingAppraiser: public swp::IAppraiser<utils::SopCombination, swp::BasicAppraiserScore>,
                            public utils::ICountingEventReceiver<utils::SopCombination>,
                            boost::noncopyable
{
public:
  using ScoreMap = boost::unordered_map<utils::SopCombination, swp::BasicAppraiserScore>;

  SopCountingAppraiser(RatingPolicyType& ratingPolicy):
      _ratingPolicy(ratingPolicy),
      _sopUsageTracker(ratingPolicy.getPredicate())
  {
    _sopUsageTracker.setReceiver(this);
    clearState();
  }

  // Throws on item duplicate
  swp::BasicAppraiserScore
  beforeItemAdded(const utils::SopCombination& item, ImplIScoreBlackboard& blackboard) override
  {
    TSE_ASSERT(_scoreMemory.find(item) == _scoreMemory.end());

    // Give the default score at the beginning
    // This can be a matter of update later in possible
    // callbacks to eventForValue()
    _scoreMemory[item] = createDefaultScore();

    saveState(item, blackboard);
    _sopUsageTracker.add(item);
    // The above may trigger callbacks to eventForValue().

    clearState();

    // We return the final score from the map
    return _scoreMemory.find(item)->second;
  }

  // Throws if removed item does not exist
  void beforeItemRemoved(const utils::SopCombination& item,
                         ImplIScoreBlackboard& blackboard) override
  {
    TSE_ASSERT(_scoreMemory.find(item) != _scoreMemory.end());

    saveState(item, blackboard);
    _sopUsageTracker.remove(item);
    // The above may trigger callbacks to eventForValue().

    clearState();

    // forget about this item
    _scoreMemory.erase(item);
  }

  bool isSatisfied() const override
  {
    for (const auto& elem : _scoreMemory)
    {
      if (!_ratingPolicy.isScoreAcceptable(elem.second))
      {
        return false;
      }
    }
    return true;
  }

  std::string toString() const override
  {
    std::ostringstream out;
    out << "SopCountingAppraiser";
    return out.str();
  }



  void eventForValue(const utils::SopCombination& item, bool increased) override
  {
    if (!isEventRelevant(item, increased))
    {
      return;
    }

    const swp::BasicAppraiserScore score = _ratingPolicy.rate(item, _sopUsageTracker);
    const ScoreMap::const_iterator it = _scoreMemory.find(item);
    // Item is in the tracker <=> is in the map
    TSE_ASSERT(it != _scoreMemory.end());
    if (score != it->second)
    {
      // Always update score memory
      _scoreMemory[item] = score;
      // Only send update if the item is not the current one
      TSE_ASSERT(_currentItem != utils::SopCombination());
      if (item != _currentItem)
      {
        TSE_ASSERT(_currentBlackboard != nullptr);
        _currentBlackboard->updateValue(item, score);
      }
    }
  }

private:
  using ImplPartUsageTracker = utils::PartUsageTracker<utils::SopCombinationDeconstructor,
                                                       typename RatingPolicyType::PredicateType>;

  swp::BasicAppraiserScore createDefaultScore() const
  {
    return swp::BasicAppraiserScore(swp::BasicAppraiserScore::IGNORE);
  }

  void saveState(const utils::SopCombination& item, ImplIScoreBlackboard& blackboard)
  {
    _currentItem = item;
    _currentBlackboard = &blackboard;
  }

  void clearState()
  {
    _currentItem = utils::SopCombination();
    _currentBlackboard = nullptr;
  }

  bool isEventRelevant(const utils::SopCombination& item, bool increased) const
  {
    // For item equal to _currentItem and increased == false,
    // we have an event triggered by beforeItemRemoved
    // for the item being removed.
    // We do not need to calculate a score for this item
    // nor do any update for the item
    if ((item == _currentItem) && (!increased))
    {
      return false;
    }
    return true;
  }

  RatingPolicyType& _ratingPolicy;
  ImplPartUsageTracker _sopUsageTracker;
  ScoreMap _scoreMemory;

  // These fields maintain the state while we are inside
  // beforeItemAdded() or beforeItemRemoved(). We need the state
  // to process callbacks to eventForValue.
  utils::SopCombination _currentItem;
  ImplIScoreBlackboard* _currentBlackboard = nullptr;
};
} // namespace tse
