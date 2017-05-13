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

#include "Pricing/Shopping/Swapper/IMapUpdater.h"

#include <string>

namespace tse
{

namespace swp
{

// IAppraiser represents a single requirement on items
// collected by the swapper. It tracks the collection and
// returns isSatisfied() == true when a specific requirement
// is met, e.g. the required number of items of some type
// is collected inside the swapper.
//
// An appraiser gives scores to new items that the user
// adds to the swapper and the swapper uses these scores
// to create a final score for a particular item. Such
// final scores are used by the swapper to prioritize
// items and remove least attractive ones.
//
// For each item added to the swapper by the user, the swapper
// executes the following protocol:
// a) swapper asks its apraisers to give the item a score
// calling beforeItemAdded() on them
// b) the item is added to the swappers collection
// c) if swapper decides to remove a single item
// (e.g. because the swapper has too many elements now),
// it calls beforeItemRemoved() on its appraisers
// with such item
// d) swapper removes the item from the collection
// (if decided to remove item in c) )
//
// In calls to beforeItemAdded() and beforeItemRemoved(),
// an appraiser:
// * has an opportunity to update its state,
//   e.g. modify its structures tracking items in
//   the appraiser's interest.
// * using the ImplIScoreBlackboard interface, an appraiser
//   is also able to update scores also for items different
//   that currently added or removed
template <typename ItemType, typename ScoreType>
class IAppraiser
{
public:
  typedef ItemType Item;
  typedef ScoreType Score;
  typedef IMapUpdater<ItemType, ScoreType> ImplIScoreBlackboard;

  // Reacts on a new item addition, returning a score for this item
  // May also update scores for other items using ImplIScoreBlackboard
  //
  // Order note: first beforeItemAdded() is called, then
  //             the item is added to the swapper
  virtual Score beforeItemAdded(const Item& item, ImplIScoreBlackboard& blackboard) = 0;

  // Reacts on a swapper's item removal
  // May also update scores for other items using ImplIScoreBlackboard
  //
  // Order note: first beforeItemRemoved() is called, then
  //             the item is removed from the swapper
  virtual void beforeItemRemoved(const Item& item, ImplIScoreBlackboard& blackboard) = 0;

  // Tells if this appraiser is satisfied
  // with the current set of items in the swapper
  virtual bool isSatisfied() const = 0;

  // Returns the string representation of this appraiser
  virtual std::string toString() const = 0;

  virtual ~IAppraiser() {}
};

} // namespace swp

} // namespace tse

