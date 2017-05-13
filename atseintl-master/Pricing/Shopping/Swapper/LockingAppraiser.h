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

#include "Pricing/Shopping/Swapper/BasicAppraiserScore.h"
#include "Pricing/Shopping/Swapper/IAppraiser.h"

namespace tse
{

namespace swp
{

// An appraiser characterized by its boolean state 'blocking'.
// If in the locking state, rates each item as 'must have'
// If not in the locking state, rates each item as 'ignore'
//
// Meant to be controlled by some external logic, it has
// itself no knowledge about items being processed.
template <typename ItemType>
class LockingAppraiser : public IAppraiser<ItemType, BasicAppraiserScore>
{
public:
  // Sets a user-specified name for this appraiser
  LockingAppraiser(const std::string& name) : _name(name), _locking(false) {}

  // Returns MUST_HAVE if in the locking state.
  // Otherwise, returns IGNORE
  BasicAppraiserScore
  beforeItemAdded(const ItemType& item,
                  IMapUpdater<ItemType, BasicAppraiserScore>& blackboard) override
  {
    if (_locking)
    {
      return BasicAppraiserScore(tse::swp::BasicAppraiserScore::MUST_HAVE);
    }
    return BasicAppraiserScore(tse::swp::BasicAppraiserScore::IGNORE);
  }

  // Does nothing
  void beforeItemRemoved(const ItemType& item,
                         IMapUpdater<ItemType, BasicAppraiserScore>& blackboard) override
  {
  }

  // Always satisfied by definition (no internal logic)
  bool isSatisfied() const override
  {
    return true;
  }

  // Returns this appraiser's name, as set by the user
  std::string toString() const override
  {
    return _name;
  }

  // Sets or clears the internal locking state,
  // according to the boolean input
  void setLocking(bool b) { _locking = b; }

private:
  std::string _name;
  bool _locking;
};

} // namespace swp

} // namespace tse

